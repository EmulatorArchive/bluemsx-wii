
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include <wiisprite.h>
#include "spritemanager.h"

#include "GameElement.h"
#include "GameList.h"

#define NUM_LIST_ITEMS   12

class GuiMenu {
public:
    GuiMenu();
    virtual ~GuiMenu();

    GameElement *DoModal(GameWindow *gwd, const char *dir, const char *filename);
private:
    u32 KBD_JoystickDirection(joystick_t *js, int threshold);

    // Game list
    GameList games;
    int num_games;
    int current_index;
    int upper_index;
    int lower_index;
    GameElement emptyGame;
    GameElement *gameInfo[NUM_LIST_ITEMS];
    char *gameList[NUM_LIST_ITEMS];
    Sprite titleTxtSprite[NUM_LIST_ITEMS];
    Sprite sprSelector;
    Sprite sprScreenShot[2];
    Sprite sprArrowUp;
    Sprite sprArrowDown;
    DrawableImage titleTxtImg[NUM_LIST_ITEMS];
    DrawableImage *titleTxtImgPtr[NUM_LIST_ITEMS];

    void InitTitleList(LayerManager *manager,
                       TextRender *fontArial, int fontsize,
                       int x, int y, int sx, int sy, int pitch);
    void SetScreenShotImage(int index, Image *img);
    void SetListIndex(int index);
    void SetSelected(int selected);
};

#endif

