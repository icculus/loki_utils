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

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <ctype.h>

#include "loki_utils.h"
#include "loki_inifile.h"

/* This is the name of the configuration file */
#define CONFIG_FILENAME    "userprofile.txt"

/* This defines the maximum number of options recognized */
#define MAX_OPTIONS 255

/* This is a list of general configuration parameters in userprofile.txt
   and set from command-line options:
*/
static int nb_options = 0;
static struct option long_options[MAX_OPTIONS+1];
static int optional[MAX_OPTIONS];
static const char *option_comment[MAX_OPTIONS];

static char **remaining_args = NULL;


/* For now, we use a simple linked-list implementation */
typedef struct config_element {
    char *key;
    char *value;
    struct config_element *next;
} config_element;

static config_element *config_list = NULL;

static char* loki_config_default = NULL;

void loki_configdefault( const char* dflt )
{

    if( loki_config_default ) {
        free( loki_config_default );
        loki_config_default = NULL;
    }

    if( dflt ) {
        int length = strlen( dflt );
        loki_config_default = (char*) malloc( length + 1 );
        
        if( loki_config_default ) {
            strncpy( loki_config_default, dflt, length );
            loki_config_default[length] = '\0';
        }
        
    }
}

void loki_insertconfig(const char *key, const char *value)
{
    config_element *pip, *last;
    static struct {
        const char *option1;
        const char *option2;
    } exclusives[] = {
        { "fullscreen", "windowed" }
    };
    int i;

    /* Handle special exclusive options here
       If one option is set, the other one is deleted
     */
    for ( i=0; i<(sizeof(exclusives)/sizeof(exclusives[0])); ++i ) {
        if ( strcasecmp(key, exclusives[i].option1) == 0 ) {
            loki_deleteconfig(exclusives[i].option2);
            break;
        }
        if ( strcasecmp(key, exclusives[i].option2) == 0 ) {
            loki_deleteconfig(exclusives[i].option1);
            break;
        }
    }

    /* Search for an existing entry with this value */
    last = NULL;
    for ( pip=config_list; pip; last=pip, pip=pip->next ) {
        if ( strcasecmp(key, pip->key) == 0 ) {
            break;
        }
    }
    if ( pip ) {  /* Replace existing entry */
        if ( value ) {
            pip->value = (char *)realloc(pip->value, strlen(value)+1);
            strcpy(pip->value, value);
        } else {
            pip->value = NULL;
        }
    } else {
        pip = (config_element *)malloc(sizeof *pip);
        assert(pip);
        pip->key = (char *)malloc(strlen(key)+1);
        assert(pip->key);
        strcpy(pip->key, key);
        if ( value ) {
            pip->value = (char *)malloc(strlen(value)+1);
            assert(pip->value);
            strcpy(pip->value, value);
        } else {
            pip->value = NULL;
        }
        pip->next = NULL;

        if ( last ) {
            last->next = pip;
        } else {
            config_list = pip;
        }
    }
}

void loki_deleteconfig(const char *key)
{
    config_element *pip, *last;

    /* Search for an existing entry with this value */
    last = NULL;
    for ( pip=config_list; pip; last=pip, pip=pip->next ) {
        if ( strcasecmp(key, pip->key) == 0 ) {
            break;
        }
    }

    /* Delete it if we found it */
    if ( pip ) {
        if ( last ) {
            last->next = pip->next;
        } else {
            config_list = pip->next;
        }
        free(pip->key);
        if ( pip->value ) {
            free(pip->value);
        }
        free(pip);
    }
}

ini_file_t *loki_openinifile_internal(const char *path, int userreg);
ini_file_t * loki_createinifile_internal(const char *path, int userreg);

static void loki_parseconfig(const char *file)
{
    ini_file_t *ini = loki_openinifile_internal(file, 1);
    
    if ( ini ) {
        const char *key, *value;
        ini_line_t *line = loki_begin_iniline(ini, NULL);
        if ( line ) {
            do {
                if ( loki_get_iniline(line, &key, &value) ) {
                    loki_insertconfig(key, value);
                }
            } while( loki_next_iniline(line) );
        }
        loki_closeinifile(ini);
    }
}

/* This function loads Loki-specific configuration values from 
   ~/.loki/userprofile.txt and ~/.loki/<game_directory>/userprofile.txt
   This function is called by loki_parseargs().
*/
void loki_initconfig(void)
{
    char *home;
    char configfile[PATH_MAX];

    /* Set initial value. */
    loki_configdefault( "" );

    /* Load defaults from game-wide config */
    home = loki_gethomedir();
    if ( home != NULL ) {
        sprintf(configfile, "%s/.loki/%s", home, CONFIG_FILENAME);
        loki_parseconfig(configfile);
    }

    /* Load game-specific default config */
    sprintf(configfile, "%s/%s", loki_getdatapath(), CONFIG_FILENAME);
    loki_parseconfig(configfile);

    /* Load the user's own overrides */
    sprintf(configfile, "%s/%s", loki_getprefpath(), CONFIG_FILENAME);
    loki_parseconfig(configfile);
}

