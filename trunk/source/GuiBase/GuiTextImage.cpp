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

#include "GuiTextImage.h"

#include <assert.h>
#include <string.h>
#include <malloc.h>

#include "GameWindow.h"
#include "TextRender.h"
#include "GuiRootContainer.h"

GuiTextImage::GuiTextImage()
{
    // Ensure we never draw if we haven't created
    m_initialized = false;

    // Ensure proper null pointer
    m_pixels = NULL;
#ifndef WII
    m_texObj = NULL;
#endif
    font = NULL;

    // Set default font style
    m_font_size = 20;
    m_font_yspacing = 0;
    m_font_color.r = m_font_color.g = m_font_color.b = m_font_color.a = 0xff;
}

GuiTextImage::~GuiTextImage()
{
    DestroyImage();
    if( font != NULL ) {
        GetRootContainer()->ReleaseAtom(font);
    }
}

void GuiTextImage::CreateImage(int width, int height, int format)
{
    // Set parameters
    m_format = format;
    m_width = width;
    m_height = height;

    int bytespp = (format == GX_TF_RGB565)? 2 : 4;

    // Allocate room
    if(m_pixels)
    {
        free(m_pixels);
    }
#ifdef WII
    m_pixels = (u8 *)memalign(32, m_width * m_height * bytespp);
#else
    m_pixels = (u8 *)malloc(m_width * m_height * bytespp);
#endif

    // Set to zero's for now
    memset(m_pixels, 0, m_width * m_height * bytespp);

#ifdef WII
    // Move flush cached memory
    DCFlushRange (m_pixels, m_width * m_height * bytespp);
#else
    // Create texture
    GameWindow::Lock();
    m_texObj = g_hge->Texture_Create(width, height);
    assert(m_texObj);

    // Clear it
    u8 *blitbuf = (u8 *)g_hge->Texture_Lock(m_texObj, false);
    int tex_width = g_hge->Texture_GetWidth(m_texObj);
    int tex_height = g_hge->Texture_GetHeight(m_texObj);
    memset(blitbuf, 0, tex_width * tex_height * 4);
    g_hge->Texture_Unlock(m_texObj);
    GameWindow::Unlock();
#endif

    // Set sprite as valid
    m_initialized = true;
}

void GuiTextImage::FillSolidColor(u8 r, u8 g, u8 b)
{
    assert(m_format == GX_TF_RGB565); // Only GX_TF_RGB565 supported
    u16 *p = (u16*)m_pixels;
    u16 c = ((u16)(r >> 3) << 11) | ((u16)(g >> 3) << 6) | (b >> 3);
    for(png_uint_32 i = 0; i < m_width*m_height; i++) {
        *p++ = c;
    }
    FlushBuffer();
}

void GuiTextImage::DestroyImage(void)
{
    if(m_pixels)
    {
        free(m_pixels);
        m_pixels = NULL;
    }
#ifndef WII
    if(m_texObj)
    {
        GameWindow::Lock();
        g_hge->Texture_Free(m_texObj);
        GameWindow::Unlock();
        m_texObj = NULL;
    }
#endif
}

void GuiTextImage::BindTexture(bool bilinear)
{
#ifdef WII
    if(!m_initialized)return;
        if(GameWindow::_lastimagebilinear == bilinear)
            if(GameWindow::_lastimage == this)
                return;

    GX_InitTexObj(&m_texObj, m_pixels, m_width, m_height, m_format, GX_CLAMP, GX_CLAMP, GX_FALSE);

    // This disables bilinear filtering if applicable
    if(!bilinear)
        GX_InitTexObjLOD(&m_texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);

    GX_LoadTexObj(&m_texObj,GX_TEXMAP0);

    GameWindow::_lastimage = this;
    GameWindow::_lastimagebilinear = bilinear;
#endif
}

float GuiTextImage::GetWidth() const
{
    return (float)m_width;
}

float GuiTextImage::GetHeight() const
{
    return (float)m_height;
}

bool GuiTextImage::IsInitialized() const
{
    return m_initialized;
}

void GuiTextImage::CopyBuffer(u8 *buf)
{
#ifdef WII
    // This function assumes the u8 buffer is RGBA quads
    if( m_format != GX_TF_RGBA8 )
        return;

    u8 *pixBuf = m_pixels;

    // Loop in chunks of 4
    for(int j = 0; j < m_height; j += 4)
    {
        for(int i = 0; i < m_width; i += 4)
        {
            // First, copy AR chunks
            for(int y = 0; y < 4; y++)
            {
                for(int x = 0; x < 4; x++)
                {
                    // Alpha
                    *pixBuf++ = buf[(((i + x) + ((j + y) * m_width)) * 4) + 3];

                    // Red
                    *pixBuf++ = buf[((i + x) + ((j + y) * m_width)) * 4];
                }
            }

            // Now, copy GB chunks
            for(int y = 0; y < 4; y++)
            {
                for(int x = 0; x < 4; x++)
                {
                    // Green
                    *pixBuf++ = buf[(((i + x) + ((j + y) * m_width)) * 4) + 1];

                    // Blue
                    *pixBuf++ = buf[(((i + x) + ((j + y) * m_width)) * 4) + 2];
                }
            }
        }
    }

    // Move flush cached memory
    DCFlushRange (m_pixels, m_width * m_height * 4);
#endif
}

GuiImage *GuiTextImage::GetImage()
{
    return (GuiImage *)this;
}

u8 *GuiTextImage::GetTextureBuffer(void)
{
    return m_pixels;
}

