
#include "GuiQuad.h"

#include <stdlib.h>
#include <gccore.h>

#include "GuiLayer.h"
#include "GuiSprite.h"

GuiQuad::GuiQuad(GuiContainer *parent, const char *name) : GuiLayer(parent, name),
    _rect(NULL), _border(false), _borderWidth(1)
{
    _rect = new Rect();
    _fillC.r = 0;
    _borderC.r = 0;
    _fillC.g = 0;
    _borderC.g = 0;
    _fillC.b = 0;
    _borderC.b = 0;
    _fillC.a = 255;
    _borderC.a = 255;
}

GuiQuad::~GuiQuad(){
    if(_rect)
        delete _rect; _rect = NULL;
}

void GuiQuad::SetRectangle(Rect* rect){
    if(rect == NULL)return;
    SetX(rect->x);
    SetY(rect->y);
    _width = (u32)rect->width;
    _height = (u32)rect->height;
}
const Rect* GuiQuad::GetRectangle(){
    // Now we assign our data to the rectangle and return it.
    _rect->x = GetX();
    _rect->y = GetY();
    _rect->width = (f32)_width;
    _rect->height = (f32)_height;
    return _rect;
}

void GuiQuad::SetWidth(u32 width){
    _width = width;
}
void GuiQuad::SetHeight(u32 height){
    _height = height;
}

void GuiQuad::SetBorderWidth(u16 width){
    _borderWidth = width;
}
u16 GuiQuad::GetBorderWidth() const{
    return _borderWidth;
}

void GuiQuad::SetBorder(bool border){
    _border = border;
}
bool GuiQuad::IsBorder() const{
    return _border;
}

void GuiQuad::SetFillColor(GXColor fillColor){
    _fillC = fillColor;
}

void GuiQuad::SetBorderColor(GXColor borderColor){
    _borderC = borderColor;
}

void GuiQuad::Draw(void)
{
    LayerTransform transform = GetTransform();

    // Do some checks if the quad should be drawn at all
    if(!_border && _fillC.a == 0)
        return;
    if(IsVisible() == false || _width == 0 || _height == 0 || (_fillC.a == 0 && _borderC.a == 0))
        return;
#ifdef WII
    f32 width = ((GetWidth())/2),
        height = ((GetHeight())/2);

    // Use all the position data one can get
    Mtx model, tmp;
    guMtxIdentity(model);
    guMtxRotDeg(tmp, 'z', transform.rotation/2);
    guMtxConcat(model, tmp, model);
    guMtxTransApply(model, model, transform.offsetX+width, transform.offsetY+height, 0.0f);
    guMtxConcat(model, tmp, model);
    GX_LoadPosMtxImm(model, GX_PNMTX0);

    // Turn off texturing
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

    // Draw a GuiQuad
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position2f32(-width, -height);
        GX_Color4u8(_fillC.r, _fillC.g, _fillC.b, _fillC.a);
        GX_Position2f32(width, -height);
        GX_Color4u8(_fillC.r, _fillC.g, _fillC.b, _fillC.a);
        GX_Position2f32(width,height);
        GX_Color4u8(_fillC.r, _fillC.g, _fillC.b, _fillC.a);
        GX_Position2f32(-width,height);
        GX_Color4u8(_fillC.r, _fillC.g, _fillC.b, _fillC.a);
    GX_End();

    // Draw its border
    if(_border){
        GX_SetLineWidth(_borderWidth,GX_TO_ONE);
        GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 5); // Do it five times to get every border
            GX_Position2f32(-width, -height);
            GX_Color4u8(_borderC.r, _borderC.g, _borderC.b, _borderC.a);
            GX_Position2f32(width, -height);
            GX_Color4u8(_borderC.r, _borderC.g, _borderC.b, _borderC.a);
            GX_Position2f32(width,height);
            GX_Color4u8(_borderC.r, _borderC.g, _borderC.b, _borderC.a);
            GX_Position2f32(-width,height);
            GX_Color4u8(_borderC.r, _borderC.g, _borderC.b, _borderC.a);
            GX_Position2f32(-width, -height);
            GX_Color4u8(_borderC.r, _borderC.g, _borderC.b, _borderC.a);
        GX_End();
        GX_SetLineWidth(_borderWidth,GX_TO_ZERO);
    }

    // Turn texturing back on
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
#endif
}

