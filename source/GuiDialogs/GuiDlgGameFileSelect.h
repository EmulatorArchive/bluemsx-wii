
#ifndef _GUI_DLG_GAME_FILE_SELECT_H
#define _GUI_DLG_GAME_FILE_SELECT_H

#include "../GuiBase/GuiDialog.h"
#include "../Gui/DirectoryHelper.h"

typedef struct _fileitem FILEITEM;

class GuiElmSelectionList;
class GuiElmFrame;

class GuiDlgGameFileSelect : public GuiDialog {
public:
    GuiDlgGameFileSelect(GuiContainer *parent, const char *name,
                         const char *dir);
    virtual ~GuiDlgGameFileSelect();

    int Create(void);
    char* DoModal(void);
private:
    DirectoryHelper directory;
    char *org_dir;
    char *root_dir;
    GuiElmFrame *frame;
    GuiElmSelectionList *list;
    int num_files;
    float posx;
    float posy;
    float sizex;
    float sizey;
    FILEITEM **items;

    void CreateFileList(void);
    void FreeFileList(void);
};

#endif

