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

/* INI library: I/O with Windows-style INI files */

#ifndef _LOKI_INIFILE_H_
#define _LOKI_INIFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct _loki_ini_file_t; /* Defined in loki_inifile.c */

typedef struct _loki_ini_file_t ini_file_t;

typedef int (*ini_callback_t)(ini_file_t *ini, 
                              const char *section, const char *key, const char *value,
                              void *param);

/* Open and loads the INI file, returns INI object or NULL if failed */
ini_file_t * loki_openinifile(const char *path);

/* Create a new INI file from scratch */
ini_file_t * loki_createinifile(const char *path);

/* Close the INI file, returns error code */
int loki_closeinifile(ini_file_t *ini);

/* Return the string corresponding to a key in the specified section of the file,
   returns NULL if could not find it */
const char *loki_getinistring(ini_file_t *ini, const char *section, const char *key);

/* Add or modify a keyed value in the INI file, returns error code */
int loki_putinistring(ini_file_t *ini, const char *section, const char *key, const char *value);

/* Returns a boolean value indicating if the image of the INI file in memory is in sync
   with the file on disc */
int loki_inihaschanged(ini_file_t *ini);

/* Write the INI file back to disk, returns error code
   If 'path' is NULL, then the original file name is used by default.
 */
int loki_writeinifile(ini_file_t *ini, const char *path);

	/******** Section Enumeration Functions ********/
	
/* Returns the name of the fist section of the given file (initializes an internal iterator) */
const char *loki_begin_inisection(ini_file_t *ini);

/* Returns the name of the next section of the given file, or NULL if no more sections
   This function must be called _AFTER_ loki_begin_inisection()
 */
const char *loki_next_inisection(ini_file_t *ini);
	
	/******** Line Enumeration Functions **********/

struct _loki_ini_line_t; /* Private type */

typedef struct _loki_ini_line_t ini_line_t;

/* Initialize the iterator to the beginning of the given section.
   Returns NULL if the section does not exist.
 */
ini_line_t *loki_begin_iniline(ini_file_t *ini, const char *section);

/* Get the current key/value pair pointed to by the iterator.
   Returns a positive value if everything was OK, or zero else.
 */
int loki_get_iniline(ini_line_t *iterator, const char **key, const char **value);

/* Update the value of a line */
int loki_update_iniline(ini_line_t *iterator, const char *value);

/* Remove a specified key in a section. If section is empty, it will be removed as well */
int loki_remove_iniline(ini_file_t *ini, const char *section, const char *key);

/* Remove the current line; the iterator is changed to point to the next line if available */
int loki_remove_current_iniline(ini_line_t *iterator);

/* Iterator to the next line of the section.
   Returns zero when at the end of the section or an error occured.
 */
int loki_next_iniline(ini_line_t *iterator);

/* Free the iterator object allocated by loki_begininisection.
   Must be called when the user is done with the iterator.
 */
void loki_free_iniline(ini_line_t *iterator);

/* Iterate through all lines of a section with a callback function */

int loki_iterate_iniline(ini_file_t *ini, const char *section, ini_callback_t func, void *param);

#ifdef __cplusplus
};
#endif

#endif

