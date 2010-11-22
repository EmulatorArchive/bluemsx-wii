
#include "GuiElement.h"

class GuiEffect;

GuiElement::GuiElement(GuiContainer *cntr)
          : GuiContainer(cntr)
{
}

GuiElement::~GuiElement()
{
}

bool GuiElement::ElmSetSelectedOnCollision(GuiSprite *sprite)
{
    return false;
}

void GuiElement::ElmSetSelected(bool sel, GuiSprite *pointer, int x, int y)
{
}

bool GuiElement::ElmGetRegion(int *px, int *py, int *pw, int *ph)
{
    return false;
}

bool GuiElement::ElmHandleKey(GuiDialog *dlg, BTN key, bool pressed)
{
    return false;
}

