/* $Id$ */
/* This is a PCX image file loading framework, ripped straight from the SDL examples */

#include <stdio.h>
#include "SDL.h"
#include "SDL_endian.h"

#include "sdl_utils.h"

struct PCXheader {
	Uint8 Manufacturer;
	Uint8 Version;
	Uint8 Encoding;
	Uint8 BitsPerPixel;
	Sint16 Xmin, Ymin, Xmax, Ymax;
	Sint16 HDpi, VDpi;
	Uint8 Colormap[48];
	Uint8 Reserved;
	Uint8 NPlanes;
	Sint16 BytesPerLine;
	Sint16 PaletteInfo;
	Sint16 HscreenSize;
	Sint16 VscreenSize;
	Uint8 Filler[54];
};

/* See if an image is contained in a data source */
static int IMG_isPCX(SDL_RWops *src)
{
	int is_PCX;
	const int ZSoft_Manufacturer = 10;
	const int PC_Paintbrush_Version = 5;
	const int PCX_RunLength_Encoding = 1;
	struct PCXheader pcxh;

	is_PCX = 0;
	if ( SDL_RWread(src, &pcxh, sizeof(pcxh), 1) == 1 ) {
		if ( (pcxh.Manufacturer == ZSoft_Manufacturer) &&
		     (pcxh.Version == PC_Paintbrush_Version) &&
		     (pcxh.Encoding == PCX_RunLength_Encoding) ) {
			is_PCX = 1;
		}
	}
	return(is_PCX);
}

/* Load a PCX type image from an SDL datasource */
static SDL_Surface *IMG_LoadPCX_RW(SDL_RWops *src)
{
	struct PCXheader pcxh;
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;
	SDL_Surface *surface;
	int width, height;
	int i, index, x, y;
	int count;
	Uint8 *row, ch;
	int read_error;

	/* Initialize the data we will clean up when we're done */
	surface = NULL;
	read_error = 0;

	/* Check to make sure we have something to do */
	if ( ! src ) {
		goto done;
	}

	/* Read and convert the header */
	if ( ! SDL_RWread(src, &pcxh, sizeof(pcxh), 1) ) {
		goto done;
	}
	pcxh.Xmin = SDL_SwapLE16(pcxh.Xmin);
	pcxh.Ymin = SDL_SwapLE16(pcxh.Ymin);
	pcxh.Xmax = SDL_SwapLE16(pcxh.Xmax);
	pcxh.Ymax = SDL_SwapLE16(pcxh.Ymax);
	pcxh.BytesPerLine = SDL_SwapLE16(pcxh.BytesPerLine);

	/* Create the surface of the appropriate type */
	width = (pcxh.Xmax - pcxh.Xmin) + 1;
	height = (pcxh.Ymax - pcxh.Ymin) + 1;
	Rmask = Gmask = Bmask = Amask = 0 ; 
	if ( pcxh.BitsPerPixel > 16 ) {
		if ( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			Amask = 0xFF000000;
		} else {
			Rmask = 0xFF000000;
			Gmask = 0x00FF0000;
			Bmask = 0x0000FF00;
			Amask = 0x000000FF;
		}
	}
	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
			pcxh.BitsPerPixel*pcxh.NPlanes,Rmask,Gmask,Bmask,Amask);
	if ( surface == NULL ) {
		SDL_SetError("Out of memory");
		goto done;
	}

	/* Decode the image to the surface */
	for ( y=0; y<surface->h; ++y ) {
		for ( i=0; i<pcxh.NPlanes; ++i ) {
			row = (Uint8 *)surface->pixels + y*surface->pitch;
			index = i;
			for ( x=0; x<pcxh.BytesPerLine; ) {
				if ( ! SDL_RWread(src, &ch, 1, 1) ) {
					read_error = 1;
					goto done;
				}
				if ( (ch & 0xC0) == 0xC0 ) {
					count = ch & 0x3F;
					SDL_RWread(src, &ch, 1, 1);
				} else {
					count = 1;
				}
				while ( count-- ) {
					row[index] = ch;
					++x;
					index += pcxh.NPlanes;
				}
			}
		}
	}

	/* Look for the palette, if necessary */
	switch (surface->format->BitsPerPixel) {
	    case 1: {
		SDL_Color *colors = surface->format->palette->colors;

		colors[0].r = 0x00;
		colors[0].g = 0x00;
		colors[0].b = 0x00;
		colors[1].r = 0xFF;
		colors[1].g = 0xFF;
		colors[1].b = 0xFF;
	    }
	    break;

	    case 8: {
		SDL_Color *colors = surface->format->palette->colors;

		/* Look for the palette */
		do {
			if ( ! SDL_RWread(src, &ch, 1, 1) ) {
				read_error = 1;
				goto done;
			}
		} while ( ch != 12 );

		/* Set the image surface palette */
		for ( i=0; i<256; ++i ) {
			SDL_RWread(src, &colors[i].r, 1, 1);
			SDL_RWread(src, &colors[i].g, 1, 1);
			SDL_RWread(src, &colors[i].b, 1, 1);
		}
	    }
	    break;

	    default: {
		/* Don't do anything... */;
	    }
	    break;
	}

done:
	if ( read_error ) {
		SDL_FreeSurface(surface);
		SDL_SetError("Error reading PCX data");
		surface = NULL;
	}
	return(surface);
}

SDL_Surface *sdl_LoadPCX(const char *filename)
{
  SDL_RWops *src = SDL_RWFromFile(filename,"rb");
  if(IMG_isPCX(src)) {
    SDL_RWseek(src, 0, SEEK_SET);
	return IMG_LoadPCX_RW(src);
  }
  return NULL;
}
