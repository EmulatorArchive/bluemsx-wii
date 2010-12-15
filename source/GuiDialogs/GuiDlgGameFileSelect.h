
#ifndef _GUI_DLG_GAME_FILE_SELECT_H
#define _GUI_DLG_GAME_FILE_SELECT_H

#include "../GuiBase/GuiDialog.h"

typedef struct _fileitem FILEITEM;

class GuiElmSelectionList;
class GuiElmFrame;

class GuiDlgGameFileSelect : public GuiDialog {
public:
    GuiDlgGameFileSelect(GuiContainer *parent, const char *name,
                         const char *directory);
    virtual ~GuiDlgGameFileSelect();

    int Create(void);
    char* DoModal(void);
private:
    char *org_dir;
    char *root_dir;
    GuiElmFrame *frame;
    GuiElmSelectionList *list;
    int num_files;
    int posx;
    int posy;
    int sizex;
    int sizey;
    FILEITEM **items;

    void CreateFileList(void);
    void FreeFileList(void);
};

#endif

