
#include "GuiLayer.h"

u32 GuiLayer::_highest_id = 0;
CMutex GuiLayer::_mutex;

GuiLayer::GuiLayer() :
    _rotation(0.0f), _rotoff(0.0f),
    _height(0), _width(0),
    _alpha(0xff), _alphaoff(0xff),
    _x(0), _y(0), _xoff(0), _yoff(0),
    _visible(true)
{
    _mutex.Lock();
    _id = ++_highest_id;
    _mutex.Unlock();
}

GuiLayer::~GuiLayer(){
}

u32 GuiLayer::GetID()
{
    return _id;
}
u32 GuiLayer::GetHeight() const{
    return _height;
}
u32 GuiLayer::GetWidth() const{
    return _width;
}

f32 GuiLayer::GetX() const{
    return _x;
}
f32 GuiLayer::GetXabs() const{
    return _x+_xoff;
}
f32 GuiLayer::GetY() const{
    return _y;
}
f32 GuiLayer::GetYabs() const{
    return _y+_yoff;
}

bool GuiLayer::IsVisible() const{
    return _visible;
}
void GuiLayer::SetVisible(bool visible){
    _visible = visible;
}

void GuiLayer::SetTransparency(u8 alpha){
    _alpha = alpha;
}
u8 GuiLayer::GetTransparency() const{
    return _alpha;
}
u8 GuiLayer::GetTransparencyAbs() const{
    return (u8)(((u16)_alpha * _alphaoff) / 255);
}

void GuiLayer::SetRotation(f32 rotation){
    _rotation = rotation;
}
f32 GuiLayer::GetRotation() const{
    return _rotation;
}
f32 GuiLayer::GetRotationAbs() const{
    f32 rotation = _rotation + _rotoff;
    if( rotation > 360.0f ) rotation -= 360.0f;
    return rotation;
}

void GuiLayer::SetPosition(f32 x, f32 y){
    _x = x;
    _y = y;
}
void GuiLayer::SetPosition(s32 x, s32 y){
    _x = (f32)x;
    _y = (f32)y;
}
void GuiLayer::Move(f32 deltaX, f32 deltaY){
    _x += deltaX;
    _y += deltaY;
}
void GuiLayer::SetX(f32 x){
    _x = x;
}
void GuiLayer::SetX(s32 x){
    SetX((f32)x);
}
void GuiLayer::SetX(u32 x){
    SetX((f32)x);
}
void GuiLayer::SetY(f32 y){
    _y = y;
}
void GuiLayer::SetY(s32 y){
    SetY((f32)y);
}
void GuiLayer::SetY(u32 y){
    SetY((f32)y);
}

void GuiLayer::SetTransform(f32 offsetX, f32 offsetY, f32 rot, u8 alpha){
    _xoff = offsetX;
    _yoff = offsetY;
    _rotoff = rot;
    _alphaoff = alpha;
}

bool GuiLayer::IsBusy(void){
    return false;
}

void GuiLayer::Draw(void){
    // Nothing at all :)
}

