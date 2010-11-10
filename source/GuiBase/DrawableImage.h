// Code by DragonMinded
// Please attribute, nothing more

#ifndef __DRAWABLE_H
#define __DRAWABLE_H

#include <stdarg.h>

#include "GuiImage.h"

class TextRender;

class DrawableImage : public GuiImage
{
  public:
    DrawableImage();
    virtual ~DrawableImage();

    void CreateImage(int width, int height, int format = GX_TF_RGBA8);
        void DestroyImage(void);

    u32 GetWidth() const;
    u32 GetHeight() const;
    bool IsInitialized() const;
    void BindTexture(bool bilinear = true);

    GuiImage *GetImage();
    void CopyBuffer(u8 *buf);

    void SetFont(TextRender* text);
    void SetColor(GXColor c);
    void SetSize(int s);
    void SetYSpacing(int s);
    void FillSolidColor(u8 r, u8 g, u8 b);
    void RenderText(const char *fmt, ...);
    void RenderText(bool center, const char *fmt, ...);
    void RenderTextVA(bool center, const char *fmt, va_list list);
    void GetTextSize(int *sx, int *sy, const char *fmt, ...);
    void GetTextSizeVA(int *sx, int *sy, const char *fmt, va_list list);
    u8 *GetTextureBuffer(void);
    void FlushBuffer(void);
  private:
    u8* _pixels;
    int _format;
    int _font_size;
    int _font_yspacing;
    GXColor _font_color;

    TextRender* font;
};

#endif
