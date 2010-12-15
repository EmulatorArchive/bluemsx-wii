
#include "GuiDlgMenu.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmSelectionList.h"

#define MENU_YPITCH      56

void GuiDlgMenu::Initialize(const char **items, int num, int width)
{
    CleanUp();

    // Add container
    int height = num_item_rows*MENU_YPITCH+(MENU_YPITCH/2);
    int posx = GetWidth()/2-width/2;
    int posy = GetHeight()/2-height/2;
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, posx, posy, width, height, 192);
    AddTop(frame, new GuiEffectFade(10));
    width = frame->GetWidth();
    height = frame->GetHeight();

    // Menu list
    GXColor white = {255, 255, 255, 255};
    list->InitSelection(new GuiElmListLineDefault(this, "defaultline", white, 32, false),
                        (void**)items, num, 0, MENU_YPITCH,
                        posx+10, posy+24, 24, width-32);
    AddTop(list, new GuiEffectFade(10));
    SetSelected(list);
    initialized = true;
}

void GuiDlgMenu::CleanUp(void)
{
    if( initialized ) {
        Remove(list, new GuiEffectFade(10));
        if( frame ) {
            RemoveAndDelete(frame, new GuiEffectFade(10));
        }
        initialized = false;
    }
}

SELRET GuiDlgMenu::DoModal(int *selected)
{
    assert( initialized );

    // Run GUI
    SELRET retval = SELRET_KEY_B;
    if( Run() ) {
        retval = SELRET_SELECTED;
        *selected = list->GetSelectedItem();
    }

    return retval;
}

GuiDlgMenu::GuiDlgMenu(GuiContainer *parent, const char *name, int rows)
           :GuiDialog(parent, name)
{
    initialized = false;
    frame = NULL;
    list = new GuiElmSelectionList(this, name, rows);
    num_item_rows = rows;
    SetRefPixelPosition(GetWidth()/2, GetHeight()/2);
}

GuiDlgMenu::~GuiDlgMenu()
{
    CleanUp();
    Delete(list);
}