/* Creates an INI file with a dump of the current configuration */
void loki_writeconfig(const char *file)
{
    ini_file_t *ini;
    char configfile[PATH_MAX];

    /* If file is NULL, default to the preferences for this game */
    if ( ! file ) {
        sprintf(configfile, "%s/%s", loki_getprefpath(), CONFIG_FILENAME);
        file = configfile;
    }
    ini = loki_createinifile_internal(file, 0);
    if ( ini ) {
        config_element *pip;

        for ( pip=config_list; pip; pip=pip->next ) {
            loki_putinistring(ini, NULL, pip->key, pip->value);
        }
        loki_writeinifile(ini, file);
        loki_closeinifile(ini);
    }
}

/* This was modified from setup - if you modify either, update the other */
const char *loki_getarch(void)
{
    const char *arch;

#ifdef __i386
    arch = "x86";
#elif defined(powerpc)
    arch = "ppc";
#elif defined(__alpha__)
    arch = "alpha";
#elif defined(__sparc__)
    arch = "sparc64";
#elif defined(__arm__)
    arch = "arm";
#else
    arch = "unknown";
#endif
    return arch;
}

/* Make this easier to change from the application code */
#ifdef LINUX_DEMO
int loki_demo = 1;
#else
int loki_demo = 0;
#endif

void loki_isdemo(int value)
{
    loki_demo = value;
}

void loki_printusage(char *argv0, const char *help_text)
{
    int len;
    int spacing = 0;
    int i;

    /* Use the basename of argv[0] in the usage message */
    if ( strrchr(argv0, '/') ) {
        argv0 = strrchr(argv0, '/')+1;
    }

    printf("Linux version by Loki Entertainment Software\n");
    printf("http://www.lokigames.com/\n");
    if ( loki_demo ) {
        printf("Technical support is only available with the full game\n");
    } else {
        printf("Support - FAQ: http://faqs.lokigames.com/\n"
               "          Web: http://fenris.lokigames.com/\n"
               "          E-mail: support@lokigames.com\n"
               "          Phone:  1-714-508-2140 (9-5 PM US Pacific Time)\n"
               );
    }
    printf("\n");
    printf("Usage: %s [options]\n", argv0);

        /* Add spacing so comments are aligned. ("\t" was not enough.) */
    for(i = 0; i<nb_options; i++)
    {
        int len = strlen(long_options[i].name);
        if (len > spacing)
            spacing = len;
    }

    spacing += 4;  /* for extra spacing. */

    for(i = 0; i < nb_options; i++)
    {
      printf("     [");
      if ( isalpha(long_options[i].val) )
      {
        printf("-%c", long_options[i].val);
        if (long_options[i].name != NULL)
            printf(" | ");
      }
      else
          printf("     ");

      if (long_options[i].name != NULL);
        printf("--%s", long_options[i].name);

      printf("]");

      if (option_comment[i] != NULL)
      {
        for (len = spacing - strlen(long_options[i].name); len >= 0; len--)
            printf(" ");

        printf("%s", option_comment[i]);
      }

      putchar('\n');
    }

    if(help_text)
      printf("%s", help_text);
    putchar('\n');
}

void loki_register_stdoptions(void)
{
    /* Don't do anything if there are options already registered */
    if ( nb_options > 0 ) {
        return;
    }

    /* Register the standard set of Loki command line options */
    loki_registeroption("help",         'h', "Display this help message");
    loki_registeroption("version",      'v', "Display the game version");
    loki_registeroption("fullscreen",   'f', "Run the game fullscreen");
    loki_registeroption("windowed",     'w', "Run the game in a window");
    loki_registeroption("nosound",      's', "Do not access the soundcard");
    if ( ! loki_demo ) {
        loki_registeroption("nocdrom",  'c', "Do not access the CD-ROM");
        loki_registeroption("update",   'u', "Run the Loki auto-update tool");
        loki_registeroption("qagent",   'q', "Run the Loki QAgent support tool");
    }
}

/* This registers a new command-line option switch */
void loki_registeroption_as(const char *lng, char sht, const char *comment, loki_optiontype t)
{
  if(nb_options < MAX_OPTIONS){
    static struct option fin_opt = {NULL, 0,0,0};

    long_options[nb_options].name = strdup(lng); /* Needs to be free()'d at some point */
    long_options[nb_options].flag = NULL;
    optional[nb_options] = 0;
    switch(t) {
    case LOKI_OPTSTRING:
      optional[nb_options] = 1;
    case LOKI_STRING:
      long_options[nb_options].has_arg = required_argument;
      break;
    case LOKI_BOOLEAN:
      long_options[nb_options].has_arg = no_argument;
      break;
    }
    long_options[nb_options].val = sht;
    option_comment[nb_options] = comment;
    nb_options ++;
    long_options[nb_options] = fin_opt;
  }
}

