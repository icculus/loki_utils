/*
    Loki Game Utility Functions
    Copyright (C) 1999  Loki Entertainment Software

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>

#include "loki_inifile.h"

struct line {
    char *key;
    char *value;
    char *comment;
    struct line *next, *previous;
};

struct section {
    char *name;
    struct line *lines;
    struct section *next, *previous;
};

struct _loki_ini_file_t {
    char path[PATH_MAX];
    int changed; /* Boolean */
    int userreg; /* Special parsing code for the .loki file */
    struct section *sections, *iterator;
};

struct _loki_ini_line_t {
    ini_file_t *ini;
    struct section *section;
    struct line *current;
};

enum status { _start, _section, _key, _value, _before_comment, _comment };

static void free_section(struct section *s);

static int isblank(char c)
{
    return (c == ' ') || (c == '\t');
}

static struct section *add_new_section(ini_file_t *ini)
{
    struct section *ret = (struct section *) malloc(sizeof(struct section));
    if( ! ret ) {
        perror("malloc");
        return NULL;
    }
    ret->next = NULL;
    ret->lines = NULL;
    ret->name = NULL;
    if( ini->sections ) {
        struct section *ins = ini->sections;
        while ( ins->next )
            ins = ins->next;
        ins->next = ret;
        ret->previous = ins;
    } else {
        ini->sections = ret;
        ret->previous = NULL;
    }

    return ret;
}

static struct line *add_new_line(struct section *s)
{
    struct line *ret = (struct line *) malloc(sizeof(struct line));
    if( ! ret ) {
        perror("malloc");
        return NULL;
    }
    ret->key = ret->value = ret->comment = NULL;
    ret->next = NULL;
    if( s->lines ) {
        struct line *ins = s->lines;
        while ( ins->next )
            ins = ins->next;
        ins->next = ret;
        ret->previous = ins;
    } else {
        s->lines = ret;
        ret->previous = NULL;
    }

    return ret;
}

/* Removes the trailing spaces */
static void trim_spaces(char *str)
{
    if ( str && *str ) { /* At least one char */
        char *ptr = str + strlen(str) - 1;

        for ( ; ptr > str; -- ptr ) {
            if ( !isblank(*ptr) ) {
                *(ptr + 1) = '\0';
                return;
            }
        }
    }
}

/* Create a new INI file from scratch */
ini_file_t * loki_createinifile_internal(const char *path, int userreg)
{
    ini_file_t *ini;
    FILE *fd;

    ini = malloc(sizeof(ini_file_t));

    if( ! ini )
        return NULL;

    ini->sections = ini->iterator = NULL;
    ini->changed = 0;
    ini->userreg = userreg;
    strncpy(ini->path, path, PATH_MAX);
    fd = fopen(path, "wb");
    if( ! fd ) {
        free(ini);
        return NULL;
    }
    fclose(fd);
    add_new_section(ini);

    return ini;
}

ini_file_t * loki_createinifile(const char *path)
{
    return loki_createinifile_internal(path, 0);
}

ini_file_t *loki_openinifile_internal(const char *path, int userreg);

/* Open and loads the INI file, returns error code */
ini_file_t *loki_openinifile(const char *path)
{
    return loki_openinifile_internal(path, 0);
}

