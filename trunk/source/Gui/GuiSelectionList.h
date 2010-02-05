
#ifndef _GUI_SELECTION_LIST_H
#define _GUI_SELECTION_LIST_H

#include "GuiManager.h"
#include "GuiElement.h"
#include "DrawableImage.h"
#include "Sprite.h"

typedef enum {
  SELRET_SELECTED,
  SELRET_CUSTOM,
  SELRET_KEY_HOME,
  SELRET_KEY_B,
  SELRET_KEY_PLUS,
} SELRET;

class GuiSelectionList : public GuiElement {
public:
    GuiSelectionList(GuiManager *man, int rows);
    virtual ~GuiSelectionList();

    virtual void ElmAddLayers(GuiManager *manager, int index, bool fix, int fade, int delay);
    virtual void ElmRemoveLayers(GuiManager *manager, bool del, int fade, int delay);
    virtual wsp::Layer* ElmGetTopLayer(void);
    virtual wsp::Layer* ElmGetBottomLayer(void);

    virtual bool ElmSetSelectedOnCollision(GuiRunner *runner, Sprite *sprite);
    virtual void ElmSetSelected(GuiRunner *runner, bool sel, int x, int y);
    virtual bool ElmGetRegion(GuiRunner *runner, int *px, int *py, int *pw, int *ph);
    virtual bool ElmHandleKey(GuiRunner *runner, KEY key, bool pressed);

    void InitSelection(const char **items, int num, int select, int fontsz, int pitchy,
                       int posx, int posy, int xspace, int width, bool centr = false);
    void ClearTitleList(void);
    void SetSelected(int fade = -1, int delay = -1);
    void SetNumberOfItems(int num);
    SELRET DoSelection(int *selection);
    bool IsShowing(void);
    int GetSelected(void);
    void DoKeyUp(void);
    void DoKeyDown(void);

protected:
    GuiManager *manager;
private:
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
    const char **item_list;
    const char **visible_items;
    Sprite **titleTxtSprite;
    Sprite *sprCursor;
    Sprite *sprSelector;
    Sprite *sprArrowUp;
    Sprite *sprArrowDown;
    DrawableImage *titleTxtImg;
    DrawableImage **titleTxtImgPtr;

    void InitTitleList(TextRender *fontArial, int x, int y, int width, int ypitch, int fade = 0);
    void RemoveTitleList(int fade = 0, int delay = 0);
};

#endif


