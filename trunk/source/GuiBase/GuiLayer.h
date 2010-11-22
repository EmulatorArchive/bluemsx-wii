/*
 * GuiBase - GuiLayer
 */

#ifndef GUIBASE_LAYER
#define GUIBASE_LAYER

#include <stdlib.h>
#include <gccore.h>

#include "Mutex.h"

#ifndef GUI_PI
#define GUI_2PI     6.28318530717958647692f
#define GUI_PI      3.14159265358979323846f
#define GUI_PI_2    1.57079632679489661923f
#define GUI_PI_4    0.785398163397448309616f
#define GUI_1_PI    0.318309886183790671538f
#define GUI_2_PI    0.636619772367581343076f
#endif

typedef struct {
  bool valid;
  f32 offsetX;
  f32 offsetY;
  f32 stretchWidth;
  f32 stretchHeight;
  f32 rotation;
  u8  alpha;
} LayerTransform;

//!Drawable objects should derive from this class. Layers can be managed by LayerManagers.
class GuiLayer{
public:
    //Constructor.
    GuiLayer();
    //Destructor.
    virtual ~GuiLayer();

    u32 GetID();

    //!Gets the height of the layer.
    //!\return the current height of this layer.
    u32 GetHeight() const;
    //!Gets the width of the layer.
    //!\return the current width of this layer.
    u32 GetWidth() const;

    //!Gets the X position on the viewport.
    //!\return The X position.
    f32 GetX() const;
    //!Gets the Y position on the viewport.
    //!\return The Y position.
    f32 GetY() const;

    //!Checks if the layer is visible. Is visible by default.
    //!\return true if it is visible, false if not.
    bool IsVisible() const;
    //!Changes the visibility of the layer.
    //!\param visible true for show, false for hide.
    void SetVisible(bool visible);

    //!Sets the transparency of the tiledlayer.
    //!\param alpha Sets the transparency. Has a range from 0x00 (invisible) to 0xFF (fully visible)
    void SetTransparency(u8 alpha);
    //!Gets the transparency of the tiledlayer.
    //!\return The current transparency of the tiledlayer. Has a range from 0x00 (invisible) to 0xFF (fully visible)
    u8 GetTransparency() const;

    //!Sets the zooming of the sprite. It resets any defined stretch values.
    //!\param zoom The new zoom of the sprite. 1 is normal size, cannot be smaller than 0.
    void SetZoom(f32 zoom);
    //!Gets the zooming of the sprite. If StretchWidth is not the same as StretchHeight, it returns 0.
    //!\return The current zoom of the sprite. 1 is normal size.
    f32 GetZoom() const;
    //!Sets the width stretch of the sprite.
    //!\param stretchWidth Stretches the width of the sprite by this value. 1 is normal size, cannot be smaller than 0.
    void SetStretchWidth(f32 stretchWidth);
    //!Sets the height stretch of the sprite.
    //!\param stretchHeight Stretches the height of the sprite by this value. 1 is normal size, cannot be smaller than 0.
    void SetStretchHeight(f32 stretchHeight);
    //!Gets the width stretch of the sprite. Is equal to zoom value if zoom was set.
    //!\return The current width stretch of the sprite. 1 is normal size.
    f32 GetStretchWidth() const;
    //!Gets the height stretch of the sprite. Is equal to zoom vallue if zoom was set.
    //!\return The current height stretch of the sprite. 1 is normal size.
    f32 GetStretchHeight() const;

    //!Sets a reference pixel. The sprite rotates and zooms around this specified point. When
    //!a new image gets initialized, the refpixel is moved to the center and the positioning
    //!is REFPIXEL_POS_TOPLEFT.
    //!\param x The x position of the reference pixel.
    //!\param y The y position of the reference pixel.
    void SetRefPixelPosition(f32 x, f32 y);
    //!Allows to set the x coordinate of the reference pixel as a standalone.
    //!\param x The x position of the reference pixel.
    void SetRefPixelX(f32 x);
    //!Allows to set the y coordinate of the reference pixel as a standalone.
    //!\param y The y position of the reference pixel.
    void SetRefPixelY(f32 y);
    //!Gets the current reference pixel x coordinate.
    //!\return The x position of the reference pixel.
    f32 GetRefPixelX() const;
    //!Gets the current reference pixel y coordinate.
    //!\return The y position of the reference pixel.
    f32 GetRefPixelY() const;

    //!Sets the rotation angle of the quad.
    //!\param rotation The new angle of the quad. It is measured in degrees/2, so if 90 degrees is wanted, 45 degrees should be the passed parameter.
    void SetRotation(f32 rotation);
    //!Gets the rotation angle of the quad.
    //!\return The current angle of the quad.
    f32 GetRotation() const;

    //!Changes the absolute position of the layer on the viewport.
    //!\param x The new X position.
    //!\param y The new Y position.
    void SetPosition(f32 x, f32 y);
    void SetPosition(s32 x, s32 y);
    //!Moves the layer some units.
    //!\param deltaX Moves the layer a specific amount of units to the left/right.
    //!\param deltaY Moves the layer a specific amount of units up/down.
    void Move(f32 deltaX, f32 deltaY);
    //!Sets the X position on the viewport.
    //!\param x The new X position of this layer.
    void SetX(f32 x);
    //!\overload
    //!\param x The new X position of this layer.
    void SetX(s32 x);
    //!\overload
    //!\param x The new X position of this layer.
    void SetX(u32 x);
    //!Sets the Y position on the viewport.
    //!\param y The new Y position of this layer.
    void SetY(f32 y);
    //!\overload
    //!\param y The new Y position of this layer.
    void SetY(s32 y);
    //!\overload
    //!\param y The new Y position of this layer.
    void SetY(u32 y);

    //!Overloadable method. Used by the parent to see if effects are in progress.
    virtual bool IsBusy(void);

    //!Overloadable method. Here should drawing code be.
    //!\param offsetX Additional offset for the X position where to draw the layer.
    //!\param offsetY Additional offset for the Y position where to draw the layer.
    virtual void ResetTransform(LayerTransform transform);
    virtual void DoTransform(LayerTransform transform);
    virtual LayerTransform GetTransform(void);
    virtual void Draw(void);
protected:
    // Transformation
    LayerTransform _transform;
    // Positioning
    u32 _height, _width;
    f32 _x, _y;
    // Transparency
    u8 _alpha, _alphaoff;
    // Zoom + Rotation
    f32 _stretchWidth, _stretchHeight;
    f32 _refPixelX, _refPixelY, _refWidth, _refHeight;
    f32 _rotation;
private:
    static CMutex _mutex;
    static u32 _highest_id;
    u32 _id;
    bool _visible;
};

#endif