ini_file_t *loki_openinifile_internal(const char *path, int userreg)
{
    ini_file_t *ini;
    FILE *fd;
    char buf[1024], c, prevc = '\0', *ptr = NULL;
    enum status st = _start;
    struct section *s = NULL;
    struct line *l = NULL;
    int line_number = 1;

    ini = malloc(sizeof(ini_file_t));

    if( ! ini )
        return NULL;

    ini->sections = ini->iterator = NULL;
    ini->changed = 0;
    ini->userreg = userreg;
    fd = fopen(path, "rb");
    if( ! fd ) {
        free(ini);
        /* Create the file if necessary */
        if ( access(path, F_OK) < 0  &&  ! userreg ) {
            return loki_createinifile(path);
        }
        return NULL;
    }
    strncpy(ini->path, path, PATH_MAX);

    s = add_new_section(ini); /* Top level section, can only contain comment lines */

    /* Parse the file */
    while ( (c = fgetc(fd)) != EOF ) {
        switch(st) {
        case _start: /* Start of line */
            ptr = buf;
            switch(c){
            case '\r':
                break;
            case '\n':
                l = add_new_line(s);
                ++ line_number;
                break;
            case ';': case '#':
                l = add_new_line(s);
                st = _comment;
                break;
            case '[':
                st = _section;
                s = add_new_section(ini);
                break;
            default:
                if ( isblank(c) ) {
                    break;
                } else if ( ! s->name && ! userreg ) {
                    fprintf(stderr,"Parse error at beginning of %s INI file (line %d)!\n", path, line_number);
                    fclose(fd);
                    free_section(ini->sections);
                    free(ini);
                    return 0;
                } else {
                    l = add_new_line(s);
                    *ptr ++ = c;
                    st = _key;
                }
                break;
            }
            break;
        case _section:
            if ( c == ']' ) {
                *ptr = '\0';
                s->name = strdup(buf);
                ptr = buf;
                st = _before_comment;
            } else {
                *ptr ++ = c;
            }
            break;
        case _key:
            if ( c == '=' ) {
                *ptr = '\0';
                trim_spaces(buf);
                l->key = strdup(buf);
                ptr = buf;
                st = _value;
            } else if ( c == '\r' ) {
                // Nothing
            } else if ( c == '\n' ) {
                fprintf(stderr,"Parse error in %s on line %d: end of line before rvalue\n", path, line_number);
                fclose(fd);
                free_section(ini->sections);
                free(ini);
                return 0;
            } else {
                *ptr ++ = c;
            }
            break;
        case _value:
            /* No more comments are allowed on the same line as an affectation */
            if ( c == '\n' ) {
                *ptr = '\0';
                trim_spaces(buf);
                l->value = strdup(buf);
                ptr = buf;
                ++ line_number;
                st = _start;
            } else if ( c != '\r' ) {
                *ptr ++ = c;
            }
            break;
        case _before_comment:
            if ( c == ';' || c == '#' ) {
                st = _comment;
            } else if ( c == '\n' ) {
                ++ line_number;
                st = _start;
            }
            break;
        case _comment: /* Till the end of line */
            if(c == '\n' ) {
                *ptr = '\0';
                if ( ! l ) {
                    l = add_new_line(s);
                }
                l->comment = strdup(buf);
                ptr = buf;
                ++ line_number;
                st = _start;
            } else if ( c != '\r' ) {
                *ptr ++ = c;
            }
            break;
        }
        prevc = c;
    }

    /* End of file reached, check for unfinished stuff */
    switch(st) {
    case _value:
		*ptr = '\0';
		trim_spaces(buf);
		l->value = strdup(buf);
		break;
    case _comment:
		*ptr = '\0';
		if ( ! l ) {
			l = add_new_line(s);
		}
		l->comment = strdup(buf);
		break;
    case _section:
		fprintf(stderr,"Parse error in %s: end of file reached while in section name.\n", path);
		break;
    default:
    }

    fclose(fd);
    return ini;
}

/* Recursive functions to free the linked lists */
static void free_line(struct line *l)
{
    if ( l ) {
        free_line( l->next );

        free(l->key);
        free(l->value);
        free(l->comment);
        free(l);
    }
}

static void free_section(struct section *s)
{
    if ( s ) {
        free_section( s->next );

        free(s->name);
        free_line(s->lines);
        free(s);
    }
}

/* Close the INI file, returns error code */
int loki_closeinifile(ini_file_t *ini)
{
    int closed;

    closed = 0;
    if ( ini ) {
        /* Free all the allocated memory */
        free_section(ini->sections);
        
        free(ini);

        closed = 1;
    }
    return closed;
}


/* Returns a boolean value indicating if the image of the INI file in memory is in sync
   with the file on disc */
int loki_inihaschanged(ini_file_t *ini)
{
    if ( ini ) {
        return ini->changed;
    }
    return 0;
}

/* Return the string corresponding to a key in the specified section of the file,
   returns NULL if could not find it */
