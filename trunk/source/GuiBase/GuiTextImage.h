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
#ifndef __GUI_TEXT_IMAGE_H
#define __GUI_TEXT_IMAGE_H

#include <stdarg.h>

#include "GuiImage.h"

class TextRender;

class GuiTextImage : public GuiImage
{
  public:
    GuiTextImage();
    virtual ~GuiTextImage();

    void CreateImage(int width, int height, int format = GX_TF_RGBA8);
        void DestroyImage(void);

    float GetWidth() const;
    float GetHeight() const;
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
    u8* m_pixels;
    int m_format;
    int m_font_size;
    int m_font_yspacing;
    GXColor m_font_color;

    TextRender* font;
};

#endif
