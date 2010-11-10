/*
 * GuiBase - GuiLayer
 */

#ifndef GUIBASE_LAYER
#define GUIBASE_LAYER

#include <stdlib.h>
#include <gccore.h>

#include "Mutex.h"

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
    f32 GetXabs() const;
    //!Gets the Y position on the viewport.
    //!\return The Y position.
    f32 GetY() const;
    f32 GetYabs() const;

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
    u8 GetTransparencyAbs() const;

    //!Sets the rotation angle of the quad.
    //!\param rotation The new angle of the quad. It is measured in degrees/2, so if 90 degrees is wanted, 45 degrees should be the passed parameter.
    void SetRotation(f32 rotation);
    //!Gets the rotation angle of the quad.
    //!\return The current angle of the quad.
    f32 GetRotation() const;
    f32 GetRotationAbs() const;

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

    void SetTransform(f32 offsetX = 0, f32 offsetY = 0, f32 rot = 0, u8 alpha = 255);

    //!Overloadable method. Used by the parent to see if effects are in progress.
    virtual bool IsBusy(void);

    //!Overloadable method. Here should drawing code be.
    //!\param offsetX Additional offset for the X position where to draw the layer.
    //!\param offsetY Additional offset for the Y position where to draw the layer.
    virtual void Draw(void);
protected:
    u32 _height, //!< Height of the GuiLayer.
      _width; //!< Width of the GuiLayer.
    u8 _alpha, _alphaoff;
    f32 _rotation, _rotoff;
    f32 _x, _y, _xoff, _yoff;
private:
    static CMutex _mutex;
    static u32 _highest_id;
    u32 _id;
    bool _visible;
};

#endif
