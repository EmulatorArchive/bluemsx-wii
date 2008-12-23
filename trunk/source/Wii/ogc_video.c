/******************************************************************************
 *
 *  Genesis Plus - Sega Megadrive / Genesis Emulator
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

#include "ogc_video.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <fat.h>
#include <string.h>
#include <sys/dir.h>
#include <gccore.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

/* Config */
int g_tv_mode;
int g_interlaced;
int g_ntsc;
int g_bilinear;
int g_render = 1; /* 480i */

/*** PAL 50hz flag ***/
int gc_pal = 0;
int vdp_pal = 1; //?

/*** VI ***/
unsigned int *xfb[2];	/*** Double buffered            ***/
int whichfb = 0;		  /*** External framebuffer index ***/
GXRModeObj *vmode;    /*** Menu video mode            ***/
u16 *texturemem;       /*** Texture Data               ***/

/*** GX ***/
static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static GXTexObj texobj;
static Mtx view;
static u32 vwidth, vheight;

/* 576 lines interlaced (PAL 50Hz, scaled) */
GXRModeObj TV50hz_576i =
{
  VI_TVMODE_PAL_INT,      // viDisplayMode
  640,             // fbWidth
  480,             // efbHeight
  574,             // xfbHeight
  (VI_MAX_WIDTH_PAL - 720)/2,         // viXOrigin
  (VI_MAX_HEIGHT_PAL - 574)/2,        // viYOrigin
  720,             // viWidth
  574,             // viHeight
  VI_XFBMODE_DF,   // xFBmode
  GX_FALSE,        // field_rendering
  GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{3,2},{9,6},{3,10},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{3,2},{9,6},{3,10},  // pix 1
		{9,2},{3,6},{9,10},  // pix 2
		{9,2},{3,6},{9,10}   // pix 3
	},

    // vertical filter[7], 1/64 units, 6 bits each
	{
		 4,         // line n-1
		 8,         // line n-1
		12,         // line n
		16,         // line n
		12,         // line n
		 8,         // line n+1
		 4          // line n+1
	}
};

/* 480 lines interlaced (NTSC or PAL 60Hz) */
GXRModeObj TV60hz_480i =
{
  VI_TVMODE_EURGB60_INT,     // viDisplayMode
  640,             // fbWidth
  480,             // efbHeight
  480,             // xfbHeight
  (VI_MAX_WIDTH_NTSC - 720)/2,        // viXOrigin
  (VI_MAX_HEIGHT_NTSC - 480)/2,       // viYOrigin
  720,             // viWidth
  480,             // viHeight
  VI_XFBMODE_DF,   // xFBmode
  GX_FALSE,        // field_rendering
  GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{3,2},{9,6},{3,10},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{3,2},{9,6},{3,10},  // pix 1
		{9,2},{3,6},{9,10},  // pix 2
		{9,2},{3,6},{9,10}   // pix 3
	},

    // vertical filter[7], 1/64 units, 6 bits each
	{
		 4,         // line n-1
		 8,         // line n-1
		12,         // line n
		16,         // line n
		12,         // line n
		 8,         // line n+1
		 4          // line n+1
	}
};

/* TV Modes table */
GXRModeObj *tvmodes[2] = {
	 &TV60hz_480i,  /* 60hz modes */
	 &TV50hz_576i   /* 50Hz modes */
};

typedef struct tagcamera
{
  Vector pos;
  Vector up;
  Vector view;
} camera;

/*** Square Matrix
     This structure controls the size of the image on the screen.
	 Think of the output as a -80 x 80 by -60 x 60 graph.
***/
static s16 square[] ATTRIBUTE_ALIGN (32) =
{
  /*
   * X,   Y,  Z
   * Values set are for roughly 4:3 aspect
   */
	-HASPECT,  VASPECT, 0,	// 0
   HASPECT,  VASPECT, 0,	// 1
	 HASPECT, -VASPECT, 0,	// 2
	-HASPECT, -VASPECT, 0,	// 3
};

static camera cam = {
  {0.0F, 0.0F, -100.0F},
  {0.0F, -1.0F, 0.0F},
  {0.0F, 0.0F, 0.0F}
};

/* rendering initialization */
/* should be called each time you change quad aspect ratio */
static void draw_init(void)
{
  /* Clear all Vertex params */
  GX_ClearVtxDesc ();

  /* Set Position Params (quad aspect ratio) */
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
  GX_SetVtxDesc (GX_VA_POS, GX_INDEX8);
  GX_SetArray (GX_VA_POS, square, 3 * sizeof (s16));

  /* Set Tex Coord Params */
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
  GX_SetTevOp (GX_TEVSTAGE0, GX_REPLACE);
  GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
  GX_SetNumTexGens (1);
  GX_SetNumChans(0);

  /* Set Modelview */
  memset (&view, 0, sizeof (Mtx));
  guLookAt(view, &cam.pos, &cam.up, &cam.view);
  GX_LoadPosMtxImm (view, GX_PNMTX0);
}

