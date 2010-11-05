#include <assert.h>
#include <math.h>
#ifndef WII
#include <hge/hgeSprite.h>
#endif

#include "sprite.h"

#include "Image.h"
#include "DrawableImage.h"
#include "TiledLayer.h"

#define FRAME_CORRECTION 0.375f //!< Displays frames a lot nicer, still needs a complete fix.


Sprite::Sprite(Image* image, int x, int y) : Layer(),
    _stretchWidth(1.0f), _stretchHeight(1.0f), _image(NULL), _draw_image(NULL), _image_owner(false),
    _trans(TRANS_NONE), _colRect(NULL), _frame(0), _frameRawCount(0), _frameSeq(NULL), _frameSeqLength(0),
    _frameSeqPos(0), _refPixelX(0), _refPixelY(0), _refWidth(0), _refHeight(0), _positioning(REFPIXEL_POS_TOPLEFT)
{
#ifndef WII
    spr = NULL;
#endif
    for(u8 i = 0; i < 4; i++)
        _txCoords[i] = 0;
    _colRect = new Rect();
    if( image != NULL) {
        SetImage(image);
    }
    SetPosition((f32)x,(f32)y);
}
Sprite::~Sprite(){
    if(_colRect)
        delete _colRect; _colRect = NULL;
    if(_frameSeq)
        delete[] _frameSeq; _frameSeq = NULL;
    CleanUp();
#ifndef WII
    if(spr)
        delete spr;
#endif
}

void Sprite::CleanUp(void)
{
    if( _image_owner ) {
        if( _draw_image ) {
            delete _draw_image;
            _draw_image = NULL;
            _image = NULL;
        }else if( _image ) {
            delete _image;
            _image = NULL;
        }
        _image_owner = false;
    }
}

void Sprite::SetImage(Image* image, DrawableImage* drawimage, u32 frameWidth, u32 frameHeight)
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
    spr = new hgeSprite(image->GetTEX(), 0, 0, image->GetWidth(), image->GetHeight());
    spr->SetBlendMode(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE);
#endif
    // Free previous image if needed
    CleanUp();
    // If Image has the same size and if frameWidth and frameHeight are not modified
    if(_image != NULL && _image->GetHeight() == image->GetHeight() &&
        _image->GetWidth() == image->GetWidth() &&
        (frameWidth == 0 || frameWidth == _width) &&
        (frameHeight == 0 || frameHeight == _height))
        {
        // Just assign our image and it should be fine then
        _image = image;
        _draw_image = drawimage;
        return;
    }

    if(frameWidth == 0 && frameHeight == 0){
        frameWidth = image->GetWidth();
        frameHeight = image->GetHeight();
    }
    // If presented with a completely different image
    // Check if framesizes are multipliers of width and height
    if(image->GetWidth() % frameWidth != 0 || image->GetHeight() % frameHeight != 0){
        frameWidth = image->GetWidth();
        frameHeight = image->GetHeight();
    }
    _width = frameWidth; _height = frameHeight;

    // Set the new collision data
    _colRect->x = 0; _colRect->y = 0;
    _colRect->width = (f32)_width; _colRect->height = (f32)_height;

    // Now set framedata
    _frame = 0; _frameRawCount = (image->GetWidth()/_width)*(image->GetHeight()/_height);
    // Erase previous sequence
    if(_frameSeq)
        delete[] _frameSeq; _frameSeq = NULL;
    // Create a new sequence with startdata
    _frameSeqLength = _frameRawCount;
    _frameSeq = new u32[_frameSeqLength];
    _frameSeqPos = 0;
    for(u32 i = 0; i < _frameSeqLength; i++)_frameSeq[i] = i;

    // Refpixel setting. This positions the refpixel at the center.
    _refPixelX = (f32)_width/2; _refPixelY = (f32)_height/2;
    _refWidth = _refPixelX; _refHeight = _refPixelY;
    SetRefPixelPosition(0,0);
    // But draws based on topleft coordinate.
    _positioning = REFPIXEL_POS_TOPLEFT;

    _image = image;
    _draw_image = drawimage;
    _CalcFrame();
}
void Sprite::SetImage(Image* image, u32 frameWidth, u32 frameHeight)
{
    SetImage(new Image(image), NULL, frameWidth, frameHeight);
    _image_owner = true;
}
void Sprite::SetImage(DrawableImage* drawimage, u32 frameWidth, u32 frameHeight)
{
    SetImage(NULL, drawimage, frameWidth, frameHeight);
}

