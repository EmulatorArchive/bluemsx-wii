/*
 * GuiBase - GuiQuad
 */

#ifndef GUIBASE_QUAD
#define GUIBASE_QUAD

#include <stdlib.h>
#include <gccore.h>

#include "GuiLayer.h"
#include "GuiSprite.h"

//!Capable of drawing a rectangle on screen.
class GuiQuad : public GuiLayer {
public:
    //!Constructor.
    GuiQuad();
    //!Destructor.
    virtual ~GuiQuad();

    //!Allows to apply data from a rectangle to this quad.
    //!\param rect A pointer to the rectangle from which the data should be applied to this quad.
    void SetRectangle(Rect* rect);
    //!Returns a rectangle with the data of the quad. X and Y aren't offsets, but
    //!actual coordinates of the quad.
    //!\return A rectangle with the current data.
    const Rect* GetRectangle();

    //!Sets the width of the quad.
    //!\param width The new width of the quad.
    void SetWidth(u32 width);
    //!Seets the height of the quad.
    //!\param height The new height of the quad.
    void SetHeight(u32 height);

    //!Sets the border width of the quad.
    //!\param width The new border width of the quad.
    void SetBorderWidth(u16 width);
    //!Gets the border width of the quad.
    //!\return The current border width of the quad.
    u16 GetBorderWidth() const;

    //!Turns the border of the quad on or off.
    //!\param border Set to true if the quad should have border, false if not.
    void SetBorder(bool border);
    //!Returns the current state of the broder.
    //!\return True if there is a border set, false if not.
    bool IsBorder() const;

    //!Sets the color of the quad. Initial color is black.
    //!\param fillColor A GXColor with the desired data.
    void SetFillColor(GXColor fillColor);
    //!Sets the color of the border of the quad. Initial color is black.
    //!\param borderColor A GXColor with the desired data.
    void SetBorderColor(GXColor borderColor);

    //!Draws the quad.
    void Draw(void);
protected:
private:
    Rect* _rect;
    bool _border;
    u16 _borderWidth;
    GXColor _fillC, _borderC;
};

#endif
