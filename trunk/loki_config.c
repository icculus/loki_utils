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

/* This defines the maximum number of options recognized */
#define MAX_OPTIONS 255

/* How many builtin options ? */
#ifndef WINDOWED_ONLY
#define COMMON_OPTIONS 6
#else
#define COMMON_OPTIONS 4
#endif

/* This is a list of general configuration parameters in userprofile.txt
   and set from command-line options:
*/
static struct option long_options[MAX_OPTIONS+1] =
{
  { "help",        0, 0, 'h' },
  { "version",     0, 0, 'v' },
#ifndef WINDOWED_ONLY
  { "fullscreen",  0, 0, 'f' },
  { "windowed",    0, 0, 'w' },
#endif
  { "nosound",     0, 0, 's' },
  { "nocdrom",     0, 0, 'c' },
  { NULL,          0, 0,  0  }
};

static const char *option_comment[MAX_OPTIONS] =
{
    "Display this help message",
    "Display the game version",
#ifndef WINDOWED_ONLY
    "Run the game fullscreen",
    "Run the game in a window",
#endif
    "Do not access the soundcard",
    "Do not access the CD-ROM",
    NULL,
};

static int nb_options = COMMON_OPTIONS;

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

    /* Set initial value. */
    loki_configdefault( "" );

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
    int len;
    int spacing = 0;
    int i;
    printf("Linux version by Loki Entertainment Software\n");
    printf("http://www.lokigames.com/\n");
#ifdef LINUX_DEMO
    printf("Support - Full versions of our games have technical support\n");
#elif !defined(LINUX_BETA)
    printf("Support - Phone:  1-801-356-7629 (9-5 PM US Mountain Time)\n");
    printf("          E-mail: support@lokigames.com\n");
#endif
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
      if (long_options[i].val != '\0')
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

/* This registers a new command-line option switch */
void loki_registeroption(const char *lng, char sht, const char *comment)
{
  if(nb_options < MAX_OPTIONS){
    static struct option fin_opt = {NULL, 0,0,0};

    long_options[nb_options].name = strdup(lng); /* Needs to be free()'d at some point */
    long_options[nb_options].flag = NULL;
    long_options[nb_options].has_arg = 0;
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
    extern char *game_version;
    char short_options[MAX_OPTIONS+1];
    int i;
   
    for(i=0; i<nb_options; i++)
    {
        if (long_options[i].val != '\0')
            short_options[i] = long_options[i].val;
    }

    short_options[i] = '\0';

    while (1) {  /* Loop terminates when getopt returns -1 and we return */
        int c, i;

        c = getopt_long_only(argc, argv, short_options, long_options, 0);
        switch (c) {
            case -1:
                return;
            case 'v':
                printf("%s", game_version);
                printf("Built with glibc-%d.%d\n", __GLIBC__, __GLIBC_MINOR__);
                exit(0);
            case 'h':
                loki_printusage(argv[0], extra_help);
                exit(0);
                break;              
            default:
                for ( i=0; long_options[i].name; ++i ) {
                    if ( c == long_options[i].val ) {
                        loki_insertconfig(long_options[i].name, "1");
                        break;
                    }
                }
                if(!long_options[i].name){
                  loki_printusage(argv[0], extra_help);
                  exit(0);
                }
                break;              
        }
    }
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

