
#ifndef _GUI_SELECTION_LIST_H
#define _GUI_SELECTION_LIST_H

#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiElement.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiSprite.h"

typedef enum {
  UPDATELIST_REBUILD,
  UPDATELIST_SCROLL_DOWN,
  UPDATELIST_SCROLL_UP
} UPDATELIST;

typedef enum {
  SELRET_SELECTED,
  SELRET_CUSTOM,
  SELRET_KEY_HOME,
  SELRET_KEY_B,
  SELRET_KEY_PLUS,
} SELRET;

class GuiElmListLine : public GuiElement {
public:
    GuiElmListLine(GuiElement *parent, const char *name) :
                   GuiElement(parent, name) {};
    virtual ~GuiElmListLine() {};

    virtual GuiElmListLine* Create(GuiElement *parent) = 0;
    virtual void Initialize(void *item) = 0;
    virtual void EnableEffect(bool enable) = 0;
};

class GuiElmListLineDefault : public GuiElmListLine {
public:
    GuiElmListLineDefault(GuiElement *parent, const char *name,
                          GXColor fontcol, int fontsz, float xspace, bool cntr);
    virtual ~GuiElmListLineDefault();

    virtual GuiElmListLine* Create(GuiElement *parent);
    virtual void Initialize(void *item);
    virtual void EnableEffect(bool enable);

    virtual bool OnTestActiveArea(float x, float y);
    virtual void OnActive(bool active);

private:
    const char *text;
    GuiSprite *sprite;
    GuiSprite *selector;
    GXColor fontcolor;
    int fontsize;
    float xspacing;
    bool m_bEnableEffect;
    bool center;
    bool selected;
};

class GuiElmSelectionList : public GuiElement {
public:
    GuiElmSelectionList(GuiElement *parent, const char *name, int rows);
    virtual ~GuiElmSelectionList();

    // GuiElement callbacks
    virtual void OnSelect(GuiElement *element);
    virtual bool OnKey(GuiDialog *dlg, BTN key, bool pressed);

    void InitSelection(GuiElmListLine *listln, void **items, int num, int select,
                       float pitchy, float posx, float posy, float width);
    void UpdateList(UPDATELIST update = UPDATELIST_REBUILD);
    void SetNumberOfItems(int num);
    bool IsShowing(void);
    int IsActive(void);
    int GetSelectedItem(void);

private:
    void CleanUp(void);

    GuiElmListLine *listline;
    float ypitch;
    float rowheight;
    int index;
    int num_items;
    int num_item_rows;
    int upper_index;
    int lower_index;
    int selected_item;
    bool is_showing;
    void **item_list;
    GuiElmListLine **visible_items;
    GuiSprite *sprCursor;
    GuiSprite *sprArrowUp;
    GuiSprite *sprArrowDown;
    GuiEffectFade effectDefault;
};

#endif


