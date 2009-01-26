
#ifndef _GUI_DIRSELECT_H
#define _GUI_DIRSELECT_H

#include <wiisprite.h>
#include "spritemanager.h"

#include "DirectoryList.h"

#define MAX_PATH        256
#define NUM_DIR_ITEMS   6

class GuiDirSelect {
public:
    GuiDirSelect(LayerManager *layman, void *sem, const char *startdir, const char *filename);
    virtual ~GuiDirSelect();

    char *DoModal(void);
private:
    DirList dirs;
    LayerManager *manager;
    void *video_semaphore;
    int num_dirs;
    int current_index;
    int upper_index;
    int lower_index;
    int dir_level;
    char *current_dir;
    char *xmlfile;
    DirElement emptyDir;
    DirElement *dirInfo[NUM_DIR_ITEMS];
    char *dirList[NUM_DIR_ITEMS];
    Sprite titleTxtSprite[NUM_DIR_ITEMS];
    Sprite sprSelector;
    Sprite sprArrowUp;
    Sprite sprArrowDown;
    DrawableImage titleTxtImg[NUM_DIR_ITEMS];
    DrawableImage *titleTxtImgPtr[NUM_DIR_ITEMS];

    void InitTitleList(TextRender *fontArial, int fontsize,
                       int x, int y, int sx, int sy, int pitch);
    void RemoveTitleList(void);
    void ClearTitleList(void);
    void SetListIndex(int index);
    void SetSelected(int selected);
};

#endif

