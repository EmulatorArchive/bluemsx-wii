
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include "GuiManager.h"
#include "DrawableImage.h"

class GuiMenu {
public:
    GuiMenu(GuiManager *man, int rows);
    virtual ~GuiMenu();

    int DoModal(const char **items, int num, int width);
private:
    GuiManager *manager;
    int num_items;
    int num_item_rows;
    int current_index;
    int upper_index;
    int lower_index;
    const char **item_list;
    const char **visible_items;
    Sprite *titleTxtSprite;
    Sprite sprSelector;
    Sprite sprArrowUp;
    Sprite sprArrowDown;
    DrawableImage *titleTxtImg;
    DrawableImage **titleTxtImgPtr;

    void InitTitleList(TextRender *fontArial, int fontsize,
                       int x, int y, int sx, int sy, int pitch);
    void RemoveTitleList(void);
    void ClearTitleList(void);
    void SetListIndex(int index);
    void SetSelected(int selected);
};

#endif


