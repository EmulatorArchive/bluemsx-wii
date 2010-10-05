
#ifndef _GUI_CHECK_LIST_H
#define _GUI_CHECK_LIST_H

#include "GuiDialog.h"
#include "GuiSelectionList.h"

class GuiCheckList : public GuiDialog
{
public:
    GuiCheckList(GuiManager *man, int rows);
    virtual ~GuiCheckList();

    SELRET DoModal(int *selected, const char **items, bool *items_selected, int num, int width);
private:
    GuiManager *manager;
    GuiSelectionList *list;
    int num_item_rows;
};

#endif


