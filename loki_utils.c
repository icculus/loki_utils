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
#include "loki_utils.h"

/* This function parses command line arguments and initializes the utility
   functions that most Loki games use.
*/
void loki_initialize(int argc, char *argv[], const char *extra_help)
{
    /* Make this semi-safe if we're set-uid root - not a security fix!! */
    seteuid(getuid());

    /* Initialize crash handlers */
    loki_initsignals();

    /* Initialize program data directories */
    loki_initpaths(argv[0]);

    /* Load default user configuration */
    loki_initconfig();

    /* Handle command line parsing */
    loki_parseargs(argc, argv, extra_help);
}

