
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
#include "GuiTiles.h"

#define FRAME_CORRECTION 0.375f //!< Displays frames a lot nicer, still needs a complete fix.


GuiSprite::GuiSprite(GuiContainer *parent, const char *name, GuiImage* image, int x, int y,
                     int clipw, int cliph, int clipx, int clipy) :
    GuiLayer(parent, name),
    _image(NULL), _draw_image(NULL),
    _colRect(NULL)
{
#ifndef WII
    spr = NULL;
#endif
    _colRect = new Rect();
    if( image != NULL ) {
        SetImage(image, clipw, cliph, clipx, clipy);
    }
    SetPosition((f32)x,(f32)y);
}
GuiSprite::~GuiSprite(){
    if(_colRect)
        delete _colRect; _colRect = NULL;
    CleanUp();
#ifndef WII
    if(spr)
        delete spr;
#endif
}

void GuiSprite::CleanUp(void)
{
    if( _image ) {
        GetParent()->GetRootContainer()->ReleaseAtom(_image);
    }
    _image = NULL;
    _draw_image = NULL;
}

void GuiSprite::SetImageIntern(GuiImage* image, GuiTextImage* drawimage,
                               u32 clipWidth, u32 clipHeight, u32 clipOffsetX, u32 clipOffsetY)
{
    if(drawimage != NULL) {
        image = drawimage;
    }
    if(image == NULL || !image->IsInitialized())return;

#ifndef WII
    // Create HGE sprite object
    if(spr) {
        delete spr;
    }
    spr = new hgeSprite(image->GetTEX(), 0, 0, (float)image->GetWidth(), (float)image->GetHeight());
    spr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE);
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
    _width = clipWidth; _height = clipHeight;
    _clip_x = clipOffsetX; _clip_y = clipOffsetY;

    // Set the new collision data
    _colRect->x = 0; _colRect->y = 0;
    _colRect->width = (f32)_width; _colRect->height = (f32)_height;

    // Refpixel setting. This positions at the upper left corner.
    SetRefPixelPosition(0,0);

    _image = image;
    _draw_image = drawimage;
}

void GuiSprite::SetImage(GuiImage* image, u32 clipWidth, u32 clipHeight, u32 clipOffsetX, u32 clipOffsetY)
{
    GetParent()->GetRootContainer()->UseAtom(image);
    SetImageIntern(image, NULL, clipWidth, clipHeight, clipOffsetX, clipOffsetY);
}
void GuiSprite::SetImage(GuiTextImage* drawimage, u32 clipWidth, u32 clipHeight, u32 clipOffsetX, u32 clipOffsetY)
{
    GetParent()->GetRootContainer()->UseAtom(drawimage);
    SetImageIntern(NULL, drawimage, clipWidth, clipHeight, clipOffsetX, clipOffsetY);
}

GuiImage* GuiSprite::GetImage() const{
    return _image;
}

bool GuiSprite::LoadImage(const unsigned char *buf){
    GuiImage *image = new GuiImage();
    if(image->LoadImage(buf) == IMG_LOAD_ERROR_NONE) {
        SetImageIntern(image, NULL, 0, 0, 0, 0);
        return true;
    }else{
        GetParent()->GetRootContainer()->ReleaseAtom(image);
        return false;
    }
}
bool GuiSprite::LoadImage(const char *file){
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
    // Free previous image if needed
    CleanUp();

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

    SetImage(drawimage, txtwidth, txtheight);
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
    // Free previous image if needed
    CleanUp();

    // Create new GuiTextImage
    GuiTextImage* drawimage = new GuiTextImage();
    drawimage->CreateImage(width, height, format);

    SetImage(drawimage, width, height);
    Delete(drawimage);
}

void GuiSprite::FillSolidColor(u8 r, u8 g, u8 b)
{
    assert( _draw_image );
    _draw_image->FillSolidColor(r, g, b);
}

u8 *GuiSprite::GetTextureBuffer(void)
{
    assert( _draw_image );
    return _draw_image->GetTextureBuffer();
}

void GuiSprite::FlushBuffer(void)
{
    assert( _draw_image );
    _draw_image->FlushBuffer();
}

void GuiSprite::DefineCollisionRectangle(f32 x, f32 y, f32 width, f32 height){
    _colRect->x = x;
    _colRect->y = y;
    _colRect->width = width;
    _colRect->height = height;
}
const Rect* GuiSprite::GetCollisionRectangle() const{
    return _colRect;
}

COLL GuiSprite::CollidesWith(const Rect* rect, f32 x, f32 y)
{
    LayerTransform transform = GetTransform();

    if(rect == NULL) return COLL_NO_COLLISION;

    if( !transform.valid ) return COLL_UNKNOWN;

    // Check if the rectangle is not in the other rectangle
    if(_colRect->y + transform.offsetY + _colRect->height <= rect->y + y ||
        _colRect->y + transform.offsetY >= rect->y + y + rect->height ||
        _colRect->x + transform.offsetX +_colRect->width <= rect->x + x ||
        _colRect->x + transform.offsetX >= rect->x + x + rect->width)
    {
        return COLL_NO_COLLISION;
    }

    return COLL_COLLISION;
}

