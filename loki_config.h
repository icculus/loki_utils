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

#ifndef _LOKI_CONFIG_H
#define _LOKI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* This is a list of general configuration parameters in userprofile.txt
   and set from command-line options:
*/
// ....

/* This function loads Loki-specific configuration values from 
   ~/.loki/userprofile.txt and ~/.loki/<game_directory>/userprofile.txt
   This function is called by loki_parseargs().
*/
extern void loki_initconfig(void);

/* This defines the possible types of additional command-line options that can
   registered by the program */
typedef enum {
  LOKI_BOOLEAN = 0,
  LOKI_STRING,
  LOKI_OPTSTRING
} loki_optiontype;

/* This function tells the library that the game is (or isn't) a demo,
   so it doesn't print out support information on usage screen.
*/
extern void loki_isdemo(int isdemo);

/* Set the default value for config names if they are not found. */
extern void loki_configdefault( const char* dflt );

#define loki_registeroption(l, s, c) loki_registeroption_as(l, s, c, LOKI_BOOLEAN)

/* Add a standard set of command line options to the list,
   this should be called after loki_setgamename(), but before loki_initalize()
*/
extern void loki_register_stdoptions(void);

/* This registers a new command-line option switch.
   This functions needs to be called BEFORE any call to loki_parseargs or loki_initialize
 */
extern void loki_registeroption_as(const char *lng, char sht, const char *comment, loki_optiontype t);

/* This function parses command line arguments to finalize the config.
   This function is called by loki_initialize().
*/
extern void loki_parseargs(int argc, char *argv[], const char *extra_help);

/* This function displays usage information to stdout. */
extern void loki_printusage( char* argv0, const char* help_text );

/* This function returns a string value from the configuration */
extern char *loki_getconfig_str(const char *key);

/* This function returns an optional string value from the configuration.
   Here is how it works :
   The function returns FALSE if the option was not mentioned.
   It returns TRUE if it was mentionned. Then check *str: if it's NULL
   then the optional argument was not there, else *str is a pointer to
   the argument string.
 */
extern int loki_getconfig_optstr(const char *key, const char **str);

/* This function returns a boolean value from the configuration */
extern int loki_getconfig_bool(const char *key);

/* This function returns an int value from the configuration */
extern int loki_getconfig_int(const char *key);

/* This function returns a float value from the configuration */
extern double loki_getconfig_float(const char *key);

/* This function only modifies the run-time config hashtable */
extern void loki_insertconfig(const char *key, const char *value);

/* This function deletes a key and value pair from the runtime hashtable */
extern void loki_deleteconfig(const char *key);

/* This function writes the current configuration to a parsable INI file */
extern void loki_writeconfig(const char *file);

/* This function returns the subset of argv[] that holds all the non-option arguments.
   This must be called AFTER loki_initialize() for it to make any sense. */
extern char **loki_getarguments(void);

#ifdef __cplusplus
};
#endif

#endif /* _LOKI_CONFIG_H */