const char *loki_getinistring(ini_file_t *ini, const char *section, const char *key)
{
    struct section *s;
    
    if ( ! ini ) {
        return NULL;
    }

    for ( s = ini->sections ; s ; s = s->next ) {
        if ( (s->name && ! strcasecmp(section, s->name)) || ini->userreg ) {
            struct line *l;
            for ( l = s->lines; l ; l = l->next ) {
                if ( l->key && ! strcasecmp(key, l->key) ) {
                    return l->value;
                }
            }
        }
    }
    return NULL;
}

/* Add or modify a keyed value in the INI file, returns error code */
int loki_putinistring(ini_file_t *ini, const char *section, const char *key, const char *value)
{
    struct section *s;

    if ( ! ini ) {
        return 0;
    }

    for ( s = ini->sections ; s ; s = s->next ) {
        if ( (s->name && section && ! strcasecmp(section, s->name)) || ini->userreg ) {
            struct line *l;
            for ( l = s->lines; l ; l = l->next ) {
                if ( l->key && ! strcasecmp(key, l->key) ) {
                    /* Replace existing value */
                    free(l->value);
                    l->value = strdup(value);
                    ini->changed = 1;
                    return 1;
                }
            }
            if ( ! l ) {
                /* Create new keyed value */
                l = add_new_line(s);
                l->key = key ? strdup(key) : NULL;
                l->value = value ? strdup(value) : NULL;
                ini->changed = 1;
                return 1;
            }
        }
    }
    if ( ! s ) {
        struct line *l;
        /* Create new section and key */
        s = add_new_section(ini);
        s->name = section ? strdup(section) : NULL;

        l = add_new_line(s);
        l->key = key ? strdup(key) : NULL;
        l->value = value ? strdup(value) : NULL;
        ini->changed = 1;
        return 1;
    }
    return 0;
}

/* Write the INI file back to disk, returns error code */
int loki_writeinifile(ini_file_t *ini, const char *path)
{
    struct section *s;
    FILE *fd;

    if ( ! ini ) {
        return 0;
    }

    s  = ini->sections;
    if ( ! path ) {
        path = ini->path;
    }
    fd = fopen(path, "wb");
    if ( ! fd ) {
        perror("INI fopen(wb)");
        return 0;
    }

    for ( ; s ; s = s->next ) {
        struct line *l;
        if( s->name ) {
            fprintf(fd, "[%s]\n", s->name);
        }
        for ( l = s->lines; l ; l = l->next ) {
            if ( l->key ) {
                fprintf(fd, "%s=", l->key);
                if ( l->value ) {
                    fprintf(fd, "%s", l->value);
                }
                fputc(' ', fd);
            }
            if ( l->comment ) {
                fprintf(fd, " ;%s", l->comment);
            }
            fputc('\n', fd);
        }
    }
    fclose(fd);

    ini->changed = 0; /* Mark as in sync with the data on disc */
    return 1;
}


/* Initialize the iterator to the beginning of the given section.
   Returns NULL if the section does not exist.
 */
ini_line_t *loki_begin_iniline(ini_file_t *ini, const char *section)
{
    struct section *s;

    if ( ! ini ) {
        return NULL;
    }

    for ( s = ini->sections ; s ; s = s->next ) {
        if ( (s->name && ! strcasecmp(section, s->name)) || ini->userreg ) {
            ini_line_t *ret = malloc(sizeof(ini_line_t));
            ret->ini = ini;
            ret->section = s;
            ret->current = s->lines;
            while( ret->current && !ret->current->key ) {
                ret->current = ret->current->next;
            }
            return ret;
        }
    }
    return NULL;
}

/* Get the current key/value pair pointed to by the iterator.
   'lkey' and 'lvalue' are the size of the buffers passed in argument to the function.
   Returns a positive value if everything was OK, or zero else.
 */
int loki_get_iniline(ini_line_t *iterator, const char **key, const char **value)
{
    if ( ! iterator || ! iterator->current ) {
        return 0;
    }
    *key = iterator->current->key;
    *value = iterator->current->value;
    return 1;
}

/* Update the value of a line */
int loki_update_iniline(ini_line_t *iterator, const char *value)
{
    if ( ! iterator || ! iterator->current ) {
        return 0;
    }
    free(iterator->current->value);
    iterator->current->value = strdup(value);
    return 1;
}

