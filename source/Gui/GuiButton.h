#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "GuiManager.h"
#include "GuiElement.h"

class DrawableImage;

typedef enum {
  BTE_SELECTOR,
  BTE_HIGHLIGHT,
  BTE_HIGHLIGHTTEXT,
} BTE;

class GuiButton : public GuiElement {
public:
    GuiButton(GuiManager *man);
    virtual ~GuiButton();

    virtual void ElmAddLayers(GuiManager *manager, int index, bool fix, int fade, int delay);
    virtual void ElmRemoveLayers(GuiManager *manager, bool del, int fade, int delay);
    virtual Layer* ElmGetTopLayer(void);
    virtual Layer* ElmGetBottomLayer(void);

    virtual bool ElmSetSelectedOnCollision(GuiRunner *runner, Sprite *sprite);
    virtual void ElmSetSelected(GuiRunner *runner, bool sel, int x, int y);
    virtual bool ElmGetRegion(GuiRunner *runner, int *px, int *py, int *pw, int *ph);

    void CreateImageSelectorButton(Image *image, int x=0, int y=0, int f_sel=0);
    void CreateImageHighlightButton(Image *image, int x=0, int y=0, int f_sel=0);
    void CreateImageTextHighlightButton(Image *image, const char *txt, int x=0, int y=0, int f_sel=0);

private:
    BTE effect;
    bool selected;
    bool shown;
    int posx, posy, fade_sel;
    int width, height;
    GuiManager *manager;
    DrawableImage *imgText;
    Sprite *sprImage;
    Sprite *sprText;
    Sprite *sprSelector;
};

#endif
