#ifndef __SDLUTILS_H__
#define __SDLUTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Simple SDL Window Manager access functions */

extern int sdl_GetScreenSize(int *width, int *height);
extern int sdl_GetWindowSize(int *width, int *height);
extern int sdl_GetWindowPosition(int *x, int *y);
extern void sdl_GetAbsoluteMouseCoords(int *x, int *y);
extern void sdl_RemoveTitleBar(void);
extern void sdl_RestoreTitleBar(void);
extern int sdl_IsMouseConfined(void);
extern void sdl_ConfineMouse(int on, int update);
extern void sdl_ToggleConfineMouse(void);
extern void sdl_IconifyWindow(int on);
extern void sdl_AllowResize(void);
extern void sdl_RemapWindow(void);

/* Display a BMP image */
extern int sdl_DisplayImage(const char *filename, SDL_Surface *screen);

#ifdef __cplusplus
};
#endif

#endif