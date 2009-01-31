
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "GuiManager.h"
#include "DrawableImage.h"
#include "GameElement.h"
#include "GameList.h"

#define NUM_LIST_ITEMS   12

class GuiGameSelect {
public:
    GuiGameSelect(GuiManager *man);
    virtual ~GuiGameSelect();

    GameElement *DoModal(const char *dir, const char *filename);
private:
    GuiManager *manager;
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

    void InitTitleList(TextRender *fontArial, int fontsize,
                       int x, int y, int sx, int sy, int pitch);
    void RemoveTitleList(void);
    void SetScreenShotImage(int index, Image *img);
    void SetSelected(int index, int selected);
};

#endif

