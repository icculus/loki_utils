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

#include <stdio.h>
#include <unistd.h>
#include "loki_utils.h"

#define QAGENT	"loki_qagent"

void loki_runqagent(const char *stack_trace_file)
{
	int i;
	const char *argv[10];

	/* Build the command line for the QAgent script */
	i = 0;
	argv[i++] = QAGENT;
	argv[i++] = "--game_name";
	argv[i++] = loki_getgamename();
	argv[i++] = "--game_data";
	argv[i++] = loki_getdatapath();
	argv[i++] = "--game_prefs";
	argv[i++] = loki_getprefpath();
	if ( stack_trace_file && *stack_trace_file ) {
		argv[i++] = "--game_stack";
		argv[i++] = stack_trace_file;
	}
	argv[i] = 0;

	/* Run it, and complain if we can't */
	execvp(QAGENT, argv);
	fprintf(stderr, "Unable to execute " QAGENT " - exiting\n");
	_exit(-1);
}
