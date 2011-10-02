
#ifndef _GUI_DIRSELECT_H
#define _GUI_DIRSELECT_H

#include "../GuiBase/GuiDialog.h"
#include "../Gui/DirectoryList.h"
#include "../Gui/DirectoryHelper.h"

#ifndef MAX_PATH
#define MAX_PATH        256
#endif
#define NUM_DIR_ITEMS   7

class GuiElmFrame;
class GuiElmSelectionList;

class GuiDlgDirSelect : public GuiDialog
{
public:
    GuiDlgDirSelect(GuiContainer* parent, const char* name, const char* startdir, const char* filename);
    virtual ~GuiDlgDirSelect();

    char* Run(void);
private:
    char* InitialiseList(char *prevsel);

    DirectoryHelper directory;
    const char **title_list;
    GuiElmFrame *frame;
    GuiElmSelectionList *list;
    DirList dirs;
    int num_dirs;
    int dir_level;
    char xmlfile[MAX_PATH];
};

#endif

