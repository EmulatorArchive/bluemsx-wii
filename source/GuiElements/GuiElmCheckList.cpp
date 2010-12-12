#include <stdio.h>
#include <stdlib.h>

#include "GuiElmCheckList.h"
#include "GuiElmSelectionList.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiBase/GuiEffectFade.h"

#define CHECKLIST_YPITCH 40
#define CHECKLIST_EFFECT new GuiEffectFade(10)

SELRET GuiElmCheckList::DoModal(int *selected, const char **items, bool *items_selected, int num, int width)
{
    // TODO
    (void)items_selected;

    // Add container
    int height = num_item_rows*CHECKLIST_YPITCH+(CHECKLIST_YPITCH/2);
    int posx = GetWidth()/2-width/2;
    int posy = GetHeight()/2-height/2;
    GuiElmFrame *frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, posx, posy, width, height, 192);
    AddTop(frame, CHECKLIST_EFFECT);
    width = GetWidth();
    height = GetHeight();

    // Menu list
    GXColor white = {255, 255, 255, 255};
    list->InitSelection(items, num, 0, 32, white, CHECKLIST_YPITCH,
                           posx+16, posy+24, 24, width-32, false);
    SetSelected(list);
    AddTop(list, CHECKLIST_EFFECT);

    AddTop(this, CHECKLIST_EFFECT);

    // Run GUI
    SELRET retval = SELRET_KEY_B;
    if( Run() ) {
        retval = SELRET_SELECTED;
        *selected = list->GetSelectedItem();
    }

    // Remove elements
    Remove(list, CHECKLIST_EFFECT);
    RemoveAndDelete(frame, CHECKLIST_EFFECT);

    Remove(this, CHECKLIST_EFFECT);

    return retval;
}

GuiElmCheckList::GuiElmCheckList(GuiContainer *parent, const char *name, int rows)
               : GuiDialog(parent, name)
{
    list = new GuiElmSelectionList(this, "list", rows);
    AddTop(list);
    num_item_rows = rows;
}

GuiElmCheckList::~GuiElmCheckList()
{
    RemoveAndDelete(list);
}

