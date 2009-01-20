
#ifndef _GUI_HOME_MENU_H
#define _GUI_HOME_MENU_H

#include <wiisprite.h>
#include "spritemanager.h"

#define MAX_PATH        256
#define NUM_MENU_ITEMS  4

class GuiHomeMenu {
public:
    GuiHomeMenu(LayerManager *layman, void *sem);
    virtual ~GuiHomeMenu();

    int DoModal(void);
private:
    LayerManager *manager;
    void *video_semaphore;
    static const char *items[NUM_MENU_ITEMS];
    Sprite titleTxtSprite[NUM_MENU_ITEMS];
    Sprite sprSelector;
    DrawableImage titleTxtImg[NUM_MENU_ITEMS];
    DrawableImage *titleTxtImgPtr[NUM_MENU_ITEMS];

    void InitTitleList(TextRender *fontArial, int fontsize,
                       int x, int y, int sx, int sy, int pitch);
    void RemoveTitleList(void);
    void SetSelected(int selected);
};

#endif

