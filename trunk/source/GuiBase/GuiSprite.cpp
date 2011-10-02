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

#include "GuiSprite.h"

#include <assert.h>
#include <math.h>
#ifndef WII
#include <hge/hgeSprite.h>
#endif

#include "GuiContainer.h"
#include "GuiRootContainer.h"
#include "GuiImage.h"
#include "GuiTextImage.h"


GuiSprite::GuiSprite(GuiContainer *parent, const char *name, GuiImage* image, float x, float y,
                     float clipw, float cliph, float clipx, float clipy) :
           GuiLayer(parent, name)
{
    m_image = NULL;
    m_draw_image = NULL;
    m_clip_x = 0.0f;
    m_clip_y = 0.0f;
#ifndef WII
    m_spr = NULL;
#endif
    if( image != NULL ) {
        SetImage(image, clipw, cliph, clipx, clipy);
    }
    SetPosition(x, y);
}

GuiSprite::~GuiSprite()
{
    CleanUp();
#ifndef WII
    if(m_spr)
        delete m_spr;
#endif
}

void GuiSprite::CleanUp(void)
{
    if( m_image ) {
        GetRootContainer()->ReleaseAtom(m_image);
    }
    m_image = NULL;
    m_draw_image = NULL;
}

void GuiSprite::SetImageIntern(GuiImage* image, GuiTextImage* drawimage,
                               float clipWidth, float clipHeight, float clipOffsetX, float clipOffsetY)
{
    if(drawimage != NULL) {
        image = drawimage;
    }
    if(image == NULL || !image->IsInitialized())return;

#ifndef WII
    // Create HGE sprite object
    if(m_spr) {
        delete m_spr;
    }
    m_spr = new hgeSprite(image->GetTEX(), 0, 0, (float)image->GetWidth(), (float)image->GetHeight());
    m_spr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE);
#endif
    // Free previous image if needed
    CleanUp();

    // Check/setup clipping rect
    if(clipWidth == 0 || (clipWidth + clipOffsetX) > image->GetWidth()) {
        clipWidth = image->GetWidth() - clipOffsetX;
    }
    if(clipHeight == 0 || (clipHeight + clipOffsetY) > image->GetHeight()) {
        clipHeight = image->GetHeight() - clipOffsetY; 
    }
    assert(clipWidth > 0);
    assert(clipHeight > 0);
    SetWidth(clipWidth);
    SetHeight(clipHeight);
    m_clip_x = clipOffsetX;
    m_clip_y = clipOffsetY;

    // Refpixel setting. This positions at the upper left corner.
    SetRefPixelPosition(0,0);

    m_image = image;
    m_draw_image = drawimage;
}

void GuiSprite::SetImage(GuiImage* image, float clipWidth, float clipHeight, float clipOffsetX, float clipOffsetY)
{
    CleanUp();
    GetRootContainer()->UseAtom(image);
    SetImageIntern(image, NULL, clipWidth, clipHeight, clipOffsetX, clipOffsetY);
}

void GuiSprite::SetImage(GuiTextImage* drawimage, float clipWidth, float clipHeight, float clipOffsetX, float clipOffsetY)
{
    CleanUp();
    GetRootContainer()->UseAtom(drawimage);
    SetImageIntern(NULL, drawimage, clipWidth, clipHeight, clipOffsetX, clipOffsetY);
}

GuiImage* GuiSprite::GetImage() const
{
    return m_image;
}

bool GuiSprite::LoadImage(const unsigned char *buf)
{
    CleanUp();
    GuiImage *image = new GuiImage();
    if(image->LoadImage(buf) == IMG_LOAD_ERROR_NONE) {
        SetImageIntern(image, NULL, 0, 0, 0, 0);
        return true;
    }else{
        GetParent()->GetRootContainer()->ReleaseAtom(image);
        return false;
    }
}

bool GuiSprite::LoadImage(const char *file)
{
    CleanUp();
    GuiImage *image = new GuiImage();
    if(image->LoadImage(file) == IMG_LOAD_ERROR_NONE) {
        SetImageIntern(image, NULL, 0, 0, 0, 0);
        return true;
    }else{
        GetParent()->GetRootContainer()->ReleaseAtom(image);
        return false;
    }
}

void GuiSprite::CreateTextImageVA(TextRender* font, int size, int minwidth, int yspace, bool center,
                                  GXColor color, const char *fmt, va_list valist )
{
    // Create new DrawableImage
    GuiTextImage* drawimage = new GuiTextImage();
    drawimage->SetFont(font);
    drawimage->SetSize(size);
    drawimage->SetYSpacing(yspace);
    drawimage->SetColor(color);
    int txtwidth, txtheight;
    drawimage->GetTextSizeVA(&txtwidth, &txtheight, fmt, valist);
    if( minwidth > txtwidth ) {
        txtwidth = minwidth;
    }
    txtwidth = (txtwidth + 3) & ~3;
    txtheight = (txtheight + 3) & ~3;
    drawimage->CreateImage(txtwidth, txtheight);
    drawimage->RenderTextVA(center, fmt, valist);

    SetImage(drawimage, (float)txtwidth, (float)txtheight);
    Delete(drawimage);
}

