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

/*
 *  (From the font code:)
 *
 * Copyright (c) 1993-1997, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED 
 * Permission to use, copy, modify, and distribute this software for 
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that 
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission. 
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * US Government Users Restricted Rights 
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(R) is a registered trademark of Silicon Graphics, Inc.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <GL/gl.h>
#include "loki_glmessage.h"
#include "SDL.h"

#ifdef STANDALONE
#include "sdl_utils.h"
#endif

#ifdef LOKI_NO_GLMSG
#error You defined LOKI_NO_GLMSG, and are trying to compile GLmsg support!
#endif

extern unsigned char loki_fontchars[][8];

static int texty = 0;

struct loki_glmsg_funcs
{
    void (*glPixelStorei)( GLenum pname, GLint param );
    GLuint (*glGenLists)( GLsizei range );
    void (*glNewList)( GLuint list, GLenum mode );
    void (*glEndList)( void );
    void (*glCallLists)( GLsizei n, GLenum type,
                                     const GLvoid *lists );
    void (*glListBase)( GLuint base );
    void (*glBitmap)( GLsizei width, GLsizei height,
                                    GLfloat xorig, GLfloat yorig,
                                    GLfloat xmove, GLfloat ymove,
                                    const GLubyte *bitmap );
    void (*glPushAttrib)( GLbitfield mask );
    void (*glPopAttrib)( void );
    void (*glShadeModel)( GLenum mode );
    void (*glClear)( GLbitfield mask );
    void (*glClearColor)(GLclampf r, GLclampf g, GLclampf b, GLclampf alpha );
    void (*glColor4f)( GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha );
    void (*glRasterPos2i)( GLint x, GLint y );
    void (*glFlush)( void );
    void (*glViewport)( GLint x, GLint y, GLsizei width, GLsizei height );
    void (*glMatrixMode)( GLenum mode );
    void (*glLoadIdentity)( void );
    void (*glOrtho)( GLdouble left, GLdouble right,
                     GLdouble bottom, GLdouble top,
                     GLdouble near_val, GLdouble far_val );
    void (*glDisable)( GLenum cap );
    void (*glDisableClientState)( GLenum cap );
};

static int font_is_made = 0;
static GLuint fontOffset;

static struct loki_glmsg_funcs glfns;


static void loki_glmsg_internal_makeRasterFont(void)
{
   GLuint i;

   if (!font_is_made)
   {
       glfns.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

       fontOffset = glfns.glGenLists (128);
       for (i = 0; i < 128; i++) {
           glfns.glNewList(fontOffset + i, GL_COMPILE);
           glfns.glColor4f(0.0, 1.0, 0.0, 1.0);
           glfns.glBitmap(8, 8, 0.0, 2.0, 10.0, 0.0, loki_fontchars[i]);
           glfns.glEndList();
       }
       font_is_made = 1;
   }
}

static void loki_glmsg_internal_printString(const char *s)
{
   glfns.glPushAttrib(GL_LIST_BIT);
   glfns.glListBase(fontOffset);
   glfns.glCallLists((GLsizei) strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
   glfns.glPopAttrib();
}


void loki_glmsg_internal_initfuncs(void)
{
    glfns.glPixelStorei  = SDL_GL_GetProcAddress("glPixelStorei");
    glfns.glGenLists     = SDL_GL_GetProcAddress("glGenLists");
    glfns.glNewList      = SDL_GL_GetProcAddress("glNewList");
    glfns.glEndList      = SDL_GL_GetProcAddress("glEndList");
    glfns.glCallLists    = SDL_GL_GetProcAddress("glCallLists");
    glfns.glListBase     = SDL_GL_GetProcAddress("glListBase");
    glfns.glBitmap       = SDL_GL_GetProcAddress("glBitmap");
    glfns.glPushAttrib   = SDL_GL_GetProcAddress("glPushAttrib");
    glfns.glPopAttrib    = SDL_GL_GetProcAddress("glPopAttrib");
    glfns.glShadeModel   = SDL_GL_GetProcAddress("glShadeModel");
    glfns.glClear        = SDL_GL_GetProcAddress("glClear");
    glfns.glClearColor   = SDL_GL_GetProcAddress("glClearColor");
    glfns.glColor4f      = SDL_GL_GetProcAddress("glColor4f");
    glfns.glRasterPos2i  = SDL_GL_GetProcAddress("glRasterPos2i");
    glfns.glFlush        = SDL_GL_GetProcAddress("glFlush");
    glfns.glViewport     = SDL_GL_GetProcAddress("glViewport");
    glfns.glMatrixMode   = SDL_GL_GetProcAddress("glMatrixMode");
    glfns.glLoadIdentity = SDL_GL_GetProcAddress("glLoadIdentity");
    glfns.glOrtho        = SDL_GL_GetProcAddress("glOrtho");
    glfns.glDisable      = SDL_GL_GetProcAddress("glDisable");
    glfns.glDisableClientState = SDL_GL_GetProcAddress("glDisableClientState");
}


/*
 *
 *  returns -1 on error, 0 on success.
 */