Image* Sprite::GetImage() const{
    return _image;
}

bool Sprite::LoadImage(const unsigned char *buf){
    Image *image = new Image;
    if(image->LoadImage(buf) == IMG_LOAD_ERROR_NONE) {
        _image = image;
        _image_owner = true;
        return true;
    }else{
        delete image;
        return false;
    }
}
bool Sprite::LoadImage(const char *file){
    Image *image = new Image;
    if(image->LoadImage(file) == IMG_LOAD_ERROR_NONE) {
        _image = image;
        _image_owner = true;
        return true;
    }else{
        delete image;
        return false;
    }
}

void Sprite::CreateTextImageVA(TextRender* font, int size, int minwidth, int yspace, bool center,
                             GXColor color, const char *fmt, va_list valist )
{
    // Free previous image if needed
    CleanUp();

    // Create new DrawableImage
    DrawableImage* drawimage = new DrawableImage();
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
    _image_owner = true;
}

void Sprite::CreateTextImage(TextRender* font, int size, int minwidth, int yspace, bool center,
                             GXColor color, const char *fmt, ... )
{
    va_list marker;
    va_start(marker,fmt);
    CreateTextImageVA(font, size, minwidth, yspace, center, color, fmt, marker);
    va_end(marker);
}

void Sprite::CreateDrawImage(int width, int height, int format)
{
    // Free previous image if needed
    CleanUp();

    // Create new DrawableImage
    DrawableImage* drawimage = new DrawableImage();
    drawimage->CreateImage(width, height, format);

    SetImage(drawimage, width, height);
    _image_owner = true;
}

void Sprite::FillSolidColor(u8 r, u8 g, u8 b)
{
    assert( _draw_image );
    _draw_image->FillSolidColor(r, g, b);
}

u8 *Sprite::GetTextureBuffer(void)
{
    assert( _draw_image );
    return _draw_image->GetTextureBuffer();
}

void Sprite::FlushBuffer(void)
{
    assert( _draw_image );
    _draw_image->FlushBuffer();
}

void Sprite::SetTransform(u8 transform){
    _trans = transform;
}
u8 Sprite::GetTransform() const{
    return _trans;
}

void Sprite::SetZoom(f32 zoom){
    if(zoom < 0)return;
    _stretchWidth = zoom;
    _stretchHeight = zoom;
}
f32 Sprite::GetZoom() const{
    if(_stretchWidth != _stretchHeight)return 0;
    return _stretchWidth;
}
void Sprite::SetStretchWidth(f32 stretchWidth){
    if(stretchWidth < 0)return;
    _stretchWidth = stretchWidth;
}
void Sprite::SetStretchHeight(f32 stretchHeight){
    if(stretchHeight < 0)return;
    _stretchHeight = stretchHeight;
}
f32 Sprite::GetStretchWidth() const{
    return _stretchWidth;
}
f32 Sprite::GetStretchHeight() const{
    return _stretchHeight;
}

