
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "GuiBackground.h"
#include "GuiButton.h"
#include "GuiContainer.h"
#include "GuiSelectionList.h"
#include "GameElement.h"
#include "GameList.h"

#define NUM_LIST_ITEMS   12

typedef enum {
  GLEDITSEL_NONE,
  GLEDITSEL_ADD,
  GLEDITSEL_DEL,
  GLEDITSEL_UP,
  GLEDITSEL_DOWN,
  GLEDITSEL_SETTINGS,
  GLEDITSEL_SCRSHOT_1,
  GLEDITSEL_SCRSHOT_2
} GLEDITSEL;

class GuiGameSelect : public GuiSelectionList {
public:
    GuiGameSelect(GuiManager *man, GuiBackground *bgr);
    virtual ~GuiGameSelect();

    void OnSetSelected(int index, int selected);
	bool OnUpdateCursorPosition(Sprite *sprite);
    bool Load(const char *dir, const char *filename);
    GameElement *DoModal(GameElement *select = NULL);
private:
	GuiBackground *background;
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
	GLEDITSEL selected_button;

	GuiContainer *grWinTitle;
	GuiContainer *grWinPlay;
	GuiContainer *grWinControls;

	GuiButton *grButtonAdd;
	GuiButton *grButtonDel;
	GuiButton *grButtonUp;
	GuiButton *grButtonDown;
	GuiButton *grButtonSettings;
	GuiButton *grButtonDelScr1;
	GuiButton *grButtonDelScr2;

    void SetScreenShotImage(int index, Image *img);
	void UpdateList(void);
};

#endif