int loki_glmsg_initialize(GLfloat bgr, GLfloat bgg, GLfloat bgb)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    int w = surface->w;
    int h = surface->h;

    texty = h - 15;  /* 13 pixels high, and 2 pixels buffer. */

    loki_glmsg_internal_initfuncs();

   // glfns.glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
   // glfns.glBlendFunc( GL_ONE, GL_ZERO  );
   // glfns.glBlendEquation(GL_FUNC_ADD);
   // glfns.glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    glfns.glDisable(GL_LIGHTING);
    glfns.glDisable(GL_COLOR_MATERIAL);
    glfns.glDisable(GL_SCISSOR_TEST);
    glfns.glDisable(GL_STENCIL_TEST);
    glfns.glDisable(GL_DEPTH_TEST);
    glfns.glDisable(GL_ALPHA_TEST);
    glfns.glDisable(GL_BLEND);
    glfns.glDisable(GL_DITHER);
    glfns.glDisable(GL_LOGIC_OP);
    glfns.glDisable(GL_FOG);
    glfns.glDisable(GL_COLOR_LOGIC_OP);
    glfns.glDisableClientState(GL_COLOR_ARRAY);
    glfns.glDisableClientState(GL_VERTEX_ARRAY);
    glfns.glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glfns.glShadeModel (GL_FLAT);

    glfns.glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glfns.glMatrixMode(GL_PROJECTION);
    glfns.glLoadIdentity();
    glfns.glOrtho (0.0, w, 0.0, h, -1.0, 1.0);
    glfns.glMatrixMode(GL_MODELVIEW);
    glfns.glLoadIdentity();

    loki_glmsg_internal_makeRasterFont();

    glfns.glClearColor(1.0, bgg, bgb, 0.0);
    glfns.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return(0);
}


void loki_glmsg_print(GLfloat r, GLfloat g, GLfloat b, const char *str)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    int textw = ((int) surface->w - ((int) (strlen(str) * 10))) / 2;

    #ifdef STANDALONE
        printf("loki_glmsg_print(): Should write this to the frame buffer:\n"
               "\"%s\"\n", str);
    #endif

    glfns.glColor4f(r, g, b, 1.0);
    glfns.glRasterPos2i(textw, texty);
    loki_glmsg_internal_printString(str);

    texty -= 15;
    if (texty <= 0)
        texty = surface->h - 15;

    glfns.glFlush ();
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
    SDL_SetVideoMode(w, h, 16, SDL_OPENGL);
    SDL_WM_SetCaption("loki_glmessage.c test.", "loki_glmsg");

    if (loki_glmsg_initialize(0.0, 0.0, 0.0) == -1)
    {
        fprintf(stderr, "loki_glmsg_initialize() failed!\n");
        SDL_Quit();
        exit(2);
    } // if

    loki_glmsg_print(1.0, 1.0, 1.0, "This is just a test");
    loki_glmsg_print(1.0, 1.0, 1.0, "do not panic");
    loki_glmsg_print(1.0, 0.0, 0.0, "red");
    loki_glmsg_print(0.0, 1.0, 0.0, "green");
    loki_glmsg_print(0.0, 0.0, 1.0, "blue");

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
    char *libname = "libGL.so";

    if (argv[1] != NULL)
        libname = argv[1];

    SDL_Init(SDL_INIT_VIDEO);

    SDL_ClearError();
    if (SDL_GL_LoadLibrary( libname ) < 0)
    {
        fprintf(stderr, "SDL_GL_LoadLibrary failed!\n");
        fprintf(stderr, "Error is [%s].\n", SDL_GetError());
        return(1);
    } // if

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );

    do_test(640, 480);
    do_test(320, 240);
    do_test(1600, 1200);

    SDL_Quit();
    return(0);
} // main
#endif  // STANDALONE

/* end of loki_glmessage.c ... */