/* This function parses command line arguments to finalize the config.
   This function is called by loki_initialize().
*/
void loki_parseargs(int argc, char *argv[], const char *extra_help)
{
    char short_options[MAX_OPTIONS*2+1] = ":";
    int i, j;
   
    for(i=0, j=1; i<nb_options; i++) {
        if (long_options[i].val != '\0') {
            short_options[j++] = long_options[i].val;
            if ( long_options[i].has_arg > 0 ) {
                short_options[j++] = ':';
            }
        }
    }
    short_options[j] = '\0';

    opterr = 0;
    while (1) {  /* Loop terminates when getopt returns -1 and we return */
        int c, i;

        c = getopt_long_only(argc, argv, short_options, long_options, NULL);
        switch (c) {
            case -1:
                remaining_args = &argv[optind];
                return;
            case 'u':
                if ( ! loki_demo ) {
                    loki_runupdate(argc, argv);
                }
                break;
            case 'q':
                if ( ! loki_demo ) {
                    loki_runqagent(NULL);
                }
                break;
            case 'v':
                printf("%s\n", loki_getgamedescription());
                printf("Built with glibc-%d.%d on %s\n",
                        __GLIBC__, __GLIBC_MINOR__, loki_getarch());
                exit(0);
            case 'h':
                loki_printusage(argv[0], extra_help);
                exit(0);
            case ':': /* Argument missing error */
                for ( i=0; long_options[i].name; ++i ) {
                    if ( optopt == long_options[i].val ) {
                        if(optional[i]) {
                            loki_insertconfig(long_options[i].name, NULL);
                        } else {
                            fprintf(stderr,"ERROR: Missing argument for option -%c\n", optopt);
                        }
                    }
                }
                break;
            default:
                for ( i=0; long_options[i].name; ++i ) {
                    if ( c == long_options[i].val ) {
                        switch ( long_options[i].has_arg ) {
                        case required_argument:
                            loki_insertconfig(long_options[i].name, optarg);
                            break;
                        case no_argument:
                            loki_insertconfig(long_options[i].name, "1");
                            break;
                        }
                        break;
                    }
                }
                if ( !long_options[i].name ) {
                    loki_printusage(argv[0], extra_help);
                    exit(0);
                }
                break;              
        }
    }
}

/* This function returns the subset of argv[] that holds all the non-option arguments.
   This must be called AFTER loki_initialize() for it to make any sense. */
char **loki_getarguments(void)
{
  return remaining_args;
}

/* This function returns a default value from the configuration */
static char *loki_getconfig_default(const char *key)
{
    return loki_config_default;
}

/* This function returns a string value from the configuration */
char *loki_getconfig_str(const char *key)
{
    config_element *pip;
    char *value;

    /* Search for an existing entry with this value */
    for ( pip=config_list; pip; pip=pip->next ) {
        if ( strcasecmp(key, pip->key) == 0 ) {
            break;
        }
    }
    if ( pip ) {
        value = pip->value;
    } else {
        value = loki_getconfig_default(key);
    }
    return(value);
}

/* This function returns a boolean value from the configuration */
int loki_getconfig_bool(const char *key)
{
    char *value;
    int retval;

    value = loki_getconfig_str(key);

    /* Okay, here's how we do it:
     * If value is NULL, return false.
     * If the value is "false", return false.
     * If the value is "no", return false.
     * If the value is "", return false.
     * Otherwise, return true.
     */
    if( value ) {
        if( !strcasecmp( value, "false" ) ) {
            retval = 0;
        } else if( !strcasecmp( value, "no" ) ) {
            retval = 0;
        } else if( !strcasecmp( value, "off" ) ) {
            retval = 0;
        } else if( !strcasecmp( value, "0" ) ) {
            retval = 0;
        } else if ( !strcasecmp( value, "" ) ) {
            retval = 0;
        } else {
            retval = 1;
        }
    } else {
        retval = 0;
    }

    return retval;
}

/* This function returns an int value from the configuration */
int loki_getconfig_int(const char *key)
{
    char *value;

    value = loki_getconfig_str(key);

    if( value ) {
        return atoi( value );
    } else {
        return 0;
    }

}

/* This function returns a float value from the configuration */
double loki_getconfig_float(const char *key)
{
    char *value;

    value = loki_getconfig_str(key);
    if( value ) {
        return atof( value );
    } else {
        return 0.0;
    }
}

/* This function returns an optional string value from the configuration */
int loki_getconfig_optstr(const char *key, const char **str)
{
    /* This may look like we're duplicating code but actually we need to
       check for the actual presence of the key instead of defaulting to
       a null string value */

    config_element *pip;

    /* Search for an existing entry with this value */
    for ( pip=config_list; pip; pip=pip->next ) {
        if ( strcasecmp(key, pip->key) == 0 ) {
            break;
        }
    }
    if ( pip ) {
        *str = pip->value;
        return 1;
    } else {
        *str = NULL;
        return 0;
    }
}