void Sprite::SetRefPixelPosition(f32 x, f32 y){
    _refPixelX = x;
    _refWidth = (f32)_width-x;
    _refPixelY = y;
    _refHeight = (f32)_height-y;
}
void Sprite::SetRefPixelX(f32 x){
    _refPixelX = x;
    _refWidth = (f32)_width-x;
}
void Sprite::SetRefPixelY(f32 y){
    _refPixelY = y;
    _refHeight = (f32)_height-y;
}
f32 Sprite::GetRefPixelX() const{
    return _refPixelX;
}
f32 Sprite::GetRefPixelY() const{
    return _refPixelY;
}
void Sprite::SetRefPixelPositioning(REFPIXEL_POSITIONING positioning){
    _positioning = positioning;
}
REFPIXEL_POSITIONING Sprite::GetRefPixelPositioning() const{
    return _positioning;
}

void Sprite::DefineCollisionRectangle(f32 x, f32 y, f32 width, f32 height){
    _colRect->x = x;
    _colRect->y = y;
    _colRect->width = width;
    _colRect->height = height;
}
const Rect* Sprite::GetCollisionRectangle() const{
    return _colRect;
}

bool Sprite::CollidesWith(const Rect* rect, f32 x, f32 y) const{
    if(rect == NULL)return false;

    // Check if the rectangle is not in the other rectangle
    if(_colRect->y+GetYabs()+_colRect->height <= rect->y+y ||
        _colRect->y+GetYabs() >= rect->y+y+rect->height ||
        _colRect->x+GetXabs()+_colRect->width <= rect->x+x ||
        _colRect->x+GetXabs() >= rect->x+x+rect->width)
            return false;

    return true;
}
bool Sprite::CollidesWith(const Sprite* sprite, bool complete) const{
    if(sprite == NULL)return false;
    if(!complete){
        // Some simple collision detecting with the base collision rectangle.
        const Rect* collision = sprite->GetCollisionRectangle();
        return CollidesWith(collision, sprite->GetXabs(), sprite->GetYabs());
    }

    // Advanced rectangle collision detecting with zoom and rectangle.
    // Code used from http://www.ragestorm.net/sample?id=80 and modified
    // to get it working properly with libwiisprite.

    // Rotation is angle/2
    f32 angle1 = (f32)GetRotation()*M_PI/90,
        angle2 = (f32)sprite->GetRotation()*M_PI/90;

    Rect rect[4]; // Points which help calculate this stuff.

    f32 cosa, sina, // For sin/cos calculations
        temp, x, a, // Temp vars for various useage.
        deltaX, // For linear equations
        vertical1, vertical2; // Min/max vertical values

    // Init data
    rect[0].x = (f32)(sprite->GetXabs()+(f32)(sprite->GetWidth()/2));
    rect[0].y = (f32)(sprite->GetYabs()+(f32)(sprite->GetHeight()/2));
    rect[1].x = (f32)(GetXabs()+(f32)((GetWidth()/2)));
    rect[1].y = (f32)(GetYabs()+(f32)((GetHeight()/2)));
    if(_positioning == REFPIXEL_POS_PIXEL){
        rect[0].x -= sprite->GetRefPixelX();
        rect[0].y -= sprite->GetRefPixelY();
        rect[1].x -= GetRefPixelX();
        rect[1].x -= GetRefPixelY();
    }
    rect[0].width = (f32)(GetWidth()/2)*GetStretchWidth();
    rect[0].height = (f32)(GetHeight()/2)*GetStretchHeight();
    rect[1].width = (f32)(sprite->GetWidth()/2)*sprite->GetStretchWidth();
    rect[1].height = (f32)(sprite->GetHeight()/2)*sprite->GetStretchHeight();

    // Move the sprite by the other sprites values
    rect[0].x -= rect[1].x; rect[0].y -= rect[1].y;

    // Rotate the other sprite clockwise by its angle to make it axis-aligned
    cosa = cos(angle2),
    sina = sin(angle2);
    temp = rect[0].x; rect[0].x = temp*cosa + rect[0].y*sina; rect[0].y = -temp*sina + rect[0].y*cosa;

    // Calculate the points of the other sprite.
    rect[2].width = rect[2].x = rect[0].x;
    rect[2].height = rect[2].y = rect[0].y;
    rect[2].width -= rect[1].width; rect[2].height -= rect[1].height;
    rect[2].x += rect[1].width; rect[2].y += rect[1].height;

    cosa = cos(angle1-angle2);
    sina = sin(angle1-angle2);
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
    if(rect[3].width > rect[2].x || rect[3].width > -rect[2].width)return false;

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
    return !((vertical1 < rect[2].height && vertical2 < rect[2].height) ||
        (vertical1 > rect[2].y && vertical2 > rect[2].y));
}

