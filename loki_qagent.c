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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "loki_utils.h"

#define QAGENT	"loki_qagent"
#define UPDATE	"loki_update"

void loki_runqagent(const char *stack_trace_file)
{
	int i;
	const char *args[16];

	/* Build the command line for the QAgent script */
	i = 0;
	args[i++] = QAGENT;
	args[i++] = "--product_name";
	args[i++] = loki_getgamename();
	args[i++] = "--product_version";
	args[i++] = loki_getgameversion();
	args[i++] = "--product_desc";
	args[i++] = loki_getgamedescription();
	args[i++] = "--product_path";
	args[i++] = loki_getdatapath();
	args[i++] = "--product_prefs";
	args[i++] = loki_getprefpath();
	args[i++] = "--product_cdrom";
	args[i++] = loki_getcdrompath();
	if ( stack_trace_file && *stack_trace_file ) {
		args[i++] = "--stack_trace";
		args[i++] = stack_trace_file;
	}
	args[i] = 0;

	/* Run it, and complain if we can't */
	execvp(QAGENT, args);
	fprintf(stderr, "Unable to execute " QAGENT " - exiting\n");
	_exit(-1);
}

void loki_runupdate(int argc, char *argv[])
{
	int i, j;
	const char *args[1024];

	/* Build the command line for the update script */
	i = 0;
	args[i++] = UPDATE;
	args[i++] = "--product_name";
	args[i++] = loki_getgamename();
	args[i++] = "--product_version";
	args[i++] = loki_getgameversion();
	args[i++] = "--product_desc";
	args[i++] = loki_getgamedescription();
	args[i++] = "--product_path";
	args[i++] = loki_getdatapath();
	args[i++] = "--product_prefs";
	args[i++] = loki_getprefpath();
	args[i++] = "--product_cdrom";
	args[i++] = loki_getcdrompath();

	/* Add the original command line, if desired */
	args[i++] = "--";
	for ( j=0; j<argc; ++j ) {
		/* Strip out the auto-update options */
		if ( (strcmp(argv[j], "-u") == 0) ||
		     (strcmp(argv[j], "--update") == 0) ) {
			continue;
		}
		args[i++] = argv[j];
	}
	args[i] = 0;

	/* Run it, and complain if we can't */
	execvp(UPDATE, args);
	fprintf(stderr, "Unable to execute " UPDATE " - exiting\n");
	_exit(-1);
}
