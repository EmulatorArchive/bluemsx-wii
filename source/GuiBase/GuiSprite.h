/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
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

#ifndef GUIBASE_SPRITE
#define GUIBASE_SPRITE

#include <stdarg.h>
#include <stdlib.h>
#include <gccore.h>

#include "GuiLayer.h"
#include "GuiImage.h"
#include "GuiRect.h"

#ifndef WII
class hgeSprite;
#endif
class GuiContainer;
class GuiTextImage;
class TextRender;

class GuiSprite : public GuiLayer {
public:
    GuiSprite(GuiContainer *parent, const char *name, GuiImage* image=NULL, float x=0.0f, float y=0.0f, GuiRect rect = GuiRect());
    virtual ~GuiSprite();

    void CleanUp(void);

    void SetImage(GuiImage* image, GuiRect rect = GuiRect());
    void SetImage(GuiTextImage* drawimage, GuiRect rect = GuiRect());
    GuiImage* GetImage() const;

    // Image
    bool LoadImage(const unsigned char *buf);
    bool LoadImage(const char *file);

    // GuiTextImage
    void CreateDrawImage(int width, int height, int format = GX_TF_RGBA8);
    void CreateTextImageVA(TextRender* font, int size, int minwidth, int yspace, bool center,
                             GXColor color, const char *fmt, va_list valist);
    void CreateTextImage(TextRender* font, int size, int minwidth, int yspace, bool center,
                             GXColor color, const char *fmt, ...);
    void FillSolidColor(u8 r, u8 g, u8 b);
    u8 *GetTextureBuffer(void);
    void FlushBuffer(void);

    // Draws the GuiSprite.
    void Draw(void);
private:
    void SetImageIntern(GuiImage* image, GuiTextImage* drawimage, GuiRect rect = GuiRect());
#ifndef WII
    hgeSprite *m_spr;
#endif
    float m_clip_x;
    float m_clip_y;
    GuiImage* m_image;
    GuiTextImage* m_draw_image;
};

#endif
