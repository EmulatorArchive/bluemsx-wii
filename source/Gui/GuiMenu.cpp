#include <stdio.h>
#include <stdlib.h>

#include "GuiMenu.h"
#include "GuiContainer.h"


int GuiMenu::DoModal(const char **items, int num, int width)
{
    // Claim UI
    manager->Lock();

    // Add container
    int height = num_item_rows*64+32;
    int posx = (320-(width >> 1)) & ~3;
    int posy = (240+37-(height >> 1)) & ~3;
    GuiContainer container(posx, posy, width, height, 192);
    manager->AddTop(container.GetLayer());
    width = container.GetWidth();
    height = container.GetHeight();

    // Start displaying
    manager->Unlock();

    // Menu list
    ShowSelection(items, num, 0, 32, 64,
                  posx+8, posy+24, 24, width-16);
    int sel = DoSelection();
    RemoveSelection();

    // Claim UI
    manager->Lock();

    // Remove container
    manager->Remove(container.GetLayer());

    // Release UI
    manager->Unlock();

    return sel;
}

GuiMenu::GuiMenu(GuiManager *man, int rows)
       : GuiSelectionList(man, rows)
{
    manager = man;
    num_item_rows = rows;
}

GuiMenu::~GuiMenu()
{
}

