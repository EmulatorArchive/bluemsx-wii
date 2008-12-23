/******************************************************************************
 *
 *  SMS Plus - Sega Master System / GameGear Emulator
 *
 *  NGC/Wii Video support
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/
#ifndef _GC_VIDEO_H_
#define _GC_VIDEO_H_

#include <gccore.h>

#if 1
#define TEX_WIDTH         (512+32)
#define TEX_HEIGHT        480
#define DEFAULT_FIFO_SIZE (256 * 1024)
#define HASPECT           320
#define VASPECT           240
#else
#define TEX_WIDTH         (360 * 2)
#define TEX_HEIGHT        576
#define DEFAULT_FIFO_SIZE (256 * 1024)
#define HASPECT           320
#define VASPECT           240
#endif

extern void ogc_video__init(int interlaced, int ntsc, int bilinear, int console);
extern void ogc_video__update(void);
extern void ogc_video__reset(void);

extern int gc_pal;
extern unsigned int *xfb[2];
extern int whichfb;
extern GXRModeObj *tvmodes[2];
extern GXRModeObj *vmode;
extern u16 *texturemem;
#endif
