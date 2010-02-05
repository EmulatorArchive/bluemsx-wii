
#include "GuiElement.h"

GuiElement::GuiElement()
{
}

GuiElement::~GuiElement()
{
}

bool GuiElement::ElmSetSelectedOnCollision(GuiRunner *runner, Sprite *sprite)
{
    return false;
}

void GuiElement::ElmSetSelected(GuiRunner *runner, bool sel, int x, int y)
{
}

bool GuiElement::ElmGetRegion(GuiRunner *runner, int *px, int *py, int *pw, int *ph)
{
    return false;
}

bool GuiElement::ElmHandleKey(GuiRunner *runner, KEY key, bool pressed)
{
    return false;
}

