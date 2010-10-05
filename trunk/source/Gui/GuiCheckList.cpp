#include <stdio.h>
#include <stdlib.h>

#include "GuiCheckList.h"
#include "GuiRunner.h"
#include "GuiSelectionList.h"
#include "GuiContainer.h"

#define CHECKLIST_YPITCH 40
#define CHECKLIST_FADE_FRAMES 10

SELRET GuiCheckList::DoModal(int *selected, const char **items, bool *items_selected, int num, int width)
{
    GuiRunner *runner = new GuiRunner(manager, this);

    // TODO
    (void)items_selected;

    // Claim UI
    manager->Lock();

    // Add container
    int height = num_item_rows*CHECKLIST_YPITCH+(CHECKLIST_YPITCH/2);
    int posx = manager->GetWidth()/2-width/2;
    int posy = manager->GetHeight()/2-height/2;
    GuiContainer *container = new GuiContainer(posx, posy, width, height, 192);
    manager->AddTop(container, CHECKLIST_FADE_FRAMES);
    width = container->GetWidth();
    height = container->GetHeight();

    // Menu list
    list->InitSelection(items, num, 0, 32, CHECKLIST_YPITCH,
                           posx+16, posy+24, 24, width-32, false);
    runner->AddTop(list, CHECKLIST_FADE_FRAMES);
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
    runner->Remove(list, CHECKLIST_FADE_FRAMES);
    manager->RemoveAndDelete(container, NULL, CHECKLIST_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    delete runner;
    return retval;
}

GuiCheckList::GuiCheckList(GuiManager *man, int rows)
{
    list = new GuiSelectionList(man, rows);
    manager = man;
    num_item_rows = rows;
}

GuiCheckList::~GuiCheckList()
{
    delete list;
}

