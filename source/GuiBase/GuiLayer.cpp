
#include "GuiLayer.h"
#include "GuiContainer.h"
#include <math.h>

u32 GuiLayer::_highest_id = 0;
CMutex GuiLayer::_mutex;

GuiLayer::GuiLayer(GuiContainer *parent, const char *name) :
    _parent(parent),
    _name(name),
    _rotation(0.0f),
    _height(0), _width(0),
    _alpha(0xff),
    _x(0), _y(0),
    _visible(true),
    _refPixelX(0), _refPixelY(0), _refWidth(0), _refHeight(0),
    _stretchWidth(1.0f), _stretchHeight(1.0f)
{
    if( _parent != NULL ) {
        _parent->RegisterForDelete(this);
    }
    _transform.valid = false;
    _transform.offsetX = 0.0f;
    _transform.offsetY = 0.0f;
    _transform.stretchWidth = 1.0f;
    _transform.stretchHeight = 1.0f;
    _transform.rotation = 0.0f;
    _transform.alpha = 255;

    _mutex.Lock();
    _id = ++_highest_id;
    _mutex.Unlock();
}

GuiLayer::~GuiLayer(){
}

GuiContainer* GuiLayer::GetParent() const
{
    return _parent;
}

const char* GuiLayer::GetName() const
{
    return _name;
}

u32 GuiLayer::GetID() const
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
f32 GuiLayer::GetY() const{
    return _y;
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

void GuiLayer::SetZoom(f32 zoom){
    if( zoom < 0 )return;
    _stretchWidth = zoom;
    _stretchHeight = zoom;
}
f32 GuiLayer::GetZoom() const{
    if(_stretchWidth != _stretchHeight)return 0;
    return _stretchWidth;
}
void GuiLayer::SetStretchWidth(f32 stretchWidth){
    if( stretchWidth < 0 ) return;
    _stretchWidth = stretchWidth;
}
void GuiLayer::SetStretchHeight(f32 stretchHeight){
    if( stretchHeight < 0 ) return;
    _stretchHeight = stretchHeight;
}
f32 GuiLayer::GetStretchWidth() const{
    return _stretchWidth;
}
f32 GuiLayer::GetStretchHeight() const{
    return _stretchHeight;
}

void GuiLayer::SetRefPixelPosition(f32 x, f32 y){
    _refPixelX = x;
    _refWidth = (f32)_width-x;
    _refPixelY = y;
    _refHeight = (f32)_height-y;
}
void GuiLayer::SetRefPixelX(f32 x){
    _refPixelX = x;
    _refWidth = (f32)_width-x;
}
void GuiLayer::SetRefPixelY(f32 y){
    _refPixelY = y;
    _refHeight = (f32)_height-y;
}
f32 GuiLayer::GetRefPixelX() const{
    return _refPixelX;
}
f32 GuiLayer::GetRefPixelY() const{
    return _refPixelY;
}

void GuiLayer::SetRotation(f32 rotation){
    _rotation = rotation;
}
f32 GuiLayer::GetRotation() const{
    return _rotation;
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

bool GuiLayer::IsBusy(void){
    return false;
}

void GuiLayer::ResetTransform(LayerTransform transform)
{
    _transform.offsetX = _x;
    _transform.offsetY = _y;
    _transform.stretchWidth = _stretchWidth;
    _transform.stretchHeight = _stretchHeight;
    _transform.rotation = _rotation;
    _transform.alpha = _alpha;
    _transform.valid = true;
    DoTransform(transform);
}

void GuiLayer::DoTransform(LayerTransform transform)
{
    _transform.offsetX += transform.offsetX;
    _transform.offsetY += transform.offsetY;
    _transform.stretchWidth *= transform.stretchWidth;
    _transform.stretchHeight *= transform.stretchHeight;
    _transform.rotation = fmod(_transform.rotation + transform.rotation, 360.0f);
    _transform.alpha = (u8)(((u16)_transform.alpha * transform.alpha) / 255);
}

LayerTransform GuiLayer::GetTransform(void)
{
    return _transform;
}

void GuiLayer::Draw(void){
    // Nothing at all :)
}

