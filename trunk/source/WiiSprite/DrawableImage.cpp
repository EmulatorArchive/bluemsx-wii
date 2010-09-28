// Code by DragonMinded
// Please attribute, nothing more

#include <assert.h>
#include <string.h>
#include "DrawableImage.h"
#include <malloc.h>
#include <wiisprite.h>

DrawableImage::DrawableImage()
{
    // Ensure we never draw if we haven't created
    _initialized = false;

    // Ensure proper null pointer
#ifdef WII
    _pixels = NULL;
#else
    _texObj = NULL;
#endif

    // Set default font style
    _font_size = 20;
    _font_yspacing = 0;
    _font_color.r = _font_color.g = _font_color.b = _font_color.a = 0xff;
}

DrawableImage::~DrawableImage()
{
    DestroyImage();
}

void DrawableImage::CreateImage(int width, int height, int format)
{
    // Set parameters
    _format = format;
    _width = width;
    _height = height;
#ifdef WII
    int bytespp = (format == GX_TF_RGB565)? 2 : 4;

    // Allocate room
    if(_pixels)
    {
        free(_pixels);
    }
    _pixels = (u8 *)memalign(32, _width * _height * bytespp);

    // Set to zero's for now
    memset(_pixels, 0, _width * _height * bytespp);

    // Move flush cached memory
    DCFlushRange (_pixels, _width * _height * bytespp);
#else
    // Create texture
    GameWindow::Lock();
    _texObj = g_hge->Texture_Create(width, height);
    assert(_texObj);

    // Clear it
    u8 *blitbuf = (u8 *)g_hge->Texture_Lock(_texObj, false);
    int tex_width = g_hge->Texture_GetWidth(_texObj);
    int tex_height = g_hge->Texture_GetHeight(_texObj);
    memset(blitbuf, 0, tex_width * tex_height * 4);
    g_hge->Texture_Unlock(_texObj);
    GameWindow::Unlock();
#endif

    // Set sprite as valid
    _initialized = true;
}

void DrawableImage::FillSolidColor(u8 r, u8 g, u8 b, u8 a)
{
#ifdef WII
    assert(_format == GX_TF_RGB565); // Only GX_TF_RGB565 supported
    assert(a == 0xff); // Transparency not supported
    u16 *p = (u16*)_pixels;
    u16 c = ((u16)(r >> 3) << 11) | ((u16)(g >> 3) << 6) | (b >> 3);
    for(u32 i = 0; i < _width*_height; i++) {
        *p++ = c;
    }
#else
    GameWindow::Lock();
    u32 *blitbuf = (u32 *)g_hge->Texture_Lock(_texObj, false);
    int tex_width = g_hge->Texture_GetWidth(_texObj);
    int tex_height = g_hge->Texture_GetHeight(_texObj);
    for(int i = 0; i < (tex_height*tex_width); i++) {
        *blitbuf++ = ((u32)a << 24) | ((u32)r << 16) | ((u32)g << 8) | b;
    }
    g_hge->Texture_Unlock(_texObj);
    GameWindow::Unlock();
#endif
}

void DrawableImage::DestroyImage(void)
{
#ifdef WII
    if(_pixels)
    {
        free(_pixels);
        _pixels = NULL;
    }
#else
    if(_texObj)
    {
        GameWindow::Lock();
        g_hge->Texture_Free(_texObj);
        GameWindow::Unlock();
        _texObj = NULL;
    }
#endif
}

void DrawableImage::BindTexture(bool bilinear)
{
#ifdef WII
    if(!_initialized)return;
        if(GameWindow::_lastimagebilinear == bilinear)
            if(GameWindow::_lastimage == this)
                return;

    GX_InitTexObj(&_texObj, _pixels, _width, _height, _format, GX_CLAMP, GX_CLAMP, GX_FALSE);

    // This disables bilinear filtering if applicable
    if(!bilinear)
        GX_InitTexObjLOD(&_texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);

    GX_LoadTexObj(&_texObj,GX_TEXMAP0);

    GameWindow::_lastimage = this;
    GameWindow::_lastimagebilinear = bilinear;
#endif
}

u32 DrawableImage::GetWidth() const
{
    return _width;
}

u32 DrawableImage::GetHeight() const
{
    return _height;
}

bool DrawableImage::IsInitialized() const
{
    return _initialized;
}

