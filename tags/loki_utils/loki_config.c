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

/* This is the name of the configuration file */
#define CONFIG_FILENAME    "userprofile.txt"

/* This is a list of general configuration parameters in userprofile.txt
   and set from command-line options:
*/
static struct option long_options[] =
{
  { "help",        0, 0, 'h' },
  { "version",     0, 0, 'v' },
  { "fullscreen",  0, 0, 'f' },
  { "windowed",    0, 0, 'w' },
  { "nosound",     0, 0, 's' },
  { "nocdrom",     0, 0, 'c' },
  { NULL,          0, 0,  0  }
};

/* For now, we use a simple linked-list implementation */
typedef struct config_element {
    char *key;
    char *value;
    struct config_element *next;
} config_element;

static config_element *config_list = NULL;

static void loki_insertconfig(const char *key, const char *value)
{
    config_element *pip, *last;

    /* Search for an existing entry with this value */
    last = NULL;
    for ( pip=config_list; pip; last=pip, pip=pip->next ) {
        if ( strcasecmp(key, pip->key) == 0 ) {
            break;
        }
    }
    if ( pip ) {  /* Replace existing entry */
        pip->value = (char *)realloc(pip->value, strlen(value)+1);
        strcpy(pip->value, value);
    } else {
        pip = (config_element *)malloc(sizeof *pip);
        assert(pip);
        pip->key = (char *)malloc(strlen(key)+1);
        assert(pip->key);
        strcpy(pip->key, key);
        pip->value = (char *)malloc(strlen(value)+1);
        assert(pip->value);
        strcpy(pip->value, value);
        pip->next = NULL;

        if ( last ) {
            last->next = pip;
        } else {
            config_list = pip;
        }
    }
}

static void loki_parseconfig(const char *file)
{
    FILE *config;
    char line[BUFSIZ];
    char *key;
    char *value;
    char *trim;

    config = fopen(file, "r");
    if ( config != NULL ) {
        while ( fgets(line, BUFSIZ-1, config) ) {
            /* Isolate the key */
            for ( key=line; isspace(*key); ++key )
                ++key;
            value = strchr(key, '=');
            if ( value == NULL ) {
                continue;
            }
            for (trim=(value-1); (trim > key) && isspace(*trim); )
                --trim;
            trim[1] = '\0';

            /* Isolate the value */
            for ( value=(value+1); isspace(*value); )
                ++value;
            for (trim=(value+strlen(value)-1); (trim > key) && isspace(*trim); )
                --trim;
            trim[1] = '\0';

            /* Make sure both exist */
            if ( !*key || !*value ) {
                continue;
            }

            /* Add them to our config */
            loki_insertconfig(key, value);
        }
        fclose(config);
    }
}

/* This function loads Loki-specific configuration values from 
   ~/.loki/userprofile.txt and ~/.loki/<game_directory>/userprofile.txt
   This function is called by loki_parseargs().
*/
void loki_initconfig(void)
{
    char *home;

    home = loki_gethomedir();
    if ( home != NULL ) {
        char configfile[PATH_MAX];

        sprintf(configfile, "%s/.loki/%s", home, CONFIG_FILENAME);
        loki_parseconfig(configfile);
        sprintf(configfile, "%s/%s", loki_getprefpath(), CONFIG_FILENAME);
        loki_parseconfig(configfile);
    }
}

void loki_printusage(char *argv0, const char *help_text)
{
    printf("Linux version by Loki Entertainment Software\n");
    printf("http://www.lokigames.com/\n");
    printf("Support - Phone:  1-801-356-7629 (9-5 PM US Mountain Time)\n");
    printf("          E-mail: support@lokigames.com\n");
    printf("\n");
    printf("Usage: %s [options]\n", argv0);
    printf("\t[-h | --help]        Display this help message\n");
    printf("\t[-v | --version]     Display the game version\n");
    printf("\t[-f | --fullscreen]  Run the game fullscreen\n");
    printf("\t[-w | --windowed]    Run the game in a window\n");
    printf("\t[-s | --nosound]     Do not access the soundcard\n");
    printf("\t[-c | --nocdrom]     Do not access the CD-ROM\n");
    printf("%s", help_text);
    printf("\n");
}

/* This function parses command line arguments to finalize the config.
   This function is called by loki_initialize().
*/
void loki_parseargs(int argc, char *argv[], const char *extra_help)
{
    extern char game_version[];

    while (1) {  /* Loop terminates when getopt returns -1 and we return */
        int c, i;

        c = getopt_long_only(argc, argv, "hvfwscdj", long_options, 0);
        switch (c) {
            case -1:
                return;
            case 'v':
                printf(game_version);
                printf("Built with glibc-%d.%d on %s\n",
                       __GLIBC__, __GLIBC_MINOR__, __DATE__);
                exit(0);
            case 'f':
            case 'w':
            case 's':
            case 'c':
                for ( i=0; long_options[i].name; ++i ) {
                    if ( c == long_options[i].val ) {
                        loki_insertconfig(long_options[i].name, "1");
                        break;
                    }
                }
                break;
            case 'h':
            default:
                loki_printusage(argv[0], extra_help);
                exit(0);
        }
    }
}

/* This function returns a default value from the configuration */
static char *loki_getconfig_default(const char *key)
{
    return("");
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

    value = loki_getconfig_str(key);
    if ( (atoi(value) > 0) ||
         (strcasecmp(value, "true") == 0) ||
         (strcasecmp(value, "yes") == 0) ) {
        return(1);
    } else {
        return(0);
    }
}

/* This function returns an int value from the configuration */
int loki_getconfig_int(const char *key)
{
    char *value;

    value = loki_getconfig_str(key);
    return(atoi(value));
}

/* This function returns a float value from the configuration */
double loki_getconfig_float(const char *key)
{
    char *value;

    value = loki_getconfig_str(key);
    return(atof(value));
}

