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
#include <sys/param.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <pwd.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <mntent.h>
#include <assert.h>

/* Prototype header */
#include "loki_utils.h"

/* A short game name, could be used as argv[0] */
static char game_name[100] = "";
static char game_versionstring[100] = "";
static char game_description[256] = "";

/* The directory where the data files can be found (CD-ROM) */
static char cdrompath[PATH_MAX];

/* The directory where the data files can be found (run directory) */
static char datapath[PATH_MAX];

/* The directory where user preferences can be found (home directory) */
static char prefpath[PATH_MAX];

/* The function to be called if we prompt for the CD */
static loki_prompt_func prompt_func = NULL;

char *loki_gethomedir(void)
{
    char *home = NULL;

    home = getenv("HOME");
    if ( home == NULL ) {
        uid_t id = getuid();
        struct passwd *pwd;

        setpwent();
        while( (pwd = getpwent()) != NULL ) {
            if( pwd->pw_uid == id ) {
                home = pwd->pw_dir;
                break;
            }
        }
        endpwent();
    }
    if ( home == NULL ) {
        home = ".";
    }
    return home;
}

/* Equivalent to the Win32 SplitPath() call */
void loki_splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    char *copy = strdup(path), *ptr;

    if ( drive ) {
        strcpy(drive, "");
    }
    ptr = strrchr(copy, '/');
    if ( !ptr ) {
        ptr = strrchr(copy, '\\');
    }
    if(ptr) {
        strcpy(fname, ptr+1);
        *ptr = '\0';
        if ( dir ) {
            strcpy(dir, copy);
            strcat(dir,"/");
        }
    } else {
        if ( dir ) {
            *dir = '\0';
        }
        strcpy(fname,copy);
    }
    ptr = strrchr(fname, '.');
    if(ptr) {
        strcpy(ext, ptr);
        *ptr = '\0';
    } else 
        *ext = '\0';
    free(copy);
}

/* Must be called BEFORE loki_initialize */
void loki_setgamename(const char *name, const char *version, const char *description)
{
#ifndef __TEST_MAIN
    if ( strcmp(name+strlen(name)-5, "_demo") == 0 ) {
        loki_isdemo(1);
    }
#endif
    strncpy(game_name, name, sizeof(game_name));
    strncpy(game_versionstring, version, sizeof(game_versionstring));
    sprintf(game_description, "%s %s", description, version);
}

const char *loki_getgamename(void)
{
    return game_name;
}
const char *loki_getgameversion(void)
{
    return game_versionstring;
}
const char *loki_getgamedescription(void)
{
    return game_description;
}


/* 
    This function gets the directory containing the running program.
    argv0 - the 0'th argument to the program
*/
void loki_initpaths(char *argv0)
{
    char temppath[PATH_MAX], env[100];
    char *home, *ptr, *data_env;

    home = loki_gethomedir();
    if(*game_name == 0) /* Game name defaults to argv[0] */
      loki_setgamename(argv0, "0.1", "");

    strcpy(temppath, argv0);  /* If this overflows, it's your own fault :) */
    if ( ! strrchr(temppath, '/') ) {
      char *path;
      char *last;
      int found;

      found = 0;
      path = getenv("PATH");
      do {
        /* Initialize our filename variable */
        temppath[0] = '\0';

        /* Get next entry from path variable */
        last = strchr(path, ':');
        if ( ! last )
          last = path+strlen(path);

        /* Perform tilde expansion */
        if ( *path == '~' ) {
          strcpy(temppath, home);
          ++path;
        }

        /* Fill in the rest of the filename */
        if ( last > (path+1) ) {
          strncat(temppath, path, (last-path));
          strcat(temppath, "/");
        }
        strcat(temppath, "./");
        strcat(temppath, argv0);

        /* See if it exists, and update path */
        if ( access(temppath, X_OK) == 0 ) {
          ++found;
        }
        path = last+1;

      } while ( *last && !found );

    } else {
      /* Increment argv0 to the basename */
      argv0 = strrchr(argv0, '/')+1;
    }

    /* Now canonicalize it to a full pathname for the data path */
    if ( realpath(temppath, datapath) ) {
      /* There should always be '/' in the path */
      *(strrchr(datapath, '/')) = '\0';
    }

    strcpy(env, game_name);

    for(ptr = env; *ptr; ptr++)
      *ptr = toupper(*ptr);
    strcat(env,"_DATA");

    data_env = getenv(env);
    if(data_env)
      strncpy(datapath, data_env, PATH_MAX);

    /* Create the preferences directory, if needed */
    sprintf(prefpath, "%s/.loki/", home);
    if ( access(prefpath, W_OK) != 0 ) {
        printf("Creating Loki preferences directory: %s\n", prefpath);
        mkdir(prefpath, 0700);
    }
    strcat(prefpath, game_name);
    if ( access(prefpath, W_OK) != 0 ) {
        printf("Creating %s preferences directory: %s\n", game_name, prefpath);
        mkdir(prefpath, 0700);
    }
}

