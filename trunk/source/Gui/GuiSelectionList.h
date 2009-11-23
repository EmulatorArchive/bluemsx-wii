
#ifndef _GUI_SELECTION_LIST_H
#define _GUI_SELECTION_LIST_H

#include "GuiManager.h"
#include "DrawableImage.h"
#include "Sprite.h"

#define SELRET_KEY_HOME -1
#define SELRET_KEY_B    -2
#define SELRET_KEY_PLUS -3
#define SELRET_CUSTOM   -4

class GuiSelectionList {
public:
    GuiSelectionList(GuiManager *man, int rows);
    virtual ~GuiSelectionList();

    virtual void OnSetSelected(int index, int selected) {};
    virtual bool OnUpdateCursorPosition(Sprite *cursor) { return false; };
    void InitSelection(const char **items, int num, int select, int fontsz, int pitchy,
                       int posx, int posy, int xspace, int width, bool centr = false);
    void ShowSelection(int fade = 0, int delay = 0);
    int DoSelection(void);
    void RemoveSelection(int fade = 0, int delay = 0);
    bool IsShowing(void);

protected:
    GuiManager *manager;
private:
	int xpos;
	int ypos;
    int xsize;
    int xspacing;
	int ypitch;
    int fontsize;
    bool center;
    int selected, index;
    int current;
    int num_items;
    int num_item_rows;
    int current_index;
    int upper_index;
    int lower_index;
    bool is_showing;
    const char **item_list;
    const char **visible_items;
    Sprite **titleTxtSprite;
    Sprite *sprSelector;
    Sprite *sprArrowUp;
    Sprite *sprArrowDown;
    DrawableImage *titleTxtImg;
    DrawableImage **titleTxtImgPtr;

    void InitTitleList(TextRender *fontArial, int x, int y, int width, int ypitch, int fade = 0);
    void RemoveTitleList(int fade = 0, int delay = 0);
    void ClearTitleList(void);
    void SetSelected(int index, int selected, int fade = -1, int delay = -1);
};

#endif


