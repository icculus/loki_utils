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

#ifndef _LOKI_UTILS_H
#define _LOKI_UTILS_H

/* For size_t. */
#include <stddef.h>

#include "loki_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Set the name of the game - should be called before loki_initialize() */
extern void loki_setgamename(const char *n);

/* Get the game name - can be called anytime after loki_setgamename() */
extern const char *loki_getgamename(void);

/* This initializes the library without parsing the command-line arguments */
extern void loki_initialize_noparse(int argc, char *argv[]);

/* This function parses command line arguments and initializes the
   utility functions that most Loki games use.
*/
extern void loki_initialize(int argc, char *argv[], const char *extra_help);

/* This function initializes crash signal handlers */
extern void loki_initsignals(void);

/* This function sets function that is called to clean up the application
   after a fatal signal is caught and handled.  If the application causes
   a fatal signal while this function is called, it calls _exit(-1);
   The cleanup function is prototyped:
	void cleanup(void);
*/
extern void loki_signalcleanup(void (*cleanup)(void));

/* This function runs the Loki support agent and never returns.
   If you have a stack trace file, pass the pathname to this function.
*/
extern void loki_runqagent(const char *stack_trace_file);

/* This function simulates a debugger breakpoint (crashes if not in GDB!) */
#ifndef NDEBUG
#ifdef __i386
#define loki_breakdebugger() { __asm__("int $03"); }
#else
/* We define this as a function so that we don't have to include
   signals.h in programs for the SIGTRAP definition */
extern void loki_breakdebugger(void);
#endif
#else
#define loki_breakdebugger()
#endif

/* These functions are used to bracket sections of code that may need root privileges */
extern void loki_acquireroot(void);
extern void loki_releaseroot(void);

/* These functions tell whether the CD-ROM path has been set */
extern int loki_hascdrompath(void);
extern void loki_setcdrompath(const char *path);

/* These functions get the data directories for the running program */
extern void loki_initpaths(char *argv0);  /* Called by loki_initialize() */
extern char *loki_getprefpath(void);
extern char *loki_getdatapath(void);
extern char *loki_getcdrompath(void);

/* Callback function to be called to prompt for the CD.
   The return value is a boolean indicating whether to prompt again
   (true = don't prompt again)
 */
typedef int (*loki_prompt_func) (const char *file);

/* Function to be called when a file could not be found either in
   the installation path or on the CD. If func is NULL, the prompt is
   disabled */
extern void  loki_cdpromptfunction (loki_prompt_func func);

/* Returns the absolute path of a data file under datapath */
extern char *loki_getdatafile(const char *file, char *filepath, int maxpath);
/* The same, but its presence is necesary and the CDROM is prompted if needed */
extern char *loki_promptdatafile(const char *file, char *filepath, int maxpath);
/* The same for a preferences file */
extern char *loki_getpreffile(const char *file, char *filepath, int maxpath);
/* And to get a file from the CDROM, if its path has been set */
extern char *loki_getcdromfile(const char *file, char *filepath, int maxpath);

/* This function returns the home directory for the current user */
extern char *loki_gethomedir(void);

/* Equivalent to the Win32 SplitPath() call */
void loki_splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);

/* Returns the available disk space in kilobytes on the filesystem that contains "path" */
extern size_t loki_getavailablespace(const char *path);

/* This function determines the mount point for a CD-ROM device */
extern int loki_getmountpoint(const char *device, char *mntpt, int max_size);

/* This function determines the primary IP address for the system
   Note: This function returns a 32-bit IPv4 address in network byte order,
         or 0xFFFFFFFF if it cannot determine a primary IP address.
 */
extern unsigned int loki_getprimaryIP(void);

#ifdef __cplusplus
};
#endif

#endif _LOKI_UTILS_H
