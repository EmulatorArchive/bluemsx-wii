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
#ifndef __TEXTRENDER_H
#define __TEXTRENDER_H

#define DEFAULT_X 0
#define DEFAULT_Y 0

#define DEFAULT_Y_CUSHION 2
#define DEFAULT_TAB_SPACE 50

#include "GuiAtom.h"

#include <stdint.h>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <gccore.h>

using std::string;

class TextRender : public GuiAtom
{
    public:
        TextRender();
        ~TextRender();

        void SetFont(string font);
        void SetFont(const unsigned char* font, int size);
        void SetColor(GXColor c);
        void SetSize(int s);
        void SetYSpacing(int s);

        void SetBuffer(uint8_t *buf, int width, int height, int bufwidth);
        void RenderSimple(const char *out, bool center = false, int *sx = NULL, int *sy = NULL);
        void Render(const char *fmt, ...);
        void GetTextSize(int *sx, int *sy, bool center, const char *fmt, ...);
    private:
        void Blit(FT_Bitmap *bmp, int bmpWidth, int bmpHeight);

        FT_Library m_library;
        FT_Face m_face;

        uint8_t *m_buf;
        int m_width;
        int m_height;
        int m_bufwidth;
        int m_fontheight;
        int m_yspacing;
        GXColor m_color;
};

#endif

