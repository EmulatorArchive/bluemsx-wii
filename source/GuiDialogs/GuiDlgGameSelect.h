/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../Gui/DirectoryHelper.h"
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
class GuiDlgGameSelect;

class GuiElmGameSelectControl : public GuiElement
{
public:
    GuiElmGameSelectControl(GuiElement *_parent, const char *name);
    virtual ~GuiElmGameSelectControl();

    // GuiElement interface
    virtual bool OnKey(GuiDialog *dlg, BTN key, bool pressed);

private:
    GuiDlgGameSelect *parent;
    GuiElmFrame *grWinControlsEdit;
    GuiElmButton *grButtonAdd;
    GuiElmButton *grButtonDel;
    GuiElmButton *grButtonUp;
    GuiElmButton *grButtonDown;
    GuiElmButton *grButtonSettings;
};

class GuiDlgGameSelect : public GuiDialog
{
    friend class GuiElmGameSelectControl;

public:
    GuiDlgGameSelect(GuiContainer *parent, const char *name, GuiElmBackground *bgr);
    virtual ~GuiDlgGameSelect();

    // GuiElement interface
    virtual bool OnKey(GuiDialog *dlg, BTN key, bool pressed);
    // GuiDialog interface
    virtual void OnUpdateScreen(void);

    bool Load(const char *dir, const char *filename, GameElement *select = NULL);
    GameElement *DoModal(void);

protected:
    DirectoryHelper directory;
    bool update_screenshots;
    GuiElmBackground *background;
    GuiElmSelectionList *list;
    GameList games;
    unsigned games_crc;
    char *games_filename;
    int num_games;
    const char **title_list;
    GuiSprite *sprScreenShotNormal[2];
    GuiSprite *sprScreenShotEdit[2];
    float screenshotWidth;
    float screenshotHeigth;
    int screenshotYpos;
    int last_index;
    int last_selected;
    GLEDITSEL selected_button;
    bool editMode;

    GuiEffectFade effectDefault;
    GuiEffectFade effectRestart;
    GuiEffectFade effectScreenshot;

    GuiElmFrame *grWinList;

    GuiContainer *containerTitleNormal;
    GuiContainer *containerTitleEdit;
    GuiContainer *containerPlayNormal;
    GuiContainer *containerPlayEdit;
    GuiElmGameSelectControl elmControl;

    GuiElmFrame *grWinTitleNormal;
    GuiElmFrame *grWinPlayNormal;

    GuiElmFrame *grWinTitleEdit;
    GuiElmFrame *grWinPlayEdit;

    GuiElmButton *grButtonDelScr1;
    GuiElmButton *grButtonDelScr2;

    void ShowElements(void);
    void SetSelectedGame(int selected);
    void UpdateList(void);
};

#endif

