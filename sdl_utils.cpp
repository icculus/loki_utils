
/* Simple SDL access functions */

#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "SDL.h"
#include "SDL_syswm.h"
#include "loki_utils.h"
#include "sdl_utils.h"

#ifndef LOKI_NO_GLMSG
#include "loki_glmessage.h"
#endif

#include "loki_2dmessage.h"

#ifdef unix
#include <X11/Xutil.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOKI_NO_GLMSG
typedef float GLfloat;
#define loki_glmsg_initialize(bgr, bgg, bgb) fprintf(stderr, "No GLmsg support!\n")
#define loki_glmsg_print(bgr, bgg, bgb, txt) fprintf(stderr, "GLmsg: %s\n", txt)
#endif

static inline void sdl_showmsg_print(float, float, float, const char *);


static inline void sdl_showmsg_initialize(float bgr, float bgg, float bgb)
{
    if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
        loki_glmsg_initialize(bgr, bgg, bgb);
    else
        loki_2dmsg_initialize(bgr, bgg, bgb);
}

static inline void sdl_showmsg_splitprint(GLfloat r, GLfloat g, GLfloat b,
                                            const char *str)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    int chars_per_line = (surface->w / 10) - 2;   // minus two for spacing.
    int len = strlen(str);
    char buffer[len + 1];
    int i;

    for (i = 0; i < len; i += chars_per_line)
    {
        strcpy(buffer, str + i);
        buffer[chars_per_line] = '\0';
        sdl_showmsg_print(r, g, b, buffer);
    } // for
}

static inline void sdl_showmsg_print(float r, float g, float b, const char *str)
{
    SDL_Surface *surface = SDL_GetVideoSurface();
    int textw = ((int) surface->w - ((int) (strlen(str) * 10))) / 2;

    if (textw < 0)
    {
        sdl_showmsg_splitprint(r, g, b, str);
        return;
    } // if

    if (SDL_GetVideoSurface()->flags & SDL_OPENGL)
        loki_glmsg_print(r, g, b, str);
    else
        loki_2dmsg_print(r, g, b, str);
}

//----------------------



void sdl_ShowMessage(const char *fmt, ...)
{
    char buffer[1024];
    int totallines = 0;
    char *nextnl = buffer;
    char *str = buffer;
    int screenlines = (SDL_GetVideoSurface()->h / 15);      // 15 pixels/line
    int screenwidth = (SDL_GetVideoSurface()->h / 10) - 2;  // -2 for spacing
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buffer, sizeof (buffer), fmt, ap);
    va_end(ap);

    sdl_showmsg_initialize(0.0, 0.0, 0.0);

    do
    {
        nextnl = strchr(str, '\n');
        if (nextnl != NULL)
        {
            totallines += ( ((int) (nextnl - str)) / screenwidth ) + 1;
            str = nextnl + 1;
        } // if
    } while (nextnl != NULL);

    for (int i = (screenlines - totallines) / 2; i >= 0; i--)
        sdl_showmsg_print(0.0, 0.0, 0.0, "");

    // okay, message is centered vertically...write it out.

    str = buffer;

    do
    {
        nextnl = strchr(str, '\n');
        if (nextnl != NULL)
            *nextnl = '\0';
        sdl_showmsg_print(1.0, 1.0, 1.0, str);
        str = nextnl + 1;
    } while (nextnl != NULL);
}



int sdl_GetScreenSize(int *width, int *height)
{
    SDL_SysWMinfo info;
    int retval;

    retval = 0;
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            info.info.x11.lock_func();
            if ( width ) {
                *width = DisplayWidth(info.info.x11.display,
                            DefaultScreen(info.info.x11.display));
            }
            if ( height ) {
                *height = DisplayHeight(info.info.x11.display,
                            DefaultScreen(info.info.x11.display));
            }
            info.info.x11.unlock_func();
            retval = 1;
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
    return retval;
}

int sdl_GetWindowSize(int *width, int *height)
{
    SDL_SysWMinfo info;
    int retval;

    retval = 0;
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Window u1; int u2; unsigned int u3;
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            XGetGeometry(display, window, &u1, &u2, &u2,
                         (unsigned int *)width,
                         (unsigned int *)height, &u3, &u3);

            info.info.x11.unlock_func();
            retval = 1;
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
    return retval;
}

int sdl_GetWindowPosition(int *x, int *y)
{
    SDL_SysWMinfo info;
    int retval;

    retval = 0;
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window, junkwin;
            XWindowAttributes attributes;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            XSync(display, False);
            XGetWindowAttributes(display, window, &attributes);
            XTranslateCoordinates (display, window, attributes.root, 
                                   -attributes.border_width,
                                   -attributes.border_width,
                                   x, y, &junkwin);
            info.info.x11.unlock_func();
            retval = 1;
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
    return retval;
}

