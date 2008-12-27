
#ifndef _GUI_DIRSELECT_H
#define _GUI_DIRSELECT_H

#include <wiisprite.h>
#include "spritemanager.h"

#include "DirectoryList.h"

#define MAX_PATH        256
#define NUM_DIR_ITEMS   6

class GuiDirSelect {
public:
    GuiDirSelect();
    virtual ~GuiDirSelect();

    char *DoModal(GameWindow *gwd, const char *startdir, const char *filename);
private:
    // Dir list
    DirList dirs;
    int num_dirs;
    int current_index;
    int upper_index;
    int lower_index;
    DirElement emptyDir;
    DirElement *dirInfo[NUM_DIR_ITEMS];
    char *dirList[NUM_DIR_ITEMS];
    Sprite titleTxtSprite[NUM_DIR_ITEMS];
    Sprite sprSelector;
    Sprite sprArrowUp;
    Sprite sprArrowDown;
    DrawableImage titleTxtImg[NUM_DIR_ITEMS];
    DrawableImage *titleTxtImgPtr[NUM_DIR_ITEMS];

    void InitTitleList(LayerManager *manager,
                       TextRender *fontArial, int fontsize,
                       int x, int y, int sx, int sy, int pitch);
    void ClearTitleList(void);
    void SetListIndex(int index);
    void SetSelected(int selected);
};

#endif

