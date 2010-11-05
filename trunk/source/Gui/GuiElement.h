#ifndef _GUI_ELEMENT_H
#define _GUI_ELEMENT_H

#include "../WiiSprite/InputDevices.h"
#include "GuiContainer.h"

class GuiDialog;
class GuiEffect;
class Layer;
class Sprite;

class GuiElement : public GuiContainer {
public:
    GuiElement(GuiContainer *cntr = NULL);
    virtual ~GuiElement();

    virtual bool ElmSetSelectedOnCollision(Sprite *sprite);
    virtual void ElmSetSelected(bool sel, int x, int y);
    virtual bool ElmGetRegion(int *px, int *py, int *pw, int *ph);
    virtual bool ElmHandleKey(GuiDialog *dlg, BTN key, bool pressed);
};

#endif
