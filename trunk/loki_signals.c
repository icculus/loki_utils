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
#include <stdio.h>
#include <features.h>
#if (__GLIBC__ >= 2)
#if (__GLIBC__ != 2) || (__GLIBC_MINOR__ >= 1)
#include <execinfo.h>
#else
#include "glibc-2.0/backtrace.c"
#endif
#define HAS_EXECINFO
#endif
#include <signal.h>

#include "loki_utils.h"

#ifdef LINUX_BETA
#define BUGS_EMAIL    "beta@lokigames.com"
#else
#define BUGS_EMAIL    "support@lokigames.com"
#endif

// Try to clean up gracefully on a signal, and terminate us if we fail
static void catch_signal(int sig)
{
    static int cleaning_up = 0;

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
#if 0
        // Kill all threads with no chance of surviving.
	// This kills the X session when run from the GNOME panel
	// since the X session is in the same process group.
        do {
            kill(0, SIGKILL);
        } while ( 1 );
#else
	_exit(-1);
#endif
        // Not reached -- we died just now.
    } else {
        cleaning_up = 1;
        fprintf(stderr, "\n");
        switch (sig) {
        case SIGHUP:
            fprintf(stderr, "Hangup signal caught, cleaning up.\n");
            break;
        case SIGQUIT:
            fprintf(stderr, "Interrupt signal caught, cleaning up.\n");
            break;
        case SIGABRT:
            fprintf(stderr, "BUG!  Assertion failed, cleaning up.\n");
            { extern char *game_version;
                fprintf(stderr, "%s", game_version);
                fprintf(stderr, "Built with glibc-%d.%d\n",
                        __GLIBC__, __GLIBC_MINOR__);
            }
            fprintf(stderr, "Please send the text of the failed assertion,\n"
                    "along with the contents of autosave to: "BUGS_EMAIL"\n");
            break;
        case SIGSEGV:
            fprintf(stderr, "BUG! (Segmentation Fault)  Going down hard...\n");
            { extern char *game_version;
                fprintf(stderr, "%s", game_version);
                fprintf(stderr, "Built with glibc-%d.%d\n",
                        __GLIBC__, __GLIBC_MINOR__);
            }
#ifdef HAS_EXECINFO
            { void *array[32]; int size, i;
                fprintf(stderr, "Stack dump:\n");
                fprintf(stderr, "{\n");
                size = backtrace(array, (sizeof array)/(sizeof array[0]));
                for ( i=0; i<size; ++i ) {
                    fprintf(stderr, "\t%p\n", array[i]);
                }
                fprintf(stderr, "}\n");
            }
#else
#warning Stack dump disabled.
#endif
            fprintf(stderr, "Please send a full bug report,\n"
                    "along with the contents of autosave to: "BUGS_EMAIL"\n");

            // Now kill any outstanding threads (network, etc.)
            do {
                kill(0, SIGKILL);
            } while ( 1 );
            // Not reached -- we died just now.
        default:
            fprintf(stderr, "Unknown signal (%d) caught, cleaning up.\n", sig);
            break;
        }
    }
    exit(sig);
}

void loki_initsignals(void)
{
    // Add signal handlers
    signal(SIGABRT, catch_signal);
    signal(SIGSEGV, catch_signal);
}
