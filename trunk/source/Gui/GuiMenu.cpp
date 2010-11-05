#include <stdio.h>
#include <stdlib.h>

#include "GuiMenu.h"
#include "GuiSelectionList.h"
#include "GuiFrame.h"
#include "GuiEffectFade.h"

#define MENU_YPITCH      56
#define MENU_FADE_FRAMES 10

SELRET GuiMenu::DoModal(int *selected, const char **items, int num, int width)
{
    // Add container
    int height = num_item_rows*MENU_YPITCH+(MENU_YPITCH/2);
    int posx = GetWidth()/2-width/2;
    int posy = GetHeight()/2-height/2;
    GuiFrame *frame = new GuiFrame(posx, posy, width, height, 192);
    RegisterForDelete(frame);
    AddTop(frame, new GuiEffectFade(MENU_FADE_FRAMES));
    width = frame->GetWidth();
    height = frame->GetHeight();

    // Menu list
    list->InitSelection(items, num, 0, 32, MENU_YPITCH,
                           posx+16, posy+24, 24, width-32, false);
    SetSelected(list);
    AddTop(list, new GuiEffectFade(MENU_FADE_FRAMES));

    // Run GUI
    SELRET retval = SELRET_KEY_B;
    if( Run() ) {
        retval = SELRET_SELECTED;
        *selected = list->GetSelectedItem();
    }

    // Remove elements
    Remove(list, new GuiEffectFade(MENU_FADE_FRAMES));
    RemoveAndDelete(frame, new GuiEffectFade(MENU_FADE_FRAMES));

    return retval;
}

GuiMenu::GuiMenu(GuiContainer *cntr, int rows)
        :GuiDialog(cntr)
{
    list = new GuiSelectionList(this, rows);
    RegisterForDelete(list);
    num_item_rows = rows;
}

GuiMenu::~GuiMenu()
{
    Delete(list);
}

