#include <stdio.h>
#include <stdlib.h>

#include "GuiCheckList.h"
#include "GuiSelectionList.h"
#include "GuiFrame.h"
#include "GuiEffectFade.h"

#define CHECKLIST_YPITCH 40
#define CHECKLIST_EFFECT new GuiEffectFade(10)

SELRET GuiCheckList::DoModal(int *selected, const char **items, bool *items_selected, int num, int width)
{
    // TODO
    (void)items_selected;

    // Add container
    int height = num_item_rows*CHECKLIST_YPITCH+(CHECKLIST_YPITCH/2);
    int posx = container->GetWidth()/2-width/2;
    int posy = container->GetHeight()/2-height/2;
    GuiFrame *frame = new GuiFrame(posx, posy, width, height, 192);
    AddTop(frame, CHECKLIST_EFFECT);
    width = container->GetWidth();
    height = container->GetHeight();

    // Menu list
    list->InitSelection(items, num, 0, 32, CHECKLIST_YPITCH,
                           posx+16, posy+24, 24, width-32, false);
    SetSelected(list);
    AddTop(list, CHECKLIST_EFFECT);

    container->AddTop(this, CHECKLIST_EFFECT);

    // Run GUI
    SELRET retval = SELRET_KEY_B;
    if( Run() ) {
        retval = SELRET_SELECTED;
        *selected = list->GetSelectedItem();
    }

    // Remove elements
    Remove(list, CHECKLIST_EFFECT);
    RemoveAndDelete(frame, CHECKLIST_EFFECT);

    container->Remove(this, CHECKLIST_EFFECT);

    return retval;
}

GuiCheckList::GuiCheckList(GuiContainer *cntr, int rows)
             :GuiDialog(cntr)
{
    list = new GuiSelectionList(cntr, rows);
    container = cntr;
    num_item_rows = rows;
}

GuiCheckList::~GuiCheckList()
{
    delete list;
}