void DrawableImage::CopyBuffer(u8 *buf)
{
#ifdef WII
    // This function assumes the u8 buffer is RGBA quads
    if( _format != GX_TF_RGBA8 )
        return;

    u8 *pixBuf = _pixels;

    // Loop in chunks of 4
    for(u32 j = 0; j < _height; j += 4)
    {
        for(u32 i = 0; i < _width; i += 4)
        {
            // First, copy AR chunks
            for(int y = 0; y < 4; y++)
            {
                for(int x = 0; x < 4; x++)
                {
                    // Alpha
                    *pixBuf++ = buf[(((i + x) + ((j + y) * _width)) * 4) + 3];

                    // Red
                    *pixBuf++ = buf[((i + x) + ((j + y) * _width)) * 4];
                }
            }

            // Now, copy GB chunks
            for(int y = 0; y < 4; y++)
            {
                for(int x = 0; x < 4; x++)
                {
                    // Green
                    *pixBuf++ = buf[(((i + x) + ((j + y) * _width)) * 4) + 1];

                    // Blue
                    *pixBuf++ = buf[(((i + x) + ((j + y) * _width)) * 4) + 2];
                }
            }
        }
    }

    // Move flush cached memory
    DCFlushRange (_pixels, _width * _height * 4);
#endif
}

Image *DrawableImage::GetImage()
{
    return (Image *)this;
}

u8 *DrawableImage::GetTextureBuffer(void)
{
#ifdef WII
    return _pixels;
#else
    return NULL;
#endif
}

void DrawableImage::FlushBuffer(void)
{
#ifdef WII
    int bytespp = (_format == GX_TF_RGB565)? 2 : 4;
    DCFlushRange (_pixels, _width * _height * bytespp);
#endif
}

void DrawableImage::SetFont(TextRender* f)
{
    font = f;
}

void DrawableImage::SetColor(GXColor c)
{
    _font_color = c;
}

void DrawableImage::SetSize(int s)
{
    _font_size = s;
}

void DrawableImage::SetYSpacing(int s)
{
    _font_yspacing = s;
}

void DrawableImage::RenderTextVA(bool center, const char *fmt, va_list list)
{
    // This function assumes the u8 buffer is RGBA quads
    if( _format != GX_TF_RGBA8 )
        return;
#ifndef WII
    if( _texObj == NULL )
        return;
#endif
    // Set font style
    font->SetColor(_font_color);
    font->SetSize(_font_size);
    font->SetYSpacing(_font_yspacing);

    // Need to make room for the sprintf'd text
    char *out = (char *)malloc(1024);

    // Need temporary blit buffer
#ifdef WII
    u8 *blitbuf = (u8 *)memalign(32, _width * _height * 4);
    memset(blitbuf, 0, _width * _height * 4);
#else
    u8 *blitbuf = (u8 *)g_hge->Texture_Lock(_texObj, false);
    int tex_width = g_hge->Texture_GetWidth(_texObj);
    int tex_height = g_hge->Texture_GetHeight(_texObj);
    memset(blitbuf, 0, tex_width * tex_height * 4);
#endif

    // Build using sprintf
    vsprintf(out,fmt,list);

    // Set up buffer
#ifdef WII
    font->SetBuffer(blitbuf, _width, _height, _width);
#else
    font->SetBuffer(blitbuf, _width, _height, tex_width);
#endif

    // Call rendering engine
    font->RenderSimple(out, center);

    // Blit to surface
    CopyBuffer(blitbuf);

    // Free memory
#ifdef WII
    free(blitbuf);
#else
    g_hge->Texture_Unlock(_texObj);
    GameWindow::Unlock();
#endif
    free(out);
}

void DrawableImage::RenderText(const char *fmt, ...)
{
    va_list marker;
    va_start(marker,fmt);
    RenderTextVA(false, fmt,marker);
    va_end(marker);
}

void DrawableImage::RenderText(bool center, const char *fmt, ...)
{
    va_list marker;
    va_start(marker,fmt);
    RenderTextVA(center, fmt,marker);
    va_end(marker);
}

void DrawableImage::GetTextSize(int *sx, int *sy, const char *fmt, ...)
{
    // Need to make room for the sprintf'd text
#ifdef WII
    char *out = (char *)memalign(32, 1024);
#else
    char *out = (char *)malloc(1024);
#endif

    // Set font style
    font->SetColor(_font_color);
    font->SetSize(_font_size);
    font->SetYSpacing(_font_yspacing);

    // Build using sprintf
    va_list marker;
    va_start(marker,fmt);
    vsprintf(out,fmt,marker);
    va_end(marker);

    // Call rendering engine
    font->SetBuffer(NULL, 0x7fffffff, 0x7fffffff, 0x7fffffff);
    font->RenderSimple(out, false, sx, sy);

    // Free memory
    free(out);
}

