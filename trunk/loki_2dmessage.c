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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "loki_2dmessage.h"
#include "SDL.h"

#ifdef STANDALONE
#include "sdl_utils.h"
#endif

extern unsigned char loki_fontchars[][8];

static int texty = 0;

/*
 *
 *  returns -1 on error, 0 on success.
 */
int loki_2dmsg_initialize(float bgr, float bgg, float bgb, float bga)
{
    SDL_Surface *surface = SDL_GetVideoSurface();

    texty = 0;

    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

        // !!! This is always color zero.
    memset(surface->pixels, '\0', surface->h * surface->pitch);

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    SDL_UpdateRect(surface, 0, 0, 0, 0);

    return(0);
}


static inline void loki_2dmsg_internal_printChar(int x, int y,
                                                 SDL_Surface *surface,
                                                 unsigned char ch)
{
    int i;
    int j;
    unsigned char *dest;
    unsigned char *src;

    if (ch > 127)
        return;

    src = loki_fontchars[ch];

    for (i = 8; i >= 0; i--)
    {
        dest = surface->pixels +
              ((surface->pitch * y) + (x + surface->format->BytesPerPixel));

        for (j = 128; j >= 1; j >>= 1)
        {
            if (src[i] & j)
                memset(dest, 0xFF, surface->format->BytesPerPixel);  // ugh! !!!
            dest += surface->format->BytesPerPixel;
        } // for

        y++;
    } // for
}

static inline void loki_2dmsg_internal_printString(int x, int y,
                                                   SDL_Surface *surface,
                                                   const char *str)
{
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

    for (; *str != '\0'; str++, x += 15)
        loki_2dmsg_internal_printChar(x, y, surface, *str);

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    SDL_UpdateRect(surface, 0, 0, 0, 0);
}


void loki_2dmsg_print(float r, float g, float b, const char *str)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    int textw = ((int) surface->w - ((int) (strlen(str) * 15))) / 2;

    #ifdef STANDALONE
        printf("loki_2dmsg_print(): Should write this to the frame buffer:\n"
               "\"%s\"\n", str);
    #endif

    loki_2dmsg_internal_printString(textw, texty, surface, str);

    texty += 15;
    if (texty <= 0)
        texty = 0;
}


#ifdef STANDALONE  // for testing only.  --ryan.
int main(int argc, char **argv)
{
    SDL_Event event;
    int quit_flag = 0;

    SDL_Init(SDL_INIT_VIDEO);
//    SDL_SetVideoMode(320, 240, 16, 0);
    SDL_SetVideoMode(640, 480, 16, 0);
    SDL_WM_SetCaption("loki_2dmessage.c test.", "loki_glmsg");

    if (loki_2dmsg_initialize(0.0, 0.0, 0.0, 0.0) == -1)
    {
        fprintf(stderr, "loki_2dmsg_initialize() failed!\n");
        SDL_Quit();
        return(2);
    } // if

/*
    loki_2dmsg_print(1.0, 1.0, 1.0, "This is just a test");
    loki_2dmsg_print(1.0, 1.0, 1.0, "do not panic");
    loki_2dmsg_print(1.0, 0.0, 0.0, "red");
    loki_2dmsg_print(0.0, 1.0, 0.0, "green");
    loki_2dmsg_print(0.0, 0.0, 1.0, "blue");
*/

    sdl_ShowMessage("Your OpenGL driver is too slow to play this game.\n"
                    "Driver used: [ ~/projects/Mesa-3.2/lib/libGL.so ]\n"
                    "Please change your driver!\n"
                    "Email support@lokigames.com for help,\n"
                    "or call 1-714-508-2140 (9-5 PM US Pacific Time).\n");

    while (quit_flag == 0)
    {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT)
            quit_flag = 1;
    } // while

    SDL_Quit();
    return(0);
} // main
#endif  // STANDALONE

/* end of loki_2dmessage.c ... */