void sdl_GetAbsoluteMouseCoords(int *x, int *y)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window, uncle, fucker;
            int u;
            unsigned int mask;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            XQueryPointer(display, window, &uncle, &fucker, x,y, &u,&u, &mask);
            info.info.x11.unlock_func();
        }
    }  
}

void sdl_ToggleConfineMouse(void)
{
    SDL_GrabMode mode;

    mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
    if ( mode == SDL_GRAB_ON ) {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    } else {
        SDL_WM_GrabInput(SDL_GRAB_ON);
    }
}

static char *clipboard = NULL;

int sdl_ClipboardFilter(const SDL_Event *event)
{
    Display *display;

    /* Post all non-window manager specific events */
    if ( event->type != SDL_SYSWMEVENT ) {
        return(1);
    }

    /* Handle window-manager specific clipboard events */
    display = event->syswm.msg->event.xevent.xany.display;
    switch (event->syswm.msg->event.xevent.type) {
        /* Copy the selection from XA_CUT_BUFFER0 to the requested property */
        case SelectionRequest: {
            XSelectionRequestEvent *req;
            XEvent sevent;
            int seln_format;
            unsigned long nbytes;
            unsigned long overflow;
            unsigned char *seln_data;

            req = &event->syswm.msg->event.xevent.xselectionrequest;
            sevent.xselection.type = SelectionNotify;
            sevent.xselection.display = req->display;
            sevent.xselection.selection = req->selection;
            sevent.xselection.target = None;
            sevent.xselection.property = None;
            sevent.xselection.requestor = req->requestor;
            sevent.xselection.time = req->time;
            if ( XGetWindowProperty(display, DefaultRootWindow(display),
                              XA_CUT_BUFFER0, 0, INT_MAX/4, False, req->target,
                              &sevent.xselection.target, &seln_format,
                              &nbytes, &overflow, &seln_data) == Success ) {
                if ( sevent.xselection.target == req->target ) {
                    if ( sevent.xselection.target == XA_STRING ) {
                        if ( seln_data[nbytes-1] == '\0' )
                            --nbytes;
                    }
                    XChangeProperty(display, req->requestor, req->property,
                        sevent.xselection.target, seln_format, PropModeReplace,
                                                            seln_data, nbytes);
                    sevent.xselection.property = req->property;
                }
                XFree(seln_data);
            }
            XSendEvent(display, req->requestor, False, 0, &sevent);
            XSync(display, False);
        }
        break;
    }

    /* Post the event for X11 clipboard reading above */
    return(1);
}
void sdl_InitClipboard(void)
{
    SDL_SysWMinfo info;

    /* See if we can use our X11 code on this driver */
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            /* Enable the special window hook events */
            SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
            SDL_SetEventFilter(sdl_ClipboardFilter);
        }
    }
}

/* Put null-terminated text into the clipboard */
void sdl_PutClipboard(const char *text)
{
    SDL_SysWMinfo info;

    /* Save the clipboard text */
    clipboard = (char *)realloc((void *)clipboard, strlen(text)+1);
    strcpy(clipboard, text);

    /* Try to put the text selection into the X server */
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;
            XChangeProperty(display, DefaultRootWindow(display),
                            XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace,
                            (unsigned char *)clipboard, strlen(clipboard));
            if ( XGetSelectionOwner(display, XA_PRIMARY) != window )
                XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
            info.info.x11.unlock_func();
        }
    }
}

/* Get null-terminated text from the clipboard */
const char *sdl_GetClipboard(void)
{
    SDL_SysWMinfo info;

    /* Try to get a text selection from the X server */
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;
            Window owner;
            Atom selection;
            Atom seln_type;
            int seln_format;
            unsigned long nbytes;
            unsigned long overflow;
            char *src;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;
            owner = XGetSelectionOwner(display, XA_PRIMARY);
            if ( (owner == None) || (owner == window) ) {
                owner = DefaultRootWindow(display);
                selection = XA_CUT_BUFFER0;
            } else {
                int selection_converted;
                XEvent xevent;

                owner = window;
                selection = XInternAtom(display, "SDL_SELECTION", False);
                XConvertSelection(display, XA_PRIMARY, XA_STRING,
                                                selection, owner, CurrentTime);
                selection_converted = 0;
                while ( ! selection_converted ) {
                    // FIXME: What mask should we use here?
                    //XMaskEvent(display, StructureNotifyMask, &xevent);
                    XNextEvent(display, &xevent);
                    if ( (xevent.type == SelectionNotify) &&
                         (xevent.xselection.requestor == owner) ) {
                        selection_converted = 1;
                    }
                }
            }
            if ( XGetWindowProperty(display, owner, selection, 0,
                     INT_MAX/4, False, XA_STRING, &seln_type, &seln_format,
                     &nbytes, &overflow, (unsigned char **)&src) == Success ) {
                if ( src ) {
                    if ( seln_type == XA_STRING ) {
                        clipboard = (char *)realloc(clipboard, nbytes+1);
                        memcpy(clipboard, src, nbytes);
                        clipboard[nbytes] = '\0';
                    }
                    XFree(src);
                }
            }
            info.info.x11.unlock_func();
        }
    }

    /* Return the clipboard text */
    if ( clipboard == NULL ) {
        clipboard = (char *)malloc(1);
        if ( clipboard ) {
            *clipboard = '\0';
        }
    }
    return(clipboard);
}


