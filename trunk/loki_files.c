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
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "loki_utils.h"


int loki_stat(const char *file, struct stat *statb)
{
    char path[PATH_MAX];
    int pass;
    int value;

    /* If it's a full pathname, we're fine */
    if ( *file == '/' ) {
        return stat(file, statb);
    }

    /* First look in preferences, then in data and cdrom directories */
    value = -1;
    for ( pass = 0; (value < 0) && (pass < 4); ++pass ) {
        switch (pass) {
            case 0:
                strcpy(path, loki_getprefpath());
                break;
            case 1:
                strcpy(path, loki_getdatapath());
                break;
            case 2:
                strcpy(path, loki_getcdrompath());
                break;
            case 3:
                strcpy(path, ".");
                break;
        }
        if ( path[0] ) {
            strcat(path, "/");
            strcat(path, file);
            value = stat(path, statb);
        }
    }
    return value;
}

/* Create the directories in the heirarchy above this path, if necessary */
static int mkdirhier(const char *path)
{
    int retval;
    char *bufp, *new_path;
    struct stat sb;

    retval = 0;
    if ( path && *path ) {
        new_path = strdup(path);
        for ( bufp=new_path+1; *bufp; ++bufp ) {
            if ( *bufp == '/' ) {
                *bufp = '\0';
                if ( stat(new_path, &sb) < 0 ) {
                    retval = mkdir(new_path, 0755);
                }
                *bufp = '/';
            }
        }
        free(new_path);
    }
    return(retval);
}

FILE *loki_fopen(const char *file, const char *mode)
{
    char path[PATH_MAX];
    int pass;
    FILE *value;

    /* If it's a full pathname, we're fine */
    if ( *file == '/' ) {
        return fopen(file, mode);
    }

    /* If we're writing, we must write to the preferences */
    if ( (*mode == 'w') || (*mode == 'a') ) {
        sprintf(path, "%s/%s", loki_getprefpath(), file);
        mkdirhier(path);
        value = fopen(path, mode);
    } else {
        /* First look in preferences, then in data directory */
        value = 0;
        for ( pass = 0; !value && (pass < 4); ++pass ) {
            switch (pass) {
                case 0:
                    strcpy(path, loki_getprefpath());
                    break;
                case 1:
                    strcpy(path, loki_getdatapath());
                    break;
                case 2:
                    strcpy(path, loki_getcdrompath());
                    break;
                case 3:
                    strcpy(path, ".");
                    break;
            }
            if ( path[0] ) {
                strcat(path, "/");
                strcat(path, file);
                value = fopen(path, mode);
            }
        }
    }
    return value;
}

/* A case insensitive open function */
static int open_nocase(const char *path, const char *file,
                       int flags, mode_t mode)
{
    char full_path[PATH_MAX];
    int value;

    snprintf(full_path, sizeof(full_path), "%s/%s", path, file);
    value = open(full_path, flags, mode);
    if ( value < 0 ) {
        char *base, *sep;
        DIR *dirp;
        struct dirent *entry;

        /* Get the base of the path */
        base = full_path + strlen(path) + 1;
        sep = strchr(base, '/');
        if ( sep ) {
            *sep = '\0';
            file += (sep-base)+1;
        } else {
            file = 0;
        }
        sep = strrchr(full_path, '/');
        *sep = '\0';

        dirp = opendir(full_path);
        if ( dirp ) {
            *sep = '/';
            while ( (value < 0) && ((entry=readdir(dirp)) != NULL) ) {
                if ( strcasecmp(entry->d_name, base) == 0 ) {
                    strcpy(base, entry->d_name);
                    if ( file ) {
                        value = open_nocase(full_path, file, flags, mode);
                    } else {
                        value = open(full_path, flags, mode);
                    }
                }
            }
            closedir(dirp);
        }
    }
    return value;
}

int loki_open(const char *file, int flags, mode_t mode)
{
    int value;

    /* If it's a full pathname, we're fine */
    if ( *file == '/' ) {
        mkdirhier(file);
        return open(file, flags, mode);
    }

    /* If we're writing, we must write to the preferences */
    if ( flags == O_RDONLY ) {
        char *path = 0;
        int pass;

        /* First look in preferences, then in data directory */
        value = -1;
        for ( pass = 0; (value < 0) && (pass < 4); ++pass ) {
            switch (pass) {
                case 0:
                    path = loki_getprefpath();
                    break;
                case 1:
                    path = loki_getdatapath();
                    break;
                case 2:
                    path = loki_getcdrompath();
                    break;
                case 3:
                    path = ".";
                    break;
            }
            if ( path[0] ) {
                value = open_nocase(path, file, O_RDONLY, 0);
            }
        }
    } else {
        char path[PATH_MAX];

        sprintf(path, "%s/%s", loki_getprefpath(), file);
        if ( flags & O_CREAT ) {
                mkdirhier(path);
        }
        value = open(path, flags, mode);
        if ( (value < 0) && (flags & O_RDWR) ) {
            /* Uh oh, need to copy from install path? */ ;
        }
    }
    return value;
}
