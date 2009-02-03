/*****************************************************************************
** $Source: WiiNotifications.c,v $
**
** $Revision: 1.5 $
**
** $Date: 2006/06/24 02:27:08 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#include "ArchNotifications.h"
#include "FrameBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

static png_bytep  png_data = NULL;
static png_size_t png_data_allocated = 0;

static void MyPngErrorFunction(png_structp png_ptr, const char *err_msg)
{
	fprintf(stderr, "%s", err_msg);
}

static void MyPngWarningFunction(png_structp png_ptr, const char *warn_msg)
{
	return;
}

static void MyPngWriteData(png_structp png_ptr, png_bytep data, png_size_t size)
{
    png_size_t index = png_data_allocated;
    (void)png_ptr;
    png_data_allocated += size;
    png_data = realloc(png_data, png_data_allocated);
    memcpy(png_data + index, data, size);
}

static void MyPngFlushData(png_structp png_ptr)
{
    (void)png_ptr;
}

static int WritePng(FrameBuffer *frame, png_uint_32 width, png_uint_32 height,
                    png_uint_32 xoffset, png_uint_32 yoffset)
{
    png_structp png_ptr = NULL;
    png_infop  info_ptr = NULL;
	png_color palette[256];
	unsigned char **row_pointers;
	int pal_used;
	int i, x, y;
	int retval = 0;

	memset(&palette, 0, sizeof(palette));

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		                              NULL, MyPngErrorFunction, MyPngWarningFunction);
	if (!png_ptr) {
		fprintf(stderr, "Error creating write_struct\n");
		goto done;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
		fprintf(stderr, "Error creating write_info_struct\n");
        goto done;
    }

    png_set_write_fn(png_ptr, NULL, MyPngWriteData, MyPngFlushData);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8,
        		 PNG_COLOR_TYPE_PALETTE, PNG_FILTER_TYPE_BASE,
        		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    /* create 8 bpp image + pallette */
    row_pointers = (unsigned char**)malloc(height * sizeof(char*));
    pal_used = 0;
	for(y = 0; y < (int)height; y++) {
        LineBuffer *line = &frame->line[y+yoffset];
        UInt16 *ps = line->buffer + xoffset;
        unsigned char *pd = (unsigned char *)malloc(width);
        row_pointers[y] = pd;
		for(x = 0; x < (int)width ; x++) {
            int found;
            png_byte red = (png_byte)((*ps >> 11) & 0x1f);
            png_byte green = (png_byte)((*ps >> 6) & 0x1f);
            png_byte blue = (png_byte)((*ps) & 0x1f);
            red = (red << 3) | (red >> 2);
            green = (green << 3) | (green >> 2);
            blue = (blue << 3) | (blue >> 2);
            for(i = 0, found = 0; i < pal_used; i++) {
                png_color *c = &palette[i];
                if( c->red == red && c->green == green && c->blue == blue ) {
                    *pd = i;
                    found = 1;
                    break;
                }
            }
            if( !found ) {
                palette[pal_used].red = red;
                palette[pal_used].green = green;
                palette[pal_used].blue = blue;
                *pd = pal_used++;
            }
            if( line->doubleWidth ) {
                ps++;
            }
            ps++; pd++;
		}
	}

	/* ... set palette colors ... */
	png_set_PLTE(png_ptr, info_ptr, palette, pal_used);

	png_write_info(png_ptr, info_ptr);

	png_set_packing(png_ptr);

	png_write_image(png_ptr, row_pointers);

	png_write_end(png_ptr, info_ptr);

	/* free row_pointers array */
	for(i = 0; i < (int)height; i++) {
		free(row_pointers[i]);
	}
    free(row_pointers);

	retval = 1;
done:

	if(png_ptr) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
    if(!retval && png_data) {
        free(png_data);
        png_data = NULL;
        png_data_allocated = 0;
    }
	return retval;
}


void* archScreenCapture(ScreenCaptureType type, int* bitmapSize, int onlyBmp)
{
    FrameBuffer *frame = frameBufferGetViewFrame();
    if( frame ) {
        png_bytep p;
        (void)WritePng(frame, 256, 212, 8, 14);
        p = png_data;
        if( bitmapSize ) {
            *bitmapSize = png_data_allocated;
        }
        png_data = NULL;
        png_data_allocated = 0;
        return p;
    }
    return NULL;
}

void archUpdateEmuDisplayConfig() {}

/* Moved to Bluemsxlite.cpp
void archDiskQuickChangeNotify() {}
*/
void archEmulationStartNotification() {}
void archEmulationStopNotification() {}
void archEmulationStartFailure() {}

void archQuit() {}

void archThemeSetNext() {}
void archThemeUpdate(struct Theme* theme) {}

void archVideoOutputChange() {}
void archMinimizeMainWindow() {}

int archGetFramesPerSecond() { return 60; }

void* archWindowCreate(struct Theme* theme, int childWindow) { return NULL; }
void archWindowStartMove() {}
void archWindowMove() {}
void archWindowEndMove() {}

void archVideoCaptureSave() {}
