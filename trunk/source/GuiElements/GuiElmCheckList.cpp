#include <stdio.h>
#include <stdlib.h>

#include "GuiElmCheckList.h"
#include "GuiElmSelectionList.h"
#include "../GuiElements/GuiElmFrame.h"

#define CHECKLIST_YPITCH 40

GuiElmCheckList::GuiElmCheckList(GuiContainer *parent, const char *name, int rows) :
                 GuiDialog(parent, name),
                 effectDefault(10)
{
    list = new GuiElmSelectionList(this, "list", rows);
    AddTop(list);
    num_item_rows = rows;
}

GuiElmCheckList::~GuiElmCheckList()
{
    RemoveAndDelete(list);
}

SELRET GuiElmCheckList::DoModal(int *selected, const char **items, bool *items_selected, int num, float width)
{
    // TODO
    (void)items_selected;

    // Add container
    float height = (float)num_item_rows*CHECKLIST_YPITCH+(CHECKLIST_YPITCH/2);
    float posx = GetWidth()/2-width/2;
    float posy = GetHeight()/2-height/2;
    GuiElmFrame *frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, posx, posy, width, height, 0.75f);
    AddTop(frame, effectDefault);
    width = GetWidth();
    height = GetHeight();

    // Menu list
    GXColor white = {255, 255, 255, 255};
    list->InitSelection(new GuiElmListLineDefault(this, "defaultline", white, 32, 24, false),
                        (void**)items, num, 0, CHECKLIST_YPITCH,
                        posx+16, posy+24, width-32);
    list->SetFocus(true);
    AddTop(list, effectDefault);

    AddTop(this, effectDefault);

    // Run GUI
    SELRET retval = SELRET_KEY_B;
    if( Run() ) {
        retval = SELRET_SELECTED;
        *selected = list->GetSelectedItem();
    }

    // Remove elements
    Remove(list, effectDefault);
    RemoveAndDelete(frame, effectDefault);

    Remove(this, effectDefault);

    return retval;
}