int sdl_DisplayImage(const char *filename, SDL_Surface *screen)
{
    SDL_Surface *file = SDL_LoadBMP(filename);
  
    if(file){
        SDL_Rect dst = {0,0, file->w, file->h};
        
        dst.x = (screen->w - file->w) / 2;
        dst.y = (screen->h - file->h) / 2;

        SDL_BlitSurface(file, NULL, screen, &dst);
        SDL_UpdateRects(screen, 1, &dst);
        SDL_FreeSurface(file);
        return 1;
    }
    return 0;
}

void sdl_SnapShot(SDL_Surface *screen)
{
	char filename[100], path[PATH_MAX];
	int count = 1;

	if ( ! screen ) {
		screen = SDL_GetVideoSurface();
	}

	if ( ! screen ) return;

	do {
		sprintf(filename,"%s_shot%d.bmp", loki_getgamename(), count++);
	}while(!access(loki_getpreffile(filename, path, sizeof(path)), R_OK));

	SDL_SaveBMP(screen, path);
}

/*********************************************************************/
/*  Old and obsolete functions                                       */
/*********************************************************************/

#if 0 /* Obsolete by SDL 1.0.2 (now three windows involved) */
typedef struct {
  Sint32 flags;
  Sint32 functions;
  Sint32 decorations;
  Sint32 inputMode;
  Sint32 unknown;
} MWM_Hints;

static enum { No_WM, Motif_WM, KDE_WM, GNOME_WM } wm_type;

void sdl_RemoveTitleBar(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Atom WM_HINTS;
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            /* This happens in SDL 1.1 */
            if ( window == 0 ) {
                info.info.x11.unlock_func();
                printf("FIXME: SDL 1.1 - Can't remove the titlebar yet\n");
                return;
            }
#if 1
            /* First try to set MWM hints */
            WM_HINTS = XInternAtom(display, "_MOTIF_WM_HINTS", True);
            if ( WM_HINTS != None ) { 
#               define    MWM_HINTS_DECORATIONS    (1L << 1)
                MWM_Hints MWMHints = { MWM_HINTS_DECORATIONS, 0, 0, 0, 0 };
                
                XChangeProperty(display, window, WM_HINTS, WM_HINTS, 32,
                                PropModeReplace, (unsigned char *)&MWMHints,
                                sizeof(MWMHints)/4);
                wm_type = Motif_WM;
            }
            /* Now try to set KWM hints */
            WM_HINTS = XInternAtom(display, "KWM_WIN_DECORATION", True);
            if ( WM_HINTS != None ) { 
                long KWMHints = 0;

                XChangeProperty(display, window, WM_HINTS, WM_HINTS, 32,
                                PropModeReplace, (unsigned char *)&KWMHints,
                                sizeof(KWMHints)/4);
                wm_type = KDE_WM;
            }
            /* Now try to set GNOME hints */
            WM_HINTS = XInternAtom(display, "_WIN_HINTS", True);
            if ( WM_HINTS != None ) { 
                long GNOMEHints = 0;

                XChangeProperty(display, window, WM_HINTS, WM_HINTS, 32,
                                PropModeReplace, (unsigned char *)&GNOMEHints,
                                sizeof(GNOMEHints)/4);
                wm_type = GNOME_WM;
            }
#endif
#if 0        /* The Butcher way of removing window decorations -- not polite */
            attributes.override_redirect = True;
            XChangeWindowAttributes(info.info.x11.display, info.info.x11.window,
                                    CWOverrideRedirect, &attributes);
#endif
            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
}

/* Looks like deleting the WM properties does the job */
void sdl_RestoreTitleBar(void)
{
    SDL_SysWMinfo info;

    if(!wm_type)
      return;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Atom WM_HINTS;
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            switch(wm_type){
                case No_WM:
                case GNOME_WM:
                case Motif_WM:
                  WM_HINTS = XInternAtom(display, "_MOTIF_WM_HINTS", True);
                  break;
                case KDE_WM:
                  WM_HINTS = XInternAtom(display, "KWM_WIN_DECORATION", True);
                  break;
            }
            XDeleteProperty(display,window,WM_HINTS);

            info.info.x11.unlock_func();
        }
    }
}

