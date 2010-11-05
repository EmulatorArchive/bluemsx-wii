
#ifndef _GUI_SELECTION_LIST_H
#define _GUI_SELECTION_LIST_H

#include "GuiContainer.h"
#include "GuiElement.h"
#include "../WiiSprite/DrawableImage.h"
#include "../WiiSprite/Sprite.h"

typedef enum {
  SELRET_SELECTED,
  SELRET_CUSTOM,
  SELRET_KEY_HOME,
  SELRET_KEY_B,
  SELRET_KEY_PLUS,
} SELRET;

class GuiSelectionList : public GuiElement {
public:
    GuiSelectionList(GuiContainer *man, int rows);
    virtual ~GuiSelectionList();

    virtual bool ElmSetSelectedOnCollision(Sprite *sprite);
    virtual void ElmSetSelected(bool sel, int x, int y);
    virtual bool ElmGetRegion(int *px, int *py, int *pw, int *ph);
    virtual bool ElmHandleKey(GuiDialog *dlg, BTN key, bool pressed);

    void InitSelection(const char **items, int num, int select, int fontsz, int pitchy,
                       int posx, int posy, int xspace, int width, bool centr = false);
    void ClearTitleList(void);
    void SetSelectedItem(int fade = -1, int delay = -1);
    void SetNumberOfItems(int num);
    SELRET DoSelection(int *selection);
    bool IsShowing(void);
    int IsActive(void);
    int GetSelectedItem(void);
    void DoKeyUp(void);
    void DoKeyDown(void);

private:
    void CleanUp(void);

    int xpos;
    int ypos;
    int xsize;
    int xspacing;
    int ypitch;
    int fontsize;
    bool center;
    int selected, index;
    int num_items;
    int num_item_rows;
    int current_index;
    int upper_index;
    int lower_index;
    bool is_showing;
    bool is_active;
    const char **item_list;
    const char **visible_items;
    Sprite **titleTxtSprite;
    Sprite *sprCursor;
    Sprite *sprSelector;
    Sprite *sprArrowUp;
    Sprite *sprArrowDown;

    void RemoveTitleList(int fade = 0, int delay = 0);
};

#endif


