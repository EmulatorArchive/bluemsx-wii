#ifndef _GUI_ELEMENT_H
#define _GUI_ELEMENT_H

#include "kbdlib.h"

class GuiManager;
class GuiRunner;
namespace wsp{
class Layer;
class Sprite;
}

using namespace wsp;

class GuiElement {
public:
    GuiElement();
    virtual ~GuiElement();

    virtual void ElmAddLayers(GuiManager *manager, int index, bool fix, int fade, int delay)=0;
    virtual void ElmRemoveLayers(GuiManager *manager, bool del, int fade, int delay)=0;
    virtual wsp::Layer* ElmGetTopLayer(void)=0;
    virtual wsp::Layer* ElmGetBottomLayer(void)=0;

    virtual bool ElmSetSelectedOnCollision(GuiRunner *runner, wsp::Sprite *sprite);
    virtual void ElmSetSelected(GuiRunner *runner, bool sel, int x, int y);
    virtual bool ElmGetRegion(GuiRunner *runner, int *px, int *py, int *pw, int *ph);
    virtual bool ElmHandleKey(GuiRunner *runner, KEY key, bool pressed);
};

#endif
