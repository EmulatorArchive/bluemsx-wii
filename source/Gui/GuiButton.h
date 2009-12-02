#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "GuiManager.h"

class DrawableImage;

typedef enum {
  BTE_SELECTOR,
  BTE_HIGHLIGHT,
  BTE_HIGHLIGHTTEXT,
} BTE;

class GuiButton {
public:
    GuiButton(GuiManager *man);
    virtual ~GuiButton();

    void ShowImageSelectorButton(Image *image, int x=0, int y=0, int f_show=0, int f_sel=0);
    void ShowImageHighlightButton(Image *image, int x=0, int y=0, int f_show=0, int f_sel=0);
    void ShowImageTextHighlightButton(Image *image, const char *txt, int x=0, int y=0, int f_show=0, int f_sel=0);
    void SetSelected(bool sel);
    void Remove(int fade=0, int delay=0);
    bool CollidesWith(Sprite *cursor);
    bool Update(Sprite *cursor);

private:
    BTE effect;
    bool selected;
    int posx, posy, fade_sel;
    GuiManager *manager;
    DrawableImage *imgText;
    Sprite *sprImage;
    Sprite *sprText;
    Sprite *sprSelector;
};

#endif
