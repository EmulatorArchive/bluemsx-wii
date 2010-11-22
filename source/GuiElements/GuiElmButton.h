#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "../GuiBase/GuiElement.h"

typedef enum {
  BTE_SELECTOR,
  BTE_HIGHLIGHT,
  BTE_HIGHLIGHTTEXT,
} BTE;

class GuiImage;

class GuiElmButton : public GuiElement {
public:
    GuiElmButton();
    virtual ~GuiElmButton();

    virtual bool ElmSetSelectedOnCollision(GuiSprite *sprite);
    virtual void ElmSetSelected(bool sel, GuiSprite *pointer, int x, int y);
    virtual bool ElmGetRegion(int *px, int *py, int *pw, int *ph);

    void CreateImageSelectorButton(GuiImage *image, int f_sel=0);
    void CreateImageHighlightButton(GuiImage *image, int f_sel=0);
    void CreateImageTextHighlightButton(GuiImage *image, const char *txt, int f_sel=0);

private:
    void CleanUp(void);

    BTE type;
    bool selected;
    bool shown;
    int fade_sel;
    GuiSprite *sprImage;
    GuiSprite *sprText;
    GuiSprite *sprSelector;
};

#endif
