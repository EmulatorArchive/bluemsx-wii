
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "GuiDialog.h"
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

class GuiRunner;
class GuiBackground;
class GuiButton;
class GuiContainer;
class GuiSelectionList;

class GuiGameSelect : public GuiDialog
{
public:
    GuiGameSelect(GuiManager *man, GuiBackground *bgr);
    virtual ~GuiGameSelect();

    // GuiDialog interface
    virtual void OnKey(GuiRunner *runner, BTN key, bool pressed);
    virtual void OnUpdateScreen(GuiRunner *runner);

    bool Load(const char *dir, const char *filename);
    void Show(bool restart);
    void Hide(bool restart);
    GameElement *DoModal(GameElement *select = NULL);
private:
    GuiManager *manager;
    GuiRunner *runner;
    GuiBackground *background;
    GuiSelectionList *list;
    GameList games;
    unsigned games_crc;
    char *games_filename;
    int num_games;
    const char **title_list;
    Sprite *sprScreenShot[2];
    float screenshotWidth;
    float screenshotHeigth;
    int screenshotYpos1;
    int screenshotYpos2;
    int last_index;
    int last_selected;
    GLEDITSEL selected_button;
    bool editMode;

    GuiContainer *grWinList;
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

    void SetSelected(int index, int selected, bool restart);
    void SetScreenShotImage(int index, Image *img);
    void UpdateList(void);
};

#endif