void GuiSprite::CreateTextImage(TextRender* font, int size, int minwidth, int yspace, bool center,
                                GXColor color, const char *fmt, ... )
{
    va_list marker;
    va_start(marker,fmt);
    CreateTextImageVA(font, size, minwidth, yspace, center, color, fmt, marker);
    va_end(marker);
}

void GuiSprite::CreateDrawImage(int width, int height, int format)
{
    // Create new GuiTextImage
    GuiTextImage* drawimage = new GuiTextImage();
    drawimage->CreateImage(width, height, format);

    SetImage(drawimage, (float)width, (float)height);
    Delete(drawimage);
}

void GuiSprite::FillSolidColor(u8 r, u8 g, u8 b)
{
    assert( m_draw_image );
    m_draw_image->FillSolidColor(r, g, b);
}

u8 *GuiSprite::GetTextureBuffer(void)
{
    assert( m_draw_image );
    return m_draw_image->GetTextureBuffer();
}

void GuiSprite::FlushBuffer(void)
{
    assert( m_draw_image );
    m_draw_image->FlushBuffer();
}

void GuiSprite::Draw(void)
{
    LayerTransform transform = GetTransform();

    if( m_image == NULL ||
        IsVisible() == false || transform.alpha == 0.0f ||
        transform.stretchWidth == 0 || transform.stretchHeight == 0 ||
        (int)GetWidth() == 0 || (int)GetHeight() == 0) return;

#ifdef WII
    #define FRAME_CORRECTION 0.375f //!< Displays frames a lot nicer, still needs a complete fix.

    // Use the sprites texture
    m_image->BindTexture(true);

    // Draw the GuiSprite Quad with transformations
    Mtx model, tmp;
    guMtxIdentity(model);
    guMtxRotDeg(tmp, 'z', transform.rotation/2);
    guMtxConcat(model, tmp, model);
    guMtxTransApply(model, model, transform.offsetX, transform.offsetY, 0.0f);
    guMtxConcat(model, tmp, model);
    GX_LoadPosMtxImm(model, GX_PNMTX0);

    // Now we apply zooming
    float refPixelX = GetRefPixelX() * transform.stretchWidth,
          refPixelY = GetRefPixelY() * transform.stretchHeight,
          refWidth = (GetWidth()-GetRefPixelX()) * transform.stretchWidth,
          refHeight = (GetWidth()-GetRefPixelY()) * transform.stretchHeight;

    // Returns the position of the frame
    if(GetWidth() == 0 || m_image == NULL || !m_image->IsInitialized() || m_image->GetWidth() == 0)return;
    // Calculates the texture position
    f32 txCoords[4] = {
        m_clip_x / m_image->GetWidth() + (FRAME_CORRECTION / m_image->GetWidth()),
        m_clip_y / m_image->GetHeight() + (FRAME_CORRECTION / m_image->GetHeight()),
        (m_clip_x + GetWidth()) / m_image->GetWidth() - (FRAME_CORRECTION / m_image->GetWidth()),
        (m_clip_y + GetHeight()) / m_image->GetHeight() - (FRAME_CORRECTION / m_image->GetHeight()),
    };

    // Normal texture
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        u8 alpha_channel = (u8)(transform.alpha * 255);
        GX_Position2f32(-refPixelX, -refPixelY);
        GX_Color4u8(0xff,0xff,0xff, alpha_channel);
        GX_TexCoord2f32(txCoords[0], txCoords[1]);
        GX_Position2f32(refWidth, -refPixelY);
        GX_Color4u8(0xff,0xff,0xff, alpha_channel);
        GX_TexCoord2f32(txCoords[2], txCoords[1]);
        GX_Position2f32(refWidth, refHeight);
        GX_Color4u8(0xff,0xff,0xff, alpha_channel);
        GX_TexCoord2f32(txCoords[2], txCoords[3]);
        GX_Position2f32(-refPixelX, refHeight);
        GX_Color4u8(0xff,0xff,0xff, alpha_channel);
        GX_TexCoord2f32(txCoords[0], txCoords[3]);
    GX_End();

#else
    assert( transform.alpha <= 1.0f );
    m_spr->SetColor(((u32)(transform.alpha * 255) << 24) + 0xffffff);
    m_spr->SetHotSpot(GetRefPixelX(), GetRefPixelY());
    m_spr->SetTextureRect(m_clip_x, m_clip_y, GetWidth(), GetHeight());
    m_spr->RenderEx((float)(transform.offsetX+1), (float)(transform.offsetY+20+1), (transform.rotation / 180.0f) * M_PI,
                  transform.stretchWidth, transform.stretchHeight);
#endif
}

