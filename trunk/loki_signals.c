/*
    Loki Game Utility Functions
    Copyright (C) 1999  Loki Software, Inc.

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

#include <sys/types.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <features.h>
#if (__GLIBC__ >= 2)
#if (__GLIBC__ != 2) || (__GLIBC_MINOR__ >= 1)
#include <execinfo.h>
#else
#include "glibc-2.0/backtrace.c"
#endif
#define HAS_EXECINFO
#endif
#include <unistd.h>
#include <signal.h>

#include "loki_utils.h"

static void (*signal_cleanup)(void) = NULL;

/* Print a message to crash log and standard error.
   This may run while the heap is corrupted, so don't use malloc() and friends.
*/
static void print_crash(int log, const char *fmt, ...)
{
    char string[4096];
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(string, sizeof(string), fmt, ap);
    va_end(ap);
    if ( len > 0 ) {
        if ( log >= 0 ) {
            write(log, string, len);
        }
        write(2, string, len);
    }
}

void loki_printstack(int level, int log)
{
#ifdef HAS_EXECINFO
    void *array[64];
    int size;
    print_crash(log, "Stack dump:\n");
    print_crash(log, "{\n");
    size = backtrace(array, (sizeof array)/(sizeof array[0]));
#if (__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1))
    backtrace_symbols_fd(array, size, 2);
    if (log >= 0)
	backtrace_symbols_fd(array, size, log);
#else
    for ( i=2; i<size; ++i ) {
        print_crash(log, "\t%p\n", array[i]);
    }
#endif
    print_crash(log, "}\n");
#else
#warning Stack dump disabled.
#endif
}

// Try to clean up gracefully on a signal, and terminate us if we fail
static void catch_signal(int sig)
{
    static int fatal_crash = 0;
    static int cleaning_up = 0;
    /* Don't malloc the path since heap may be corrupted */
    static char logfile[PATH_MAX] = { 0 };

    if ( cleaning_up ) {
        // We received a signal during cleanup, probably due to unstable state
        // or the user being impatient.  Abort with extreme prejudice.
        switch (sig ) {
        case SIGABRT:
            fprintf(stderr, "Failed assertion in cleanup -- aborting\n");
            break;
        case SIGSEGV:
            // Don't bother warning user about an already unstable state
            //fprintf(stderr, "Crashed during cleanup -- aborting\n");
            break;
        default:
            fprintf(stderr, "Caught signal in cleanup -- aborting\n");
            break;
        }
    } else {
        int log;

        cleaning_up = 1;

        /* Create a log of the crash - write directly to it */
        if ( *loki_getprefpath() ) {
            strcpy(logfile, loki_getprefpath());
            strcat(logfile, "/stack-trace.txt");
        }
        log = open(logfile, O_WRONLY|O_CREAT|O_TRUNC, 0644);

        print_crash(log, "\n");
        switch (sig) {
        case SIGHUP:
            print_crash(log, "Hangup signal caught, cleaning up.\n");
            break;
        case SIGQUIT:
            print_crash(log, "Interrupt signal caught, cleaning up.\n");
            break;
        case SIGABRT:
            print_crash(log, "BUG!  Exception triggered, cleaning up.\n");
            print_crash(log, "%s\n", loki_getgamedescription());
            print_crash(log, "Built with glibc-%d.%d on %s\n",
                        __GLIBC__, __GLIBC_MINOR__, loki_getarch());
            loki_printstack(1, log);
            fatal_crash = 1;
#ifdef LINUX_BETA
            fprintf(stderr, "Please file a full bug report in Fenris,\n"
                    "at http://fenris.lokigames.com/\n");
#else
            fprintf(stderr, "Please send the text of the failed assertion,\n"
                    "along with the contents of autosave to: support@lokigames.com\n");
#endif
            break;
        case SIGSEGV:
            print_crash(log, "BUG! (Segmentation Fault)  Going down hard...\n");
            print_crash(log, "%s\n", loki_getgamedescription());
            print_crash(log, "Built with glibc-%d.%d on %s\n",
                        __GLIBC__, __GLIBC_MINOR__, loki_getarch());
            loki_printstack(1, log);
            fatal_crash = 1;

#ifdef LINUX_BETA
            fprintf(stderr, "Please file a full bug report in Fenris,\n"
                    "at http://fenris.lokigames.com/\n");
#else
            fprintf(stderr, "Please send a full bug report,\n"
                    "along with the contents of autosave to: support@lokigames.com\n");
#endif
            break;

        default:
            print_crash(log, "Unknown signal (%d) caught, cleaning up.\n", sig);
            break;
        }

        /* Close the log file */
        if ( log >= 0 ) {
            close(log);
        }

        /* Cleanup after catching fatal signal */
        if ( signal_cleanup ) {
            signal_cleanup();
        }
    }

    /* Run the Loki support QAgent on fatal crashes (never returns) */
    if ( fatal_crash ) {
        loki_runqagent(logfile);
    }
    /* Not a fatal crash, but we may have crashed in signal cleanup */
    if ( (sig == SIGABRT) || (sig == SIGSEGV) ) {
        _exit(sig);
    }
    exit(sig);
}

void loki_initsignals(void)
{
    // Add signal handlers
    signal(SIGHUP, catch_signal);
    signal(SIGQUIT, catch_signal);
    signal(SIGABRT, catch_signal);
    signal(SIGSEGV, catch_signal);
}

/* This function sets function that is called to clean up the application
   after a fatal signal is caught and handled.  If the application causes
   a fatal signal while this function is called, it calls _exit(-1);
   The cleanup function is prototyped:
    void cleanup(void);
*/
void loki_signalcleanup(void (*cleanup)(void))
{
    signal_cleanup = cleanup;
}

#ifndef __i386
void loki_breakdebugger(void)
{
    raise(SIGTRAP);
}
#endif
