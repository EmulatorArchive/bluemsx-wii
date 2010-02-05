
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include "GuiDialog.h"
#include "GuiSelectionList.h"

class GuiSelectionList;

class GuiMenu : public GuiDialog
{
public:
    GuiMenu(GuiManager *man, int rows);
    virtual ~GuiMenu();

    SELRET DoModal(int *selected, const char **items, int num, int width);
private:
    GuiManager *manager;
    GuiSelectionList *list;
    int num_item_rows;
};

#endif

