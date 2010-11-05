#include "layer.h"

u32 Layer::_highest_id = 0;
CMutex Layer::_mutex;

Layer::Layer() :
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

Layer::~Layer(){
}

u32 Layer::GetID()
{
    return _id;
}
u32 Layer::GetHeight() const{
    return _height;
}
u32 Layer::GetWidth() const{
    return _width;
}

f32 Layer::GetX() const{
    return _x;
}
f32 Layer::GetXabs() const{
    return _x+_xoff;
}
f32 Layer::GetY() const{
    return _y;
}
f32 Layer::GetYabs() const{
    return _y+_yoff;
}

bool Layer::IsVisible() const{
    return _visible;
}
void Layer::SetVisible(bool visible){
    _visible = visible;
}

void Layer::SetTransparency(u8 alpha){
    _alpha = alpha;
}
u8 Layer::GetTransparency() const{
    return _alpha;
}
u8 Layer::GetTransparencyAbs() const{
    return (u8)(((u16)_alpha * _alphaoff) / 255);
}

void Layer::SetRotation(f32 rotation){
    _rotation = rotation;
}
f32 Layer::GetRotation() const{
    return _rotation;
}
f32 Layer::GetRotationAbs() const{
    f32 rotation = _rotation + _rotoff;
    if( rotation > 360.0f ) rotation -= 360.0f;
    return rotation;
}

void Layer::SetPosition(f32 x, f32 y){
    _x = x;
    _y = y;
}
void Layer::SetPosition(s32 x, s32 y){
    _x = (f32)x;
    _y = (f32)y;
}
void Layer::Move(f32 deltaX, f32 deltaY){
    _x += deltaX;
    _y += deltaY;
}
void Layer::SetX(f32 x){
    _x = x;
}
void Layer::SetX(s32 x){
    SetX((f32)x);
}
void Layer::SetX(u32 x){
    SetX((f32)x);
}
void Layer::SetY(f32 y){
    _y = y;
}
void Layer::SetY(s32 y){
    SetY((f32)y);
}
void Layer::SetY(u32 y){
    SetY((f32)y);
}

void Layer::SetTransform(f32 offsetX, f32 offsetY, f32 rot, u8 alpha){
    _xoff = offsetX;
    _yoff = offsetY;
    _rotoff = rot;
    _alphaoff = alpha;
}

bool Layer::IsBusy(void){
    return false;
}

void Layer::Draw(void){
    // Nothing at all :)
}

