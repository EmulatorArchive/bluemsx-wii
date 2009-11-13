
#ifndef _GUI_CHECK_LIST_H
#define _GUI_CHECK_LIST_H

#include "GuiManager.h"
#include "DrawableImage.h"

class GuiCheckList {
public:
    GuiCheckList(GuiManager *man, int rows);
    virtual ~GuiCheckList();

    virtual void OnSetSelected(int index, int selected) {};
    void ShowSelection(const char **items, bool *enabled, int num, int select, int fontsz, int ypitch,
                       int posx, int posy, int xspace, int width, bool centr = false, int fad = 0);
    int DoSelection(void);
    void RemoveSelection(void);
    bool IsShowing(void);

protected:
    GuiManager *manager;
private:
    int xsize;
    int xspacing;
    int fontsize;
    bool center;
    int fade;
    int selected, index;
    int current;
    int num_items;
    int num_item_rows;
    int current_index;
    int upper_index;
    int lower_index;
    bool is_showing;
    const char **item_list;
    const char **visible_items;
    bool *enabled_list;
    Sprite **titleTxtSprite;
    Sprite **CheckboxSprite;
    Sprite *sprCursor;
    Sprite *sprSelector;
    Sprite *sprArrowUp;
    Sprite *sprArrowDown;
    DrawableImage *titleTxtImg;
    DrawableImage **titleTxtImgPtr;

    void InitTitleList(TextRender *fontArial, int x, int y, int width, int ypitch);
    void RemoveTitleList(void);
    void ClearTitleList(void);
    void SetSelected(int index, int selected);
};

#endif


