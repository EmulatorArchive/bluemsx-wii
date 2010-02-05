
#ifndef _GUI_DIRSELECT_H
#define _GUI_DIRSELECT_H

#include "GuiDialog.h"
#include "DirectoryList.h"

#define MAX_PATH        256
#define NUM_DIR_ITEMS   7

class GuiSelectionList;

class GuiDirSelect : public GuiDialog
{
public:
    GuiDirSelect(GuiManager *man, const char *startdir, const char *filename);
    virtual ~GuiDirSelect();

    char *DoModal(void);
private:
    GuiManager *manager;
    GuiRunner *runner;
    GuiSelectionList *list;
    DirList dirs;
    int num_dirs;
    int dir_level;
    char current_dir[MAX_PATH];
    char xmlfile[MAX_PATH];
};

#endif

