
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

class GuiBackground;
class GuiButton;
class GuiFrame;
class GuiSelectionList;

class GuiGameSelect : public GuiDialog
{
public:
    GuiGameSelect(GuiContainer *cntr, GuiBackground *bgr);
    virtual ~GuiGameSelect();

    // GuiDialog interface
    virtual void OnKey(BTN key, bool pressed);
    virtual void OnUpdateScreen(void);

    bool Load(const char *dir, const char *filename, GameElement *select = NULL);
    void Show(bool restart);
    void Hide(bool restart);
    GameElement *DoModal(void);
private:
    bool is_showing;
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

    GuiFrame *grWinList;
    GuiFrame *grWinTitle;
    GuiFrame *grWinPlay;
    GuiFrame *grWinControls;

    GuiButton *grButtonAdd;
    GuiButton *grButtonDel;
    GuiButton *grButtonUp;
    GuiButton *grButtonDown;
    GuiButton *grButtonSettings;
    GuiButton *grButtonDelScr1;
    GuiButton *grButtonDelScr2;

    void SetSelectedGame(int index, int selected, bool restart);
    void UpdateList(void);
};

#endif