bool Sprite::CollidesWith(const TiledLayer* tiledlayer) const{
    if(tiledlayer == NULL ||
        _colRect->x+GetXabs() < 0 || _colRect->y+GetYabs() < 0 ||
        tiledlayer->GetCellWidth() == 0 || tiledlayer->GetCellHeight() == 0)return false;

    // Get on which tiles the sprite is drawn
    Rect rect;
    rect.x = (s32)((_colRect->x+GetXabs())/tiledlayer->GetCellWidth());
    rect.y = (s32)((_colRect->y+GetYabs())/tiledlayer->GetCellHeight());
    rect.width = (u32)((_colRect->x+GetXabs()+_colRect->width)/tiledlayer->GetCellWidth());
    rect.height = (u32)((_colRect->y+GetYabs()+_colRect->height)/tiledlayer->GetCellHeight());

    for(s32 y = rect.y; y < rect.height+1; y++){
        for(s32 x = rect.x; x < rect.width+1; x++){
            // Since checks are done inside the tiledlayer, we do not need to check here
            s32 data = tiledlayer->GetCell(x, y);
            if(data < 0)data = tiledlayer->GetAnimatedTile(data);
            if(data != 0){
                return true;
            }
        }
    }
    return false;
}

u32 Sprite::GetFrame() const{
    return _frame;
}
u32 Sprite::GetFrameSequencePos() const{
    return _frameSeqPos;
}
u32 Sprite::GetFrameSequenceLength() const{
    return _frameSeqLength;
}
u32 Sprite::GetRawFrameCount() const{
    return _frameRawCount;
}
void Sprite::SetFrame(u32 sequenceIndex){
    if(sequenceIndex >= _frameSeqLength)return;

    _frameSeqPos = sequenceIndex;
    if(!_frameSeq){
        _frame = 0;
    }
    _frame = _frameSeq[_frameSeqPos];
    _CalcFrame();
}
void Sprite::NextFrame(){
    _frameSeqPos++;
    if(_frameSeqPos == _frameSeqLength)_frameSeqPos = 0;
    if(!_frameSeq){
        _frame = 0;
    }
    _frame = _frameSeq[_frameSeqPos];
    _CalcFrame();
}
void Sprite::PrevFrame(){
    if(_frameSeqPos == 0)_frameSeqPos = _frameSeqLength;
    _frameSeqPos--;
    if(!_frameSeq){
        _frame = 0;
    }
    _frame = _frameSeq[_frameSeqPos];
    _CalcFrame();
}
void Sprite::SetFrameSequence(u32* sequence, u32 length){
    if(sequence == NULL || length == 0)return;
    for(u32 i = 0; i < length; i++){
        if(sequence[i] >= _frameRawCount)return;
    }
    // Erase old frame sequence and copy the new one
    if(_frameSeq)
        delete[] _frameSeq; _frameSeq = NULL;
    _frameSeqLength = length;
    _frameSeq = new u32[length];
    for(u32 i = 0; i < length; i++){
        _frameSeq[i] = sequence[i];
    }
    _frameSeqPos = 0;
    _frame = _frameSeq[_frameSeqPos];
    _CalcFrame();
}