COLL GuiSprite::CollidesWith(GuiSprite* sprite, bool complete)
{
    if(sprite == NULL)return COLL_NO_COLLISION;

    LayerTransform me = GetTransform();
    LayerTransform spr = sprite->GetTransform();

    if( !me.valid || !spr.valid ) return COLL_UNKNOWN;

    if(!complete){
        // Some simple collision detecting with the base collision rectangle.
        const Rect* collision = sprite->GetCollisionRectangle();
        return CollidesWith(collision, spr.offsetX, spr.offsetY);
    }

    // Advanced rectangle collision detecting with zoom and rectangle.
    // Code used from http://www.ragestorm.net/sample?id=80 and modified
    // to get it working properly with libwiisprite.

    // Rotation is angle/2
    f32 angle1 = (f32)me.rotation*M_PI/90,
        angle2 = (f32)spr.rotation*M_PI/90;

    Rect rect[4]; // Points which help calculate this stuff.

    f32 cosa, sina, // For sin/cos calculations
        temp, x, a, // Temp vars for various useage.
        deltaX, // For linear equations
        vertical1, vertical2; // Min/max vertical values

    // Init data
    rect[0].x = (f32)(spr.offsetX+(f32)(sprite->GetWidth()/2));
    rect[0].y = (f32)(spr.offsetY+(f32)(sprite->GetHeight()/2));
    rect[1].x = (f32)(me.offsetX+(f32)((GetWidth()/2)));
    rect[1].y = (f32)(me.offsetY+(f32)((GetHeight()/2)));
    rect[0].x -= sprite->GetRefPixelX();
    rect[0].y -= sprite->GetRefPixelY();
    rect[1].x -= GetRefPixelX();
    rect[1].x -= GetRefPixelY();
    rect[0].width = (f32)(GetWidth()/2)*me.stretchWidth;
    rect[0].height = (f32)(GetHeight()/2)*me.stretchHeight;
    rect[1].width = (f32)(sprite->GetWidth()/2)*spr.stretchWidth;
    rect[1].height = (f32)(sprite->GetHeight()/2)*spr.stretchHeight;

    // Move the sprite by the other sprites values
    rect[0].x -= rect[1].x; rect[0].y -= rect[1].y;

    // Rotate the other sprite clockwise by its angle to make it axis-aligned
    cosa = (f32)cos(angle2),
    sina = (f32)sin(angle2);
    temp = rect[0].x; rect[0].x = temp*cosa + rect[0].y*sina; rect[0].y = -temp*sina + rect[0].y*cosa;

    // Calculate the points of the other sprite.
    rect[2].width = rect[2].x = rect[0].x;
    rect[2].height = rect[2].y = rect[0].y;
    rect[2].width -= rect[1].width; rect[2].height -= rect[1].height;
    rect[2].x += rect[1].width; rect[2].y += rect[1].height;

    cosa = (f32)cos(angle1-angle2);
    sina = (f32)sin(angle1-angle2);
    // Calculate the points of this sprite
    rect[3].x = -rect[0].height*sina; rect[3].width = rect[3].x; temp = rect[0].width*cosa; rect[3].x += temp; rect[3].width -= temp;
    rect[3].y =  rect[0].height*cosa; rect[3].height = rect[3].y; temp = rect[0].width*sina; rect[3].y += temp; rect[3].height -= temp;

    temp = sina*cosa;

    // Check for the right A and B values, swap them if they're not
    if(temp < 0){
        temp = rect[3].x; rect[3].x = rect[3].width; rect[3].width = temp;
        temp = rect[3].y; rect[3].y = rect[3].height; rect[3].height = temp;
    }

    // Verify that B is the vertex closest to the left
    if(sina < 0){
        rect[3].width = -rect[3].width; rect[3].height = -rect[3].height;
    }

    // Check B if it is in the range that needs to be checked
    if(rect[3].width > rect[2].x || rect[3].width > -rect[2].width) return COLL_NO_COLLISION;

    // Check if they are aligned to the axis, checking is easier then
    if(temp == 0){
        vertical1 = rect[3].y; vertical2 = -vertical1;
    }else{
        // Check for the vertical range

        x = rect[2].width-rect[3].x; a = rect[2].x-rect[3].x;
        vertical1 = rect[3].y;

        // If the first vertical range is not between the given values,
        // check on the value itself
        if(a*x > 0){
            deltaX = rect[3].x;
            if(x < 0){
                deltaX -= rect[3].width; vertical1 -= rect[3].height; x = a;
            }else{
                deltaX += rect[3].width; vertical1 += rect[3].height;
            }
            vertical1 *= x; vertical1 /= deltaX; vertical1 += rect[3].y;
        }

        x = rect[2].width+rect[3].x; a = rect[2].x+rect[3].x;
        vertical2 = -rect[3].y;

        //If the second vertical range is not between the given values,
        //check on the value itself
        if(a*x > 0){
            deltaX = -rect[3].x;
            if(x < 0){
                deltaX -= rect[3].width; vertical2 -= rect[3].height; x = a;
            }else{
                deltaX += rect[3].width; vertical2 += rect[3].height;
            }
            vertical2 *= x; vertical2 /= deltaX; vertical2 -= rect[3].y;
        }
    }

    //Check if this sprite is in the vertical range of the other. Returns either true or false.
    return ((vertical1 < rect[2].height && vertical2 < rect[2].height) ||
            (vertical1 > rect[2].y && vertical2 > rect[2].y))? COLL_NO_COLLISION : COLL_COLLISION;
}