void sdl_RemapWindow(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;
            XEvent event;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            XUnmapWindow(display,window);
            XMapRaised(display,window);
            do {
              XNextEvent(display, &event);
            } while ( event.type != MapNotify );

            XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);
            info.info.x11.unlock_func();
        }
    }
}
#endif /* Obsolete by SDL 1.0.2 */

#if 0 /* Obsolete by SDL 1.0.2 (now use SDL_WM_GrabInput()) */
static int isMouseConfined = 0;
static int grabsKeyboard = 0;

void sdl_ConfineGrabsKeyboard(int flag)
{
   grabsKeyboard = flag;
}

int sdl_IsMouseConfined(void)
{
    return isMouseConfined;
}

void sdl_ToggleConfineMouse( void )
{
    sdl_ConfineMouse( isMouseConfined ? 0 : 1, 1 );
}

void sdl_ConfineMouse(int on, int update)
{
    SDL_SysWMinfo info;
    SDL_Surface *sdl_screen;

    /* Update the state variable */
    if ( update ) {
        isMouseConfined = on;
    }

    /* See if the display is in fullscreen mode */
    sdl_screen = SDL_GetVideoSurface();
    if ( !sdl_screen || (sdl_screen->flags & SDL_FULLSCREEN) ) {
        return;
    }

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;
            
            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;
            if ( on ) {
                int numTries, grabResult;
                /* The sync is required so the window is available */
                XSync(display, False);
                /* Grab the input focus so that the cursor is moved to the window */
                XSetInputFocus(display, window, RevertToNone, CurrentTime);
                for (numTries = 0; numTries < 10; numTries++) {
                    grabResult = XGrabPointer(display, window,
                                              True, ButtonPressMask|ButtonReleaseMask|ButtonMotionMask
                                              |PointerMotionMask, GrabModeAsync, GrabModeAsync, window,
                                              None, CurrentTime);
                    if (grabResult != AlreadyGrabbed) {
                        break;
                    }
                    SDL_Delay(100);
                }
                if(grabsKeyboard){
                    grabResult = XGrabKeyboard(display, window,
                                               False, GrabModeAsync, GrabModeAsync, CurrentTime);
                    if (grabResult != 0) {
                        XUngrabPointer(display, CurrentTime);
                    }
                }
            } else {
                XUngrabPointer(display, CurrentTime);
                XUngrabKeyboard(display, CurrentTime);
            }
            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
}
#endif /* Obsolete by SDL 1.0.2 */

#if 0  // This is dangerous to use, because it may be called when
       // the application is unmapped, causing a fatal X11 error.
void sdl_GetInputFocus(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;
            XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);
            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
}
#endif

#if 0 /* Obsolete by SDL 1.0.2 (now use SDL_WM_IconifyWindow()) */
/* Once the window is iconified, it doesn't get input until the window
   manager brings it back, so sdl_IconifyWindow(0) is nearly useless.
*/
int sdl_IconifyWindow(int on)
{
    SDL_SysWMinfo info;
    SDL_Surface *sdl_screen;
    int retval = 0;

    /* See if the display is in fullscreen mode */
    sdl_screen = SDL_GetVideoSurface();
    if ( !sdl_screen || (sdl_screen->flags & SDL_FULLSCREEN) ) {
        return retval;
    }

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            if ( on ) {
                /* Notify the window manager and unmap the window.
                   Don't wait for the iconify to complete, otherwise 
                   SDL will miss the deactivation event.
                 */
                XIconifyWindow(display, window, DefaultScreen(display));
            } else {
                /* Map the window.
                   Don't wait for the map to complete, otherwise 
                   SDL will miss the activation event.
                 */
                XMapWindow(display, window);
            }
            retval = 1;
            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
    return retval;
}
#endif /* Obsolete by SDL 1.0.2 */

#if 0 /* Obsolete by SDL 1.0.2 (now three windows involved) */
void sdl_AllowResize(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) > 0 ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;
            XSizeHints   *sizehints;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            sizehints = XAllocSizeHints();
            sizehints->min_width   = 0;
            sizehints->min_height  = 0;
            sizehints->max_width   = 4000;
            sizehints->max_height  = 4000;
            sizehints->flags       = (PMinSize | PMaxSize);
            XSetWMNormalHints(display, window, sizehints);
            XFree(sizehints);

            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
}
#endif /* Obsolete by SDL 1.0.2 */

/*********************************************************************/

#ifdef __cplusplus
};
#endif
