
#ifndef _GUI_CHECK_LIST_H
#define _GUI_CHECK_LIST_H

#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiEffectFade.h"
#include "GuiElmSelectionList.h"

class GuiElmCheckList : public GuiDialog
{
public:
    GuiElmCheckList(GuiContainer *parent, const char *name, int rows);
    virtual ~GuiElmCheckList();

    SELRET DoModal(int *selected, const char **items, bool *items_selected, int num, float width);
private:
    GuiContainer *container;
    GuiElmSelectionList *list;
    GuiEffectFade effectDefault;
    int num_item_rows;
};

#endif


