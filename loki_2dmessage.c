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
int loki_2dmsg_initialize(float bgr, float bgg, float bgb)
{
    SDL_Surface *surface = SDL_GetVideoSurface();

    texty = 0;

    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0, 0, 0));

    return(0);
}


static inline void loki_2dmsg_internal_printChar(int x, int y,
                                                 SDL_Surface *surface,
                                                 unsigned char ch,
                                                 Uint32 color)
{
    int bpp;
    int i;
    int j;
    unsigned char *dest;
    unsigned char *src;

    if (ch > 127)
        return;

    bpp = surface->format->BytesPerPixel;

    src = loki_fontchars[ch];

    for (i = 7; i >= 0; i--)
    {
        dest = surface->pixels + ((surface->pitch * y) + (x * bpp));

        for (j = 128; j > 0; j >>= 1)
        {
            if (src[i] & j) {
                switch(bpp) {
                    case 1:
                        *(Uint8 *)dest = color;
                        break;
                    case 2:
                        *(Uint16 *)dest = color;
                        break;
                    case 3:
                    case 4:
                        *(Uint32 *)dest = color;
                        break;
                }
            }
            dest += bpp;
        } // for

        y++;
    } // for
}

static inline void loki_2dmsg_internal_printString(int x, int y,
                                                   SDL_Surface *surface,
                                                   const char *str,
                                                   Uint32 color)
{
    if (SDL_MUSTLOCK(surface))
        SDL_LockSurface(surface);

    for (; *str != '\0'; str++, x += 10)
        loki_2dmsg_internal_printChar(x, y, surface, *str, color);

    if (SDL_MUSTLOCK(surface))
        SDL_UnlockSurface(surface);

    SDL_UpdateRect(surface, 0, 0, 0, 0);
}


void loki_2dmsg_print(float r, float g, float b, const char *str)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    Uint32 color;
    int textw = ((int) surface->w - ((int) (strlen(str) * 10))) / 2;

    #ifdef STANDALONE
        printf("loki_2dmsg_print(): Should write this to the frame buffer:\n"
               "\"%s\"\n", str);
    #endif

    color = SDL_MapRGB(surface->format, r*255.0, g*255.0, b*255.0);
    loki_2dmsg_internal_printString(textw, texty, surface, str, color);

    texty += 15;
    if (texty <= 0)
        texty = 0;
}


#ifdef STANDALONE  // for testing only.  --ryan.
static inline void do_delay(int ms)
{
    SDL_Event event;
    Uint32 endtime = SDL_GetTicks() + ms;

    while (1)
    {
        if ((ms >= 0) && (SDL_GetTicks() >= endtime))
            return;

        if (SDL_PollEvent(&event) == 0)
            SDL_Delay(1);
        else
        {
            if (event.type == SDL_QUIT)
                return;
        } // else
    } // while
} // do_delay


static inline void do_test(int w, int h)
{
    SDL_SetVideoMode(w, h, 16, 0);
    SDL_WM_SetCaption("loki_2dmessage.c test.", "loki_2dmsg");

    if (loki_2dmsg_initialize(0.0, 0.0, 0.0) == -1)
    {
        fprintf(stderr, "loki_2dmsg_initialize() failed!\n");
        SDL_Quit();
        exit(2);
    } // if

    loki_2dmsg_print(1.0, 1.0, 1.0, "This is just a test");
    loki_2dmsg_print(1.0, 1.0, 1.0, "do not panic");
    loki_2dmsg_print(1.0, 0.0, 0.0, "red");
    loki_2dmsg_print(0.0, 1.0, 0.0, "green");
    loki_2dmsg_print(0.0, 0.0, 1.0, "blue");

    do_delay(5000);

    sdl_ShowMessage("Your OpenGL driver is too slow to play this game.\n"
                    "Driver used: [ ~/projects/Mesa-3.2/lib/libGL.so ]\n"
                    "Please change your driver!\n"
                    "Email support@lokigames.com for help,\n"
                    "or call 1-714-508-2140 (9-5 PM US Pacific Time).\n");

    do_delay(5000);
}


int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    do_test(640, 480);
    do_test(320, 240);
    do_test(1600, 1200);

    SDL_Quit();
    return(0);
} // main
#endif  // STANDALONE

/* end of loki_2dmessage.c ... */