/* Iterator to the next line of the section.
   Returns zero when at the end of the section or an error occured.
 */
int loki_next_iniline(ini_line_t *iterator)
{
    if ( ! iterator || ! iterator->current ) {
        return 0;
    }
    /* Skip the void lines */
    do {
        iterator->current = iterator->current->next;
        if ( ! iterator->current ) {
            return 0;
        }
    } while ( ! iterator->current->key );

    return 1;
}

/* Free the iterator object allocated by loki_begininisection.
   Must be called when the user is done with the iterator.
 */
void loki_free_iniline(ini_line_t *iterator)
{
    free(iterator);
}

/* More general function to remove a specified key in a section */

int loki_remove_iniline(ini_file_t *ini, const char *section, const char *key)
{
    struct section *s;
    struct line *l;

    for ( s = ini->sections ; s ; s = s->next ) {
        if ( (s->name && !strcasecmp(section, s->name)) || ini->userreg ) {
            for( l = s->lines; l; l = l->next ) {
                if ( l->key && !strcasecmp(key, l->key) ) {
                    struct line *prevl = l->previous;
                    /* Found it */
                    if ( prevl ) {
                        prevl->next = l->next;
                    } else {
                        s->lines = l->next;
                    }
                    if ( l->next ) {
                        l->next->previous = prevl;
                    }
                    free(l->key); free(l->value); free(l->comment);
                    free(l);
                    ini->changed = 1;
                    if ( ! s->lines ) { /* Section is now empty, remove it */
                        struct section *prevs = s->previous;
                        if ( prevs ) {
                            prevs->next = s->next;
                        } else {
                            ini->sections = s->next;
                        }
                        if ( s->next ) {
                            s->next->previous = prevs;
                        }
                        free(s->name);
                        free(s);
                    }
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0;
}

/* Remove the current line; the iterator is changed to point to the next line if available */
int loki_remove_current_iniline(ini_line_t *iterator)
{
    struct line *prev = iterator->current->previous, *cur = iterator->current;
    if ( prev ) {
        prev->next = cur->next;
    } else {
        iterator->section->lines = cur->next;
    }
    iterator->current = cur->next;
    if ( iterator->current ) {
        iterator->current->previous = prev;
    }
    free(cur->key);
    free(cur->value);
    free(cur->comment);
    free(cur);

    iterator->ini->changed = 1;

    /* Check if section is empty now */
    if ( iterator->section->lines ) {
        /* We should still have a valid iterator at this point, unless we reached the end */
        return iterator->current != NULL;
    } else {
        struct section *section = iterator->section;
        /* Section is empty, remove it */
        if ( section->previous ) {
            section->previous->next = section->next;
        } else {
            iterator->ini->sections = section->next;
        }
        if ( section->next ) {
            section->next->previous = section->previous;
        }
        /* Check if the iterator for this file is pointing to the deleted section */
        /* TODO: Check for bad things that might happen with that */
        if ( iterator->ini->iterator == section ) {
            iterator->ini->iterator = section->next;
        }
        free(section->name);
        free(section);
    }
    return 0;
}


/* Returns the name of the fist section of the given file (initializes an internal iterator) */
const char *loki_begin_inisection(ini_file_t *ini)
{
    /* Skip the first pseudo-section */
    ini->iterator = ini->sections->next;
    if ( ini->iterator ) {
        return ini->iterator->name;
    } else {
        return NULL;
    }
}

/* Returns the name of the next section of the given file, or NULL if no more sections */
const char *loki_next_inisection(ini_file_t *ini)
{
    if ( ini->iterator ) {
        ini->iterator = ini->iterator->next;
        if ( ini->iterator ) {
            return ini->iterator->name;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

int loki_iterate_iniline(ini_file_t *ini, const char *section, ini_callback_t func, void *param)
{
    struct section *s;
    int ret = 0;
    for ( s = ini->sections ; s ; s = s->next ) {
        if ( (s->name && !strcasecmp(section, s->name)) || ini->userreg ) {
            struct line *l;
            for( l = s->lines; l; l = l->next ) {
                ret += func(ini, section, l->key, l->value, param);
            }
        }
    }
    return ret;
}
