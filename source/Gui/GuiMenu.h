
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include <wiisprite.h>
#include "spritemanager.h"

class GuiMenu {
public:
    GuiMenu(LayerManager *layman, mutex_t mut, int rows);
    virtual ~GuiMenu();

    int DoModal(const char **items, int num, int width);
private:
    LayerManager *manager;
    mutex_t video_mutex;
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


