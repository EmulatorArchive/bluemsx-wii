#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "GuiElement.h"

typedef enum {
  BTE_SELECTOR,
  BTE_HIGHLIGHT,
  BTE_HIGHLIGHTTEXT,
} BTE;

class GuiButton : public GuiElement {
public:
    GuiButton();
    virtual ~GuiButton();

    virtual bool ElmSetSelectedOnCollision(Sprite *sprite);
    virtual void ElmSetSelected(bool sel, int x, int y);
    virtual bool ElmGetRegion(int *px, int *py, int *pw, int *ph);

    void CreateImageSelectorButton(Image *image, int f_sel=0);
    void CreateImageHighlightButton(Image *image, int f_sel=0);
    void CreateImageTextHighlightButton(Image *image, const char *txt, int f_sel=0);

private:
    void CleanUp(void);

    BTE type;
    bool selected;
    bool shown;
    int fade_sel;
    Sprite *sprImage;
    Sprite *sprText;
    Sprite *sprSelector;
};

#endif
