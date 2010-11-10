
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include "../GuiBase/GuiDialog.h"
#include "../GuiElements/GuiElmSelectionList.h"

class GuiDlgMenu : public GuiDialog
{
public:
    GuiDlgMenu(GuiContainer *cntr, int rows);
    virtual ~GuiDlgMenu();

    SELRET DoModal(int *selected, const char **items, int num, int width);
private:
    GuiElmSelectionList *list;
    int num_item_rows;
};

#endif

