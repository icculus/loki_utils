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

#include <unistd.h>
#include <stdio.h>
#include "loki_utils.h"

//#define DEBUG_SETUID

static unsigned int root_cnt = 0;

/* This initializes the library without parsing the command-line arguments */
void loki_initialize_noparse(int argc, char *argv[])
{
    /* Make this semi-safe if we're set-uid root - not a security fix!! */
    loki_releaseroot();

    /* Initialize crash handlers */
    loki_initsignals();

    /* Initialize program data directories */
    loki_initpaths(argv[0]);

    /* Load default user configuration */
    loki_initconfig();
}

/* This function parses command line arguments and initializes the utility
   functions that most Loki games use.
*/
void loki_initialize(int argc, char *argv[], const char *extra_help)
{
	loki_initialize_noparse(argc, argv);

    /* Handle command line parsing */
    loki_parseargs(argc, argv, extra_help);
}

/* These two functions are used to bracket specific sections of
   code that may need root privileges.
   It uses a counter to track the number of calls, so seteuid()
   is called only when necessary.
*/
void loki_acquireroot(void)
{
  if(!root_cnt){
#ifdef DEBUG_SETUID
	fprintf(stderr,"Switching to root\n");
#endif
	seteuid(0);
  }
  root_cnt ++;
}

void loki_releaseroot(void)
{
  if(root_cnt > 0)
	root_cnt --;
  if(!root_cnt){
#ifdef DEBUG_SETUID
	fprintf(stderr,"Switching to user\n");
#endif
	seteuid(getuid());
  }
}
