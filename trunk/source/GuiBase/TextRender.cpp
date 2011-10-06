/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * Based on code of "DragonMinded"
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include "TextRender.h"

TextRender::TextRender()
{
    // Initialize the library
    FT_Error error = FT_Init_FreeType(&m_library);

    // Default values
    m_yspacing = DEFAULT_Y_CUSHION;

    if(error)
    {
        // Better handling in the future
        exit(0);
    }
}

TextRender::~TextRender()
{
    // Free the library (and all resources)
    FT_Done_FreeType(m_library);
}

void TextRender::SetFont(string path)
{
    // Initialize a font
    FT_Error error = FT_New_Face(m_library, path.c_str(), 0, &m_face);

    if ( error )
    {
        // Better handling in the future
        exit(0);
    }
}

void TextRender::SetFont(const unsigned char* font, int size)
{
    // Initialize a font
    FT_Error error = FT_New_Memory_Face(m_library, font, size, 0, &m_face);
    m_fontheight = size;

    if ( error )
    {
        // Better handling in the future
        exit(0);
    }
}

void TextRender::SetColor(GXColor c)
{
    m_color = c;
}

void TextRender::SetSize(int s)
{
    // Set up the font face (see freetype samples for magic values)
    FT_Set_Pixel_Sizes(m_face, 0, s);
    m_fontheight = s;
}

void TextRender::SetYSpacing(int s)
{
    m_yspacing = s + DEFAULT_Y_CUSHION;
}

void TextRender::SetBuffer(uint8_t *buf, int width, int height, int bufwidth)
{
    // Set up a buffer to render to (RGBA quads, most likely to be blitted into a GuiTextImage and rendered using libwiisprite)
    m_buf = buf;
    m_width = width;
    m_height = height;
    m_bufwidth = bufwidth;
}

void TextRender::Blit(FT_Bitmap *bmp, int left, int top)
{
    int runWidth = bmp->width;
    int runHeight = bmp->rows;

    // Precalculate the width
    if((left + bmp->width) >= m_width)
    {
        // Run height needs adjustment
        runWidth = m_width - (left + bmp->width);
    }

    // Precalculate the height
    if((top + bmp->rows) >= m_height)
    {
        // Run height needs adjustment
        runHeight = m_height - (top + bmp->rows);
    }

    // Copy alpha data over, setting the color to the predefined color and the alpha to the value of the glyph
    for(int y = 0; y < runHeight; y++)
    {
        // Precalculate
        int sywidth = (y * bmp->width);
        int dywidth = ((y + top) * m_bufwidth);

        for(int x = 0; x < runWidth; x++)
        {
            // Precalculate
            int srcloc = x + sywidth;
            int dstloc = ((x + left) + dywidth) << 2;

            // Copy data over
            m_buf[dstloc] = m_color.b;
            m_buf[dstloc + 1] = m_color.g;
            m_buf[dstloc + 2] = m_color.r;
            m_buf[dstloc + 3] = ((uint8_t *)bmp->buffer)[srcloc];
        }
    }
}

extern "C" void archThreadSleep(int milliseconds);

void TextRender::RenderSimple(const char *out, bool center, int *sx, int *sy)
{
    // Remember x position
    int maxx = 0;
    int x = DEFAULT_X;
    int y = DEFAULT_Y;

    // Shortcut from examples
    FT_GlyphSlot slot = m_face->glyph;

    // Render
    bool newline = true;
    for(uint32_t i = 0; i < strlen(out); i++)
    {
        if(center && newline) {
            int j = i;
            int s = 0;
            while(out[j] != '\0' && out[j] != '\r' && out[j] != '\n') {
                FT_Error error = FT_Load_Char(m_face, out[j], FT_LOAD_RENDER);
                if(error) break;  /* break on error */
                s += slot->advance.x >> 6;
                j++;
            }
            x += (m_width - s) >> 1;
            if( x < 0 ) x = 0;
            newline = false;

        }
        if(out[i] == '\r' || out[i] == '\n')
        {
            // Newline
            x = DEFAULT_X;
            y += m_fontheight + m_yspacing;
            newline = true;

            continue;
        }
        else if(out[i] == '\t')
        {
            // Truncate to floor (nearest bounds)
            x /= DEFAULT_TAB_SPACE;
            x *= DEFAULT_TAB_SPACE;

            // Add tab stop
            x += DEFAULT_TAB_SPACE;

            continue;
        }

        // Render glyph
        FT_Error error = FT_Load_Char(m_face, out[i], FT_LOAD_RENDER);
        if(error) continue;  /* ignore errors */

        // Blit glyph to surface
        if( sx == NULL && sy == NULL ) {
            Blit(&slot->bitmap, x + slot->bitmap_left, (y + m_fontheight) - slot->bitmap_top - 4);
        }

        // Advance the position
        x += slot->advance.x >> 6;
        if( x > maxx ) maxx = x;
    }
    if( sx ) *sx = maxx + 1;
    if( sy ) *sy = y + m_fontheight + 4;
}

void TextRender::GetTextSize(int *sx, int *sy, bool center, const char *fmt, ...)
{
    // Need to make room for the sprintf'd text
    char *out = (char *)malloc(1024);

    // Build using sprintf
    va_list marker;
    va_start(marker,fmt);
    vsprintf(out,fmt,marker);
    va_end(marker);

    // Call rendering engine
    RenderSimple(out, center, sx, sy);

    // Free memory
    free(out);
}

void TextRender::Render(const char *fmt, ...)
{
    // Need to make room for the sprintf'd text
    char *out = (char *)malloc(1024);

    // Build using sprintf
    va_list marker;
    va_start(marker,fmt);
    vsprintf(out,fmt,marker);
    va_end(marker);

    // Call rendering engine
    RenderSimple(out);

    // Free memory
    free(out);
}