void GuiTextImage::FlushBuffer(void)
{
#ifdef WII
    int bytespp = (m_format == GX_TF_RGB565)? 2 : 4;
    DCFlushRange (m_pixels, m_width * m_height * bytespp);
#else
    if( m_format == GX_TF_RGB565 ) {
        GameWindow::Lock();
        u32 *blitbuf = (u32 *)g_hge->Texture_Lock(m_texObj, false);
        int tex_width = g_hge->Texture_GetWidth(m_texObj);
        u16 *p = (u16*)m_pixels;
        for(int y = 0; y < (int)m_height; y += 4) {
            u32 *pd[4];
            pd[0] = blitbuf + y * tex_width;
            pd[1] = pd[0] + tex_width;
            pd[2] = pd[1] + tex_width;
            pd[3] = pd[2] + tex_width;
            for(int x = 0; x < (int)m_width; x += 4) {
                for(int i = 0; i < 4; i++) {
                    *pd[i]++ = ((u32)0xff << 24) | ((u32)((u8)(*p >> 11) << 3) << 16) | ((u32)((u8)(*p >> 6) << 3) << 8) | ((u8)(*p) << 3);
                    p++;
                    *pd[i]++ = ((u32)0xff << 24) | ((u32)((u8)(*p >> 11) << 3) << 16) | ((u32)((u8)(*p >> 6) << 3) << 8) | ((u8)(*p) << 3);
                    p++;
                    *pd[i]++ = ((u32)0xff << 24) | ((u32)((u8)(*p >> 11) << 3) << 16) | ((u32)((u8)(*p >> 6) << 3) << 8) | ((u8)(*p) << 3);
                    p++;
                    *pd[i]++ = ((u32)0xff << 24) | ((u32)((u8)(*p >> 11) << 3) << 16) | ((u32)((u8)(*p >> 6) << 3) << 8) | ((u8)(*p) << 3);
                    p++;
                }
            }
        }
        g_hge->Texture_Unlock(m_texObj);
        GameWindow::Unlock();
    }
#endif
}

void GuiTextImage::SetFont(TextRender* f)
{
    if( font != NULL ) {
        GetRootContainer()->ReleaseAtom(font);
    }
    if( f != NULL ) {
        GetRootContainer()->UseAtom(f);
    }
    font = f;
}

void GuiTextImage::SetColor(GXColor c)
{
    m_font_color = c;
}

void GuiTextImage::SetSize(int s)
{
    m_font_size = s;
}

void GuiTextImage::SetYSpacing(int s)
{
    m_font_yspacing = s;
}

void GuiTextImage::RenderTextVA(bool center, const char *fmt, va_list list)
{
    // This function assumes the u8 buffer is RGBA quads
    if( m_format != GX_TF_RGBA8 )
        return;
#ifndef WII
    if( m_texObj == NULL )
        return;
#endif
    // Set font style
    font->SetColor(m_font_color);
    font->SetSize(m_font_size);
    font->SetYSpacing(m_font_yspacing);

    // Need to make room for the sprintf'd text
    char *out = (char *)malloc(1024);

    // Need temporary blit buffer
#ifdef WII
    u8 *blitbuf = (u8 *)memalign(32, m_width * m_height * 4);
    memset(blitbuf, 0, m_width * m_height * 4);
#else
    GameWindow::Lock();
    u8 *blitbuf = (u8 *)g_hge->Texture_Lock(m_texObj, false);
    int tex_width = g_hge->Texture_GetWidth(m_texObj);
    int tex_height = g_hge->Texture_GetHeight(m_texObj);
    memset(blitbuf, 0, tex_width * m_height * 4);
#endif

    // Build using sprintf
    vsprintf(out,fmt,list);

    // Set up buffer
#ifdef WII
    font->SetBuffer(blitbuf, m_width, m_height, m_width);
#else
    font->SetBuffer(blitbuf, m_width, m_height, tex_width);
#endif

    // Call rendering engine
    font->RenderSimple(out, center);

    // Blit to surface
    CopyBuffer(blitbuf);

    // Free memory
#ifdef WII
    free(blitbuf);
#else
    g_hge->Texture_Unlock(m_texObj);
    GameWindow::Unlock();
#endif
    free(out);
}

void GuiTextImage::RenderText(const char *fmt, ...)
{
    va_list marker;
    va_start(marker,fmt);
    RenderTextVA(false, fmt,marker);
    va_end(marker);
}

void GuiTextImage::RenderText(bool center, const char *fmt, ...)
{
    va_list marker;
    va_start(marker,fmt);
    RenderTextVA(center, fmt,marker);
    va_end(marker);
}

void GuiTextImage::GetTextSizeVA(int *sx, int *sy, const char *fmt, va_list list)
{
    // Need to make room for the sprintf'd text
#ifdef WII
    char *out = (char *)memalign(32, 1024);
#else
    char *out = (char *)malloc(1024);
#endif

    // Set font style
    font->SetColor(m_font_color);
    font->SetSize(m_font_size);
    font->SetYSpacing(m_font_yspacing);

    // Build using sprintf
    vsprintf(out, fmt, list);

    // Call rendering engine
    font->SetBuffer(NULL, 0x7fffffff, 0x7fffffff, 0x7fffffff);
    font->RenderSimple(out, false, sx, sy);

    // Free memory
    free(out);
}

void GuiTextImage::GetTextSize(int *sx, int *sy, const char *fmt, ...)
{
    va_list marker;
    va_start(marker,fmt);
    GetTextSizeVA(sx, sy, fmt, marker);
    va_end(marker);
}

