#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "../GuiBase/GuiElement.h"

typedef enum {
  BTE_SELECTOR,
  BTE_HIGHLIGHT,
  BTE_HIGHLIGHTTEXT,
} BTE;

class GuiImage;
class GuiRect;

class GuiElmButton : public GuiElement {
public:
    GuiElmButton(GuiElement *parent, const char *name);
    virtual ~GuiElmButton();

    virtual bool OnTestActiveArea(float x, float y);
    virtual void OnFocus(bool focus);

    void CreateImageSelectorButton(const char *name, int f_sel=0);
    void CreateImageHighlightButton(const char *name, int f_sel=0);
    void CreateImageTextHighlightButton(const char *name, const char *txt, int f_sel=0);

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
