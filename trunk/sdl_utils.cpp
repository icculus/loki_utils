
/* Simple SDL access functions */

#include "SDL.h"
#include "SDL_syswm.h"
#ifdef unix
#include <X11/Xutil.h>
#endif
#include "sdl_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

int sdl_GetScreenSize(int *width, int *height)
{
    SDL_SysWMinfo info;
    int retval;

    retval = 0;
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            info.info.x11.lock_func();
            *width = DisplayWidth(info.info.x11.display,
                            DefaultScreen(info.info.x11.display));
            *height = DisplayHeight(info.info.x11.display,
                            DefaultScreen(info.info.x11.display));
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
    if ( SDL_GetWMInfo(&info) ) {
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
    if ( SDL_GetWMInfo(&info) ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window, junkwin;
            XWindowAttributes attributes;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

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
    if ( SDL_GetWMInfo(&info) ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Atom WM_HINTS;
            Display *display;
            Window window;
            XSetWindowAttributes attributes;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

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
    if ( SDL_GetWMInfo(&info) ) {
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Atom WM_HINTS;
            Display *display;
            Window window;
            XSetWindowAttributes attributes;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;

            switch(wm_type){
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
    if ( SDL_GetWMInfo(&info) ) {
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

void sdl_GetAbsoluteMouseCoords(int *x, int *y)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
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

static int isMouseConfined = 0;

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
    extern SDL_Surface *SDL_VideoSurface;

    if(!SDL_VideoSurface && update){
      isMouseConfined = on; // Only initialize the variable
      return;
    }

    if( SDL_VideoSurface->flags & SDL_FULLSCREEN )
        return;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
#ifdef unix
        if ( info.subsystem == SDL_SYSWM_X11 ) {
            Display *display;
            Window window;

            info.info.x11.lock_func();
            display = info.info.x11.display;
            window = info.info.x11.window;
            if ( on ) {
                /* The sync is required so the window is available */
                XSync(display, False);
                XGrabPointer(display, window, True, 0, GrabModeAsync,
                             GrabModeAsync, window, None, CurrentTime);
            } else {
                XUngrabPointer(display, CurrentTime);
            }
            if(update)
              isMouseConfined = on;
            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
}

void sdl_AutoRaise(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
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

#if 0  // This is dangerous to use, because it may be called when
       // the application is unmapped, causing a fatal X11 error.
void sdl_GetInputFocus(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
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

/* Once the window is iconified, it doesn't get input until the window
   manager brings it back, so sdl_IconifyWindow(0) is nearly useless.
*/
void sdl_IconifyWindow(int on)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
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

            info.info.x11.unlock_func();
        }
#else
#error Need to implement these functions for other systems
#endif // unix
    }
}

void sdl_AllowResize(void)
{
    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo(&info) ) {
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

#ifdef __cplusplus
};
#endif
