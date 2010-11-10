#ifndef _GUI_ELEMENT_H
#define _GUI_ELEMENT_H

#include "InputDevices.h"
#include "GuiContainer.h"

class GuiDialog;
class GuiEffect;
class GuiLayer;
class GuiSprite;

class GuiElement : public GuiContainer {
public:
    GuiElement(GuiContainer *cntr = NULL);
    virtual ~GuiElement();

    virtual bool ElmSetSelectedOnCollision(GuiSprite *sprite);
    virtual void ElmSetSelected(bool sel, int x, int y);
    virtual bool ElmGetRegion(int *px, int *py, int *pw, int *ph);
    virtual bool ElmHandleKey(GuiDialog *dlg, BTN key, bool pressed);
};

#endif