/* vertex rendering */
static inline void draw_vert(u8 pos, f32 s, f32 t)
{
  GX_Position1x8 (pos);
  GX_TexCoord2f32 (s, t);
}

/* textured quad rendering */
static inline void draw_square (void)
{
  GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
  draw_vert (3, 0.0, 0.0);
  draw_vert (2, 1.0, 0.0);
  draw_vert (1, 1.0, 1.0);
  draw_vert (0, 0.0, 1.0);
  GX_End ();
}

/* retrace handler */
static void framestart(u32 retraceCnt)
{
  /* simply increment the tick counter */
  //frameticker++;
}

/* Initialize GX */
static void gxStart(void)
{
  Mtx p;
  GXColor gxbackground = { 0, 0, 0, 0xff };

  /*** Clear out FIFO area ***/
  memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

  /*** GX default ***/
  GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
  GX_SetCopyClear (gxbackground, 0x00ffffff);

  GX_SetViewport (0.0F, 0.0F, vmode->fbWidth, vmode->efbHeight, 0.0F, 1.0F);
  GX_SetScissor (0, 0, vmode->fbWidth, vmode->efbHeight);
  f32 yScale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
  u16 xfbHeight = GX_SetDispCopyYScale (yScale);
  GX_SetDispCopySrc (0, 0, vmode->fbWidth, vmode->efbHeight);
  GX_SetDispCopyDst (vmode->fbWidth, xfbHeight);
  GX_SetCopyFilter (vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
  GX_SetFieldMode (vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  GX_SetPixelFmt (GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GX_SetCullMode (GX_CULL_NONE);
  GX_SetDispCopyGamma (GX_GM_1_0);
  GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
  GX_SetColorUpdate (GX_TRUE);
  guOrtho(p, vmode->efbHeight/2, -(vmode->efbHeight/2), -(vmode->fbWidth/2), vmode->fbWidth/2, 100, 1000);
  GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);

  /*** Reset XFB ***/
  GX_CopyDisp (xfb[whichfb ^ 1], GX_TRUE);

  /*** Initialize texture data ***/
  texturemem = memalign(32, TEX_WIDTH * TEX_HEIGHT * 2);
  memset (texturemem, 0, TEX_WIDTH * TEX_HEIGHT * 2);

  /*** Initialize renderer */
  draw_init();
}

/* Reset GX/VI scaler */
static void gxScale(GXRModeObj *rmode)
{
  int xscale = 320;
  int yscale = 240;
  int xshift = 0;
  int yshift = 0;

  /* GX Scaler (by default, use EFB maximal width) */
  rmode->fbWidth = 640;

  /* enable VI scaler */
  rmode->viWidth = xscale * 2;
  if(CONF_GetAspectRatio()) {
    /* widescreen fix */
    rmode->viWidth += 68;
  }
  rmode->viXOrigin = (VI_MAX_WIDTH_PAL - rmode->viWidth) / 2;
  /* set GX scaling to max EFB width */
  xscale = rmode->fbWidth / 2;

  /* update GX scaler (Vertex Position Matrix) */
  square[6] = square[3]  =  xscale + xshift;
	square[0] = square[9]  = -xscale + xshift;
  square[4] = square[1]  =  yscale + yshift;
	square[7] = square[10] =  -yscale + yshift;

  DCFlushRange (square, 32);
  GX_InvVtxCache ();
}

/* Reinitialize Video */
void ogc_video__reset()
{
	Mtx p;
  GXRModeObj *rmode;

  /* 50Hz/60Hz mode */
  if ((g_tv_mode == 1) || ((g_tv_mode == 2) && vdp_pal)) gc_pal = 1;
  else gc_pal = 0;

  /* Set current TV mode */
  rmode = tvmodes[gc_pal];

  /* Aspect ratio */
  gxScale(rmode);

  /* Configure VI */
  VIDEO_Configure (rmode);
	VIDEO_ClearFrameBuffer(rmode, xfb[whichfb], COLOR_BLACK);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
	else while (VIDEO_GetNextField())  VIDEO_WaitVSync();

  /* Configure GX */
  GX_SetViewport (0.0F, 0.0F, rmode->fbWidth, rmode->efbHeight, 0.0F, 1.0F);
  GX_SetScissor (0, 0, rmode->fbWidth, rmode->efbHeight);
  f32 yScale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
  u16 xfbHeight = GX_SetDispCopyYScale (yScale);
  GX_SetDispCopySrc (0, 0, rmode->fbWidth, rmode->efbHeight);
  GX_SetDispCopyDst (rmode->fbWidth, xfbHeight);
  GX_SetCopyFilter (rmode->aa, rmode->sample_pattern, g_render ? GX_TRUE : GX_FALSE, rmode->vfilter);
  GX_SetFieldMode (rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  GX_SetPixelFmt (rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  guOrtho(p, rmode->efbHeight/2, -(rmode->efbHeight/2), -(rmode->fbWidth/2), rmode->fbWidth/2, 100, 1000);
  GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);
}

/* GX render update */
void ogc_video__update(void)
{
  static int initialized = 0;
  /* check if viewport has changed */
  if( !initialized )
  {
    initialized = 1;

    /* update texture size */
    vwidth  = TEX_WIDTH; //?
    vheight = TEX_HEIGHT; //?

    /* special cases */
    if (g_interlaced) vheight *= 2;

    /* texels size must be multiple of 4 */
    vwidth  = vwidth  >> 2;
    vheight = vheight >> 2;

    /* image size has changed, reset GX */
    ogc_video__reset();

    /* reinitialize texture */
    GX_InitTexObj (&texobj, texturemem, vwidth << 2, vheight << 2, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);

    /* enable/disable bilinear filtering */
    if (!g_bilinear)
    {
      GX_InitTexObjLOD(&texobj,GX_NEAR,GX_NEAR_MIP_NEAR,2.5,9.0,0.0,GX_FALSE,GX_FALSE,GX_ANISO_1);
    }

    /* load texture */
    GX_LoadTexObj (&texobj, GX_TEXMAP0);
  }

  /* update texture cache */
  DCFlushRange (texturemem, TEX_WIDTH * TEX_HEIGHT * 2);
  GX_InvalidateTexAll ();

  /* render textured quad */
  draw_square ();
  GX_DrawDone ();

  /* switch external framebuffers then copy EFB to XFB */
  whichfb ^= 1;
  GX_CopyDisp (xfb[whichfb], GX_TRUE);
  GX_Flush ();

  /* set next XFB */
  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
}

/* Initialize VIDEO subsystem */
void ogc_video__init(int interlaced, int ntsc, int bilinear, int console)
{
  g_interlaced = interlaced;
  g_ntsc = ntsc;
  g_bilinear = bilinear;

  /*
   * Before doing anything else under libogc,
   * Call VIDEO_Init
   */
  VIDEO_Init ();

  /* Get the current video mode then :
      - set menu video mode (480p, 480i or 576i)
      - set emulator rendering TV modes (PAL/MPAL/NTSC/EURGB60)
   */
  vmode = VIDEO_GetPreferredMode(NULL);

  /* Adjust display settings */
  switch (vmode->viTVMode >> 2)
  {
    case VI_PAL:  /* 576 lines (PAL 50Hz) */

      TV60hz_480i.viTVMode = VI_TVMODE_EURGB60_INT;
      g_tv_mode = 1;

      /* display should be centered vertically (borders) */
      vmode = &TVPal574IntDfScale;
      vmode->xfbHeight = 480;
      vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - 480)/2;
      vmode->viHeight = 480;

      break;

    case VI_NTSC: /* 480 lines (NTSC 60hz) */
      TV60hz_480i.viTVMode = VI_TVMODE_NTSC_INT;
      g_tv_mode = 0;

#ifndef HW_RVL
      /* force 480p on NTSC GameCube if the Component Cable is present */
      if (VIDEO_HaveComponentCable()) vmode = &TVNtsc480Prog;
#endif
      break;

    default:  /* 480 lines (PAL 60Hz) */
      TV60hz_480i.viTVMode = VI_TVMODE(vmode->viTVMode >> 2, VI_INTERLACE);
      g_tv_mode = 2;
  	  gc_pal = 0;
      break;
  }

  /* Configure video mode */
  VIDEO_Configure (vmode);

  /* Configure the framebuffers (double-buffering) */
  xfb[0] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TV50hz_576i));
  xfb[1] = (u32 *) MEM_K0_TO_K1((u32 *) SYS_AllocateFramebuffer(&TV50hz_576i));

  /* Clear framebuffers to black */
  VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);

  /* Define a console */
  if( console ) {
	  //console_init(xfb[0], 20, 64, 640, 574, 574 * 2);
	  console_init(xfb[0], 20, 20, vmode->fbWidth, vmode->xfbHeight,
	               vmode->fbWidth * VI_DISPLAY_PIX_SZ);
  }

  /* Set the framebuffer to be displayed at next VBlank */
  VIDEO_SetNextFramebuffer(xfb[0]);

  /* Register Video Retrace handlers */
  VIDEO_SetPreRetraceCallback(framestart);

  /* Enable Video Interface */
  VIDEO_SetBlack (FALSE);

  /* Update video settings for next VBlank */
  VIDEO_Flush ();

  /* Wait for VBlank */
  VIDEO_WaitVSync();
  VIDEO_WaitVSync();

  /* Initialize GX */
  gxStart();
}
