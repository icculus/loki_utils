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

#include <sys/param.h>                /* For MAXHOSTNAMELEN */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>

#include "loki_utils.h"

/* Auxiliary function for getting the primary IP from the system */
static unsigned int loki_getifconfigaddr(void)
{
    const char *tag = "inet addr:";
    const char *bad_addrs[] = {
        "127.0.0", "0.0.0.0", NULL
    };
    FILE *pipe;
    char line[BUFSIZ];
    char *hostip, *ptr;
    int i;
    unsigned int address;

    address = INADDR_NONE;
    pipe = popen("/sbin/ifconfig", "r");
    if ( pipe ) {
        while ( (INADDR_NONE == address) && fgets(line, BUFSIZ-1, pipe) ) {
            ptr = strstr(line, tag);
            if (ptr) {
                ptr += strlen(tag);
                while ( *ptr && isspace(*ptr) )
                    ++ptr;
                hostip = ptr;
                while ( *ptr && !isspace(*ptr) )
                    ++ptr;
                *ptr = '\0';
                /* Check to see if address should be ignored */
                for ( i=0; bad_addrs[i]; ++i ) {
                    if (strncmp(hostip,bad_addrs[i],strlen(bad_addrs[i])) == 0)
                        break;
                }
                if ( ! bad_addrs[i] ) {
                    address = inet_addr(hostip);
                }
            }
        }
        pclose(pipe);
    }
    return(address);
}

/* This function returns the primary IP address of this system */
unsigned int loki_getprimaryIP(void)
{
    unsigned int address;

    /* Check the Loki configuration, then the system information */
    address = inet_addr(loki_getconfig_str("ipaddress"));
    if ( INADDR_NONE == address ) {
        char my_name[MAXHOSTNAMELEN];

        if ( gethostname(my_name, MAXHOSTNAMELEN) == 0 ) {
            struct hostent *hp = gethostbyname(my_name);
            if (NULL == hp) {
                address = inet_addr(my_name);
            } else {
                memcpy(&address, hp->h_addr_list[0], hp->h_length);
            }
        }
    }
    if ( INADDR_NONE == address ) {
        address = loki_getifconfigaddr();
    }
    return(address);
}
