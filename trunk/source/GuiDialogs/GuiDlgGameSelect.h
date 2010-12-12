
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "../GuiBase/GuiDialog.h"
#include "../Gui/GameList.h"

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

class GuiElmBackground;
class GuiElmButton;
class GuiElmFrame;
class GuiElmSelectionList;

class GuiDlgGameSelect : public GuiDialog
{
public:
    GuiDlgGameSelect(GuiContainer *parent, const char *name, GuiElmBackground *bgr);
    virtual ~GuiDlgGameSelect();

    // GuiDialog interface
    virtual void OnKey(BTN key, bool pressed);
    virtual void OnUpdateScreen(void);

    bool Load(const char *dir, const char *filename, GameElement *select = NULL);
    void Show(bool restart);
    void Hide(bool restart);
    GameElement *DoModal(void);
private:
    bool is_showing;
    GuiElmBackground *background;
    GuiElmSelectionList *list;
    GameList games;
    unsigned games_crc;
    char *games_filename;
    int num_games;
    const char **title_list;
    GuiSprite *sprScreenShot[2];
    float screenshotWidth;
    float screenshotHeigth;
    int screenshotYpos1;
    int screenshotYpos2;
    int last_index;
    int last_selected;
    GLEDITSEL selected_button;
    bool editMode;

    GuiElmFrame *grWinList;
    GuiElmFrame *grWinTitle;
    GuiElmFrame *grWinPlay;
    GuiElmFrame *grWinControls;

    GuiElmButton *grButtonAdd;
    GuiElmButton *grButtonDel;
    GuiElmButton *grButtonUp;
    GuiElmButton *grButtonDown;
    GuiElmButton *grButtonSettings;
    GuiElmButton *grButtonDelScr1;
    GuiElmButton *grButtonDelScr2;

    void SetSelectedGame(int index, int selected, bool restart);
    void UpdateList(void);
};

#endif