int loki_hascdrompath(void)
{
    int has_cdrom;

    if ( cdrompath[0] ) {
        has_cdrom = 1;
    } else {
        has_cdrom = 0;
    }
    return(has_cdrom);
}

void loki_setcdrompath(const char *path)
{
    strncpy(cdrompath, path, PATH_MAX);
    cdrompath[PATH_MAX-1] = '\0';
}

void  loki_cdpromptfunction (loki_prompt_func func)
{
    prompt_func = func;
}

char *loki_getprefpath(void)
{
    return(prefpath);
}

char *loki_getdatapath(void)
{
    return(datapath);
}

char *loki_getcdrompath(void)
{
    return(cdrompath);
}

char *loki_getdatafile(const char *file, char *filepath, int maxpath)
{
    strncpy(filepath, loki_getdatapath(), maxpath);
    strncat(filepath, "/", maxpath);
    strncat(filepath, file, maxpath);
    if ( (access(filepath, R_OK) != 0) && loki_hascdrompath() ) {
        strncpy(filepath, loki_getcdrompath(), maxpath);
        strncat(filepath, "/", maxpath);
        strncat(filepath, file, maxpath);
    }
    return filepath;
}

char *loki_promptdatafile(const char *file, char *filepath, int maxpath)
{
    static int in_prompt = 0;

    strncpy(filepath, loki_getdatapath(), maxpath);
    strncat(filepath, "/", maxpath);
    strncat(filepath, file, maxpath);
    if ( (access(filepath, R_OK) != 0) && loki_hascdrompath() ) {
        strncpy(filepath, loki_getcdrompath(), maxpath);
        strncat(filepath, "/", maxpath);
        strncat(filepath, file, maxpath);
        if ( (access(filepath, R_OK) != 0) && prompt_func && !in_prompt ) {
            in_prompt = 1;
            while(! (*prompt_func)(file) );
            in_prompt = 0;
        }
    }
    return filepath;
}

char *loki_getpreffile(const char *file, char *filepath, int maxpath)
{
    strncpy(filepath, loki_getprefpath(), maxpath);
    strncat(filepath, "/", maxpath);
    strncat(filepath, file, maxpath);
    return filepath;
}

char *loki_getcdromfile(const char *file, char *filepath, int maxpath)
{
    /* The CDROM path must have been set */
    strncpy(filepath, loki_getcdrompath(), maxpath);
    strncat(filepath, "/", maxpath);
    strncat(filepath, file, maxpath);
    return filepath;
}

size_t loki_getavailablespace(const char *path)
{
    struct statfs buf;
    if(statfs(path,&buf))
        return 0;
    return ((long long)buf.f_bsize * buf.f_bavail) / 1024;
}

/* Code to determine the mount point of a CD-ROM */
int loki_getmountpoint(const char *device, char *mntpt, int max_size)
{
    char devpath[PATH_MAX], mntdevpath[PATH_MAX];
    FILE * mountfp;
    struct mntent *mntent;
    int mounted;

    /* Nothing to do with no device file */
    if( device == NULL ){
        *mntpt = '\0';
        return -1;
    }

    /* Get the fully qualified path of the CD-ROM device */
    if ( realpath(device, devpath) == NULL ) {
        perror("realpath() on your CD-ROM failed");
        return(-1);
    }

    /* Get the mount point */
    mounted = -1;
    memset(mntpt, 0, max_size);
    mountfp = setmntent( _PATH_MOUNTED, "r" );
    if( mountfp != NULL ) {
        mounted = 0;
        while( (mntent = getmntent( mountfp )) != NULL ){
            char *tmp, mntdev[1024];

            strcpy(mntdev, mntent->mnt_fsname);
            if ( strcmp(mntent->mnt_type, "supermount") == 0 ) {
                tmp = strstr(mntent->mnt_opts, "dev=");
                if ( tmp ) {
                    strcpy(mntdev, tmp+strlen("dev="));
                    tmp = strchr(mntdev, ',');
                    if ( tmp ) {
                        *tmp = '\0';
                    }
                }
            }
            if( strncmp(mntdev, "/dev", 4) || 
                realpath(mntdev, mntdevpath) == NULL ) {
                continue;
            }
            if( strcmp( mntdevpath, devpath ) == 0 ){
                mounted = 1;
                assert(strlen( mntent->mnt_dir ) < max_size);
                strncpy( mntpt, mntent->mnt_dir, max_size-1);
                mntpt[max_size-1] = '\0';
                break;
            }
        }
        endmntent( mountfp );
    }
    return(mounted);
}

#ifdef __TEST_MAIN
int main(int argc, char *argv[])
{
    char cdrom[1024];

    loki_setgamename("test", "1.0");
    loki_initpaths(argv[0]);
    printf("Data path: %s\n", loki_getdatapath());
    printf("Pref path: %s\n", loki_getprefpath());
    if ( loki_getmountpoint("/dev/cdrom", cdrom, sizeof(cdrom)) ) {
        printf("CD-ROM at: %s\n", cdrom);
    } else {
        printf("CD-ROM not detected\n");
    }
    exit(0);
}
#endif