void Sprite::Draw(void)
{
    f32 absX = GetXabs();
    f32 absY = GetYabs();
    f32 rotation = GetRotationAbs();
    u8 alpha = GetTransparencyAbs();

    if(_image == NULL ||
        IsVisible() == false || alpha == 0x00 || _stretchWidth == 0 || _stretchHeight == 0 ||
        _width == 0 || _height == 0)return;

#ifdef WII
    // Turn on additive blending.
    if(_trans & TRANS_ADDITIVE_BLENDING)
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);

    // Use the sprites texture
    _image->BindTexture((_trans & TRANS_BILINEAR_OFF)?false:true);

    // Draw the Sprite Quad with transformations
    Mtx model, tmp;
    guMtxIdentity(model);
    guMtxRotDeg(tmp, 'z', rotation/2);
    guMtxConcat(model, tmp, model);
    if(_positioning == REFPIXEL_POS_PIXEL){
        guMtxTransApply(model, model, absX, absY, 0.0f);
    }else{ // REFPIXEL_POS_TOPLEFT
        guMtxTransApply(model, model, absX+_refPixelX, absY+_refPixelY, 0.0f);
    }
    guMtxConcat(model, tmp, model);
    GX_LoadPosMtxImm(model, GX_PNMTX0);

    // Now we apply zooming
    f32 refPixelX = _refPixelX * _stretchWidth,
        refPixelY = _refPixelY * _stretchHeight,
        refWidth = _refWidth * _stretchWidth,
        refHeight = _refHeight * _stretchHeight;

    // Mirrored texture
    if(_trans & TRANS_MIRROR){
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
            GX_Position2f32(-refPixelX, -refPixelY);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[2], _txCoords[1]);
            GX_Position2f32(refWidth, -refPixelY);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[0], _txCoords[1]);
            GX_Position2f32(refWidth, refHeight);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[0], _txCoords[3]);
            GX_Position2f32(-refPixelX, refHeight);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[2], _txCoords[3]);
        GX_End();

    // Normal texture
    }else{
        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
            GX_Position2f32(-refPixelX, -refPixelY);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[0], _txCoords[1]);
            GX_Position2f32(refWidth, -refPixelY);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[2], _txCoords[1]);
            GX_Position2f32(refWidth, refHeight);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[2], _txCoords[3]);
            GX_Position2f32(-refPixelX, refHeight);
            GX_Color4u8(0xff,0xff,0xff, alpha);
            GX_TexCoord2f32(_txCoords[0], _txCoords[3]);
        GX_End();
    }

    // Turn additive blending off again.
    if(_trans & TRANS_ADDITIVE_BLENDING)
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
#else
    spr->SetColor(((u32)alpha << 24) + 0xffffff);
    if( _positioning == REFPIXEL_POS_PIXEL ) {
        absX -= _refPixelX * _stretchWidth;
        absY -= _refPixelY * _stretchHeight;
    }else{
        absX *= _stretchWidth;
        absY *= _stretchHeight;
    }
    spr->SetHotSpot(_refPixelX, _refPixelY);
    absX += _refPixelX * _stretchWidth;
    absY += _refPixelY * _stretchHeight;
    spr->RenderEx(absX, absY+20, (rotation / 180.0f) * M_PI, _stretchWidth, _stretchHeight);
#endif
}

void Sprite::_CalcFrame(){
    // Returns the position of the frame
    if(_width == 0 || _image == NULL || !_image->IsInitialized() || _image->GetWidth() == 0)return;
    f32 frameX = (f32)((_frame%(_image->GetWidth()/_width))*_width),
        frameY = (f32)((_frame/(_image->GetWidth()/_width))*_height);
    // Calculates the texture position
    _txCoords[0] = frameX/_image->GetWidth()+(FRAME_CORRECTION/_image->GetWidth());
    _txCoords[1] = frameY/_image->GetHeight()+(FRAME_CORRECTION/_image->GetHeight());
    _txCoords[2] = (frameX+_width)/_image->GetWidth()-(FRAME_CORRECTION/_image->GetWidth());
    _txCoords[3] = (frameY+_height)/_image->GetHeight()-(FRAME_CORRECTION/_image->GetHeight());
}

