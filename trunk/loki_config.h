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

/* This registers a new command-line option switch.
   This functions needs to be called BEFORE any call to loki_parseargs or loki_initialize
 */
extern void loki_registeroption(const char *lng, char sht, const char *comment);

/* This function parses command line arguments to finalize the config.
   This function is called by loki_initialize().
*/
extern void loki_parseargs(int argc, char *argv[], const char *extra_help);

/* This function returns a string value from the configuration */
extern char *loki_getconfig_str(const char *key);

/* This function returns a boolean value from the configuration */
extern int loki_getconfig_bool(const char *key);

/* This function returns an int value from the configuration */
extern int loki_getconfig_int(const char *key);

/* This function returns a float value from the configuration */
extern double loki_getconfig_float(const char *key);

/* This function only modifies the run-time config hashtable */
extern void loki_insertconfig(const char *key, const char *value);

#ifdef __cplusplus
};
#endif

#endif /* _LOKI_CONFIG_H */
