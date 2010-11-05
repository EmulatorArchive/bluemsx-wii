
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include "GuiDialog.h"
#include "GuiSelectionList.h"

class GuiMenu : public GuiDialog
{
public:
    GuiMenu(GuiContainer *cntr, int rows);
    virtual ~GuiMenu();

    SELRET DoModal(int *selected, const char **items, int num, int width);
private:
    GuiSelectionList *list;
    int num_item_rows;
};

#endif

