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

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "loki_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Set the name of the game - should be called before loki_initialize() */
extern void loki_setgamename(const char *name, const char *version, const char *description);

/* Get the game name - can be called anytime after loki_setgamename() */
extern const char *loki_getgamename(void);

/* Get the game version - can be called anytime after loki_setgamename() */
extern const char *loki_getgameversion(void);

/* Get the game description - can be called anytime after loki_setgamename() */
extern const char *loki_getgamedescription(void);

/* This initializes the library without parsing the command-line arguments */
extern void loki_initialize_noparse(int argc, char *argv[]);

/* This function parses command line arguments and initializes the
   utility functions that most Loki games use.
*/
extern void loki_initialize(int argc, char *argv[], const char *extra_help);

/* This function initializes crash signal handlers */
extern void loki_initsignals(void);

/* This function prints out a stack trace to standard error as well as
   the given log file descriptor (use -1 to disable secondary logging).
   'level' should usually be 0.
 */
extern void loki_printstack(int level, int log);

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

/* This function runs the Loki update agent and never returns.
   The update agent should verify the installed version, possibly
   update the install, and prompt to restart the game.

   If argc and argv are non-zero, the game will be restarted with
   the given options after the update process is complete.  The '-u'
   and '--update' options are stripped out before being passed to
   the update agent.
*/
extern void loki_runupdate(int argc, char *argv[]);

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

/* File functions which look in the prefs path for write access,
   and the prefs path then the data path then the cdrom path for write access.
*/
extern int loki_stat(const char *file, struct stat *statb);
extern FILE *loki_fopen(const char *file, const char *mode);
extern int loki_open(const char *file, int flags, mode_t mode);

/* Returns the available disk space in kilobytes on the filesystem that contains "path" */
extern size_t loki_getavailablespace(const char *path);

/* This function determines the mount point for a CD-ROM device */
extern int loki_getmountpoint(const char *device, char *mntpt, int max_size);

/* This function determines the primary IP address for the system
   Note: This function returns a 32-bit IPv4 address in network byte order,
         or 0xFFFFFFFF if it cannot determine a primary IP address.
 */
extern unsigned int loki_getprimaryIP(void);

/* This function launches the user's web browser with the given URL.
   The browser detection can be overridden by the LOKI_BROWSER environment
   variable, which is used as the format string: %s is replaced with the
   URL to be launched.
   This function returns -1 if a browser could not be found, or the return
   value of system("browser command") if one is available.
   There is no way to tell whether or not the URL was valid.

   WARNING: This function should NOT be called when a video mode is set.
 */
extern int loki_launchURL(const char *url);

#ifdef __cplusplus
};
#endif

#endif _LOKI_UTILS_H