COLL GuiSprite::CollidesWith(GuiTiles* tiledlayer)
{
    LayerTransform transform = GetTransform();

    if( !transform.valid ) return COLL_UNKNOWN;

    if(tiledlayer == NULL ||
        _colRect->x + transform.offsetX < 0 || _colRect->y + transform.offsetY < 0 ||
        tiledlayer->GetCellWidth() == 0 || tiledlayer->GetCellHeight() == 0) return COLL_NO_COLLISION;

    // Get on which tiles the sprite is drawn
    Rect rect;
    rect.x = (f32)((_colRect->x + transform.offsetX)/tiledlayer->GetCellWidth());
    rect.y = (f32)((_colRect->y + transform.offsetY)/tiledlayer->GetCellHeight());
    rect.width = (f32)((_colRect->x + transform.offsetX + _colRect->width)/tiledlayer->GetCellWidth());
    rect.height = (f32)((_colRect->y + transform.offsetY + _colRect->height)/tiledlayer->GetCellHeight());

    for(s32 y = (s32)rect.y; y < rect.height+1; y++){
        for(s32 x = (s32)rect.x; x < rect.width+1; x++){
            // Since checks are done inside the tiledlayer, we do not need to check here
            s32 data = tiledlayer->GetCell(x, y);
            if(data < 0)data = tiledlayer->GetAnimatedTile(data);
            if(data != 0){
                return COLL_COLLISION;
            }
        }
    }
    return COLL_NO_COLLISION;
}

void GuiSprite::Draw(void)
{
    LayerTransform transform = GetTransform();

    if(_image == NULL ||
        IsVisible() == false || transform.alpha == 0x00 ||
        transform.stretchWidth == 0 || transform.stretchHeight == 0 ||
        _width == 0 || _height == 0)return;

#ifdef WII
    // Use the sprites texture
    _image->BindTexture(true);

    // Draw the GuiSprite Quad with transformations
    Mtx model, tmp;
    guMtxIdentity(model);
    guMtxRotDeg(tmp, 'z', transform.rotation/2);
    guMtxConcat(model, tmp, model);
    guMtxTransApply(model, model, transform.offsetX, transform.offsetY, 0.0f);
    guMtxConcat(model, tmp, model);
    GX_LoadPosMtxImm(model, GX_PNMTX0);

    // Now we apply zooming
    f32 refPixelX = _refPixelX * transform.stretchWidth,
        refPixelY = _refPixelY * transform.stretchHeight,
        refWidth = _refWidth * transform.stretchWidth,
        refHeight = _refHeight * transform.stretchHeight;

    // Returns the position of the frame
    if(_width == 0 || _image == NULL || !_image->IsInitialized() || _image->GetWidth() == 0)return;
    f32 frameX = (f32)_clip_x,
        frameY = (f32)_clip_y;
    // Calculates the texture position
    f32 txCoords[4] = {
        frameX/_image->GetWidth()+(FRAME_CORRECTION/_image->GetWidth()),
        frameY/_image->GetHeight()+(FRAME_CORRECTION/_image->GetHeight()),
        (frameX+_width)/_image->GetWidth()-(FRAME_CORRECTION/_image->GetWidth()),
        (frameY+_height)/_image->GetHeight()-(FRAME_CORRECTION/_image->GetHeight()),
    };

    // Normal texture
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position2f32(-refPixelX, -refPixelY);
        GX_Color4u8(0xff,0xff,0xff, transform.alpha);
        GX_TexCoord2f32(txCoords[0], txCoords[1]);
        GX_Position2f32(refWidth, -refPixelY);
        GX_Color4u8(0xff,0xff,0xff, transform.alpha);
        GX_TexCoord2f32(txCoords[2], txCoords[1]);
        GX_Position2f32(refWidth, refHeight);
        GX_Color4u8(0xff,0xff,0xff, transform.alpha);
        GX_TexCoord2f32(txCoords[2], txCoords[3]);
        GX_Position2f32(-refPixelX, refHeight);
        GX_Color4u8(0xff,0xff,0xff, transform.alpha);
        GX_TexCoord2f32(txCoords[0], txCoords[3]);
    GX_End();

#else
    spr->SetColor(((u32)transform.alpha << 24) + 0xffffff);
    spr->SetHotSpot(_refPixelX, _refPixelY);
    spr->SetTextureRect((f32)_clip_x, (f32)_clip_y, (f32)_width, (f32)_height);
    spr->RenderEx(transform.offsetX+1, transform.offsetY+20+1, (transform.rotation / 180.0f) * M_PI,
                  transform.stretchWidth, transform.stretchHeight);
#endif
}

