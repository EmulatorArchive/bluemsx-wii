
#ifndef _GUI_DIRSELECT_H
#define _GUI_DIRSELECT_H

#include "../GuiBase/GuiDialog.h"
#include "../Gui/DirectoryList.h"

#define MAX_PATH        256
#define NUM_DIR_ITEMS   7

class GuiElmFrame;
class GuiElmSelectionList;

class GuiDlgDirSelect : public GuiDialog
{
public:
    GuiDlgDirSelect(GuiContainer* parent, const char* name, const char* startdir, const char* filename);
    virtual ~GuiDlgDirSelect();

    char* DoModal(void);
private:
    char* InitialiseList(char *prevsel);

    const char **title_list;
    GuiElmFrame *frame;
    GuiElmSelectionList *list;
    DirList dirs;
    int num_dirs;
    int dir_level;
    char current_dir[MAX_PATH];
    char xmlfile[MAX_PATH];
};

#endif

