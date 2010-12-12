
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include "../GuiBase/GuiDialog.h"
#include "../GuiElements/GuiElmSelectionList.h"

class GuiElmFrame;

class GuiDlgMenu : public GuiDialog
{
public:
    GuiDlgMenu(GuiContainer *parent, const char *name, int rows);
    virtual ~GuiDlgMenu();

    void Initialize(const char **items, int num, int width);
    void CleanUp(void);
    SELRET DoModal(int *selected);
private:
    bool initialized;
    GuiElmFrame *frame; 
    GuiElmSelectionList *list;
    int num_item_rows;
};

#endif

