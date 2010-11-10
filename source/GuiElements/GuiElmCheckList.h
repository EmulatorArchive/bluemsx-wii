
#ifndef _GUI_CHECK_LIST_H
#define _GUI_CHECK_LIST_H

#include "../GuiBase/GuiDialog.h"
#include "GuiElmSelectionList.h"

class GuiElmCheckList : public GuiDialog
{
public:
    GuiElmCheckList(GuiContainer *cntr, int rows);
    virtual ~GuiElmCheckList();

    SELRET DoModal(int *selected, const char **items, bool *items_selected, int num, int width);
private:
    GuiContainer *container;
    GuiElmSelectionList *list;
    int num_item_rows;
};

#endif


