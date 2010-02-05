#include <stdio.h>
#include <stdlib.h>

#include "GuiMenu.h"
#include "GuiRunner.h"
#include "GuiSelectionList.h"
#include "GuiContainer.h"

#define MENU_YPITCH      56
#define MENU_FADE_FRAMES 10

SELRET GuiMenu::DoModal(int *selected, const char **items, int num, int width)
{
    GuiRunner *runner = new GuiRunner(manager, this);

    // Claim UI
    manager->Lock();

    // Add container
    int height = num_item_rows*MENU_YPITCH+(MENU_YPITCH/2);
    int posx = manager->GetWidth()/2-width/2;
    int posy = manager->GetHeight()/2-height/2;
    GuiContainer *container = new GuiContainer(posx, posy, width, height, 192);
    manager->AddTop(container, MENU_FADE_FRAMES);
    width = container->GetWidth();
    height = container->GetHeight();

    // Menu list
    list->InitSelection(items, num, 0, 32, MENU_YPITCH,
                           posx+16, posy+24, 24, width-32, false);
    runner->AddTop(list, MENU_FADE_FRAMES);
    runner->SetSelected(list);

    // Start displaying
    manager->Unlock();

    // Run GUI
    SELRET retval = SELRET_KEY_B;
    if( runner->Run() ) {
        retval = SELRET_SELECTED;
        *selected = list->GetSelected();
    }

    // Claim UI
    manager->Lock();

    // Remove elements
    runner->Remove(list, MENU_FADE_FRAMES);
    manager->RemoveAndDelete(container, NULL, MENU_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    delete runner;
    return retval;
}

GuiMenu::GuiMenu(GuiManager *man, int rows)
{
    list = new GuiSelectionList(man, rows);
    manager = man;
    num_item_rows = rows;
}

GuiMenu::~GuiMenu()
{
    delete list;
}

