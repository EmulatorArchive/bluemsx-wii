
#ifndef _GUI_SELECTION_LIST_H
#define _GUI_SELECTION_LIST_H

#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiElement.h"
#include "../GuiBase/GuiSprite.h"

typedef enum {
  SELRET_SELECTED,
  SELRET_CUSTOM,
  SELRET_KEY_HOME,
  SELRET_KEY_B,
  SELRET_KEY_PLUS,
} SELRET;

class GuiElmListLine : public GuiContainer {
public:
    GuiElmListLine(GuiContainer *parent, const char *name)
                 : GuiContainer(parent, name) {};
    virtual ~GuiElmListLine() {};

    virtual GuiElmListLine* Create(GuiContainer *parent) = 0;
    virtual void Initialize(void *item) = 0;
    virtual COLL CollidesWith(GuiSprite* spr, bool complete = false) = 0;
};

class GuiElmListLineDefault : public GuiElmListLine {
public:
    GuiElmListLineDefault(GuiContainer *parent, const char *name,
                          GXColor fontcol, int fontsz, bool cntr);
    virtual ~GuiElmListLineDefault();

    virtual GuiElmListLine* Create(GuiContainer *parent);
    virtual void Initialize(void *item);
    virtual COLL CollidesWith(GuiSprite* spr, bool complete);
private:

    const char *text;
    GuiSprite *sprite;
    GXColor fontcolor;
    int fontsize;
    bool center;
};

class GuiElmSelectionList : public GuiElement {
public:
    GuiElmSelectionList(GuiContainer *parent, const char *name, int rows);
    virtual ~GuiElmSelectionList();

    virtual bool ElmSetSelectedOnCollision(GuiSprite *sprite);
    virtual void ElmSetSelected(bool sel, GuiSprite *pointer, int x, int y);
    virtual bool ElmGetRegion(int *px, int *py, int *pw, int *ph);
    virtual bool ElmHandleKey(GuiDialog *dlg, BTN key, bool pressed);

    void InitSelection(GuiElmListLine *listln, void **items, int num, int select,
                       int pitchy, int posx, int posy, int xspace, int width);
    void ClearTitleList(void);
    void SetSelectedItem(int fade = -1, int delay = -1);
    void SetNumberOfItems(int num);
    bool IsShowing(void);
    int IsActive(void);
    int GetSelectedItem(void);
    void DoKeyUp(void);
    void DoKeyDown(void);

private:
    void CleanUp(void);

    GuiElmListLine *listline;
    int xpos;
    int ypos;
    int xsize;
    int xspacing;
    int ypitch;
    int selected, prev_selected;
    int index;
    int num_items;
    int num_item_rows;
    int current_index;
    int upper_index;
    int lower_index;
    bool is_showing;
    bool is_active;
    void **item_list;
    GuiElmListLine **visible_items;
    GuiSprite *sprCursor;
    GuiSprite *sprSelector;
    GuiSprite *sprArrowUp;
    GuiSprite *sprArrowDown;
};

#endif


