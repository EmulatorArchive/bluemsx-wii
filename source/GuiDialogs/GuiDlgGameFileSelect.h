
#ifndef _GUI_DLG_GAME_FILE_SELECT_H
#define _GUI_DLG_GAME_FILE_SELECT_H

#include "../GuiBase/GuiDialog.h"

class GuiElmSelectionList;
class GuiElmFrame;

class GuiDlgGameFileSelect : public GuiDialog {
public:
    GuiDlgGameFileSelect(GuiContainer *parent, const char *name);
    virtual ~GuiDlgGameFileSelect();

    int Create(char *directory);
    char* DoModal(void);
private:
    GuiElmFrame *frame;
    GuiElmSelectionList *list;
    int num_files;
    int posx;
    int posy;
    int sizex;
    int sizey;
    char *filenames[256];

    void CreateFileList(char *directory);
    void FreeFileList(void);
};

#endif

