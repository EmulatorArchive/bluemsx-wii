
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "GuiSelectionList.h"
#include "GameElement.h"
#include "GameList.h"

#define NUM_LIST_ITEMS   12

class GuiGameSelect : public GuiSelectionList {
public:
    GuiGameSelect(GuiManager *man);
    virtual ~GuiGameSelect();

    void OnSetSelected(int index, int selected);
    bool Load(const char *dir, const char *filename);
    GameElement *DoModal(GameElement *select = NULL);
private:
    GameList games;
    int num_games;
    const char **title_list;
    Sprite *sprScreenShot[2];
	float screenshotWidth;
	float screenshotHeigth;
	int screenshotYpos1;
	int screenshotYpos2;
	int fade_time;
	int fade_delay;
	int last_index;
	int last_selected;

    void SetScreenShotImage(int index, Image *img);
};

#endif

