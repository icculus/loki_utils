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

#ifndef _LOKI_2DMESSAGE_H
#define _LOKI_2DMESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

int loki_2dmsg_initialize(float bgr, float bgg, float bgb, float bga);
void loki_2dmsg_print(float r, float g, float b, const char *str);

#ifdef __cplusplus
}
#endif

#endif

/* end of loki_glmessage.h ... */

