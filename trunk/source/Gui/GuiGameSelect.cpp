#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "GuiGameSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"

// Resources
#include "GuiImages.h"


void GuiGameSelect::SetScreenShotImage(int index, Image *img)
{
    if( img == NULL ) {
        img = g_imgNoise;
    }
    Sprite *spr = &sprScreenShot[index];
    spr->SetImage(img);
    spr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr->SetRefPixelPosition(0, 0);
    spr->SetStretchWidth(252.0f/img->GetWidth());
    spr->SetStretchHeight(180.0f/img->GetHeight());
}

void GuiGameSelect::OnSetSelected(int index, int selected)
{
    // Update screenshots
    if( selected >= 0 ) {
        GameElement *game = games.GetGame(index+selected);
        SetScreenShotImage(0, game->GetImage(0));
        SetScreenShotImage(1, game->GetImage(1));
    }
    // Free images of games that are not on the screen
    for(int i = 0; i < games.GetNumberOfGames(); i++) {
        GameElement *game = games.GetGame(i);
        if( game != NULL && (i < index || i >= (index + NUM_LIST_ITEMS)) ) {
            game->FreeImage(0);
            game->FreeImage(1);
        }
    }
}

GameElement *GuiGameSelect::DoModal(const char *dir, const char *filename, GameElement *select)
{
    GameElement *returnValue = NULL;

    // Load games database
    chdir(dir);
    games.Load(filename);
    num_games = games.GetNumberOfGames();
    if( num_games == 0 ) {
        return NULL;
    }
    title_list = (const char**)malloc(num_games * sizeof(const char*));
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }

    // Claim UI
    manager->Lock();

    // Containers
    GuiContainer grWinList(32-8, 24, 288, 420+12);
	manager->AddTop(grWinList.GetLayer());
    GuiContainer grWinTitle(344-8, 24, 264+12, 204);
	manager->AddTop(grWinTitle.GetLayer());
    GuiContainer grWinPlay(344-8, 240+12, 264+12, 204);
	manager->AddTop(grWinPlay.GetLayer());

    // Screen shots (240x186)
	sprScreenShot[0].SetPosition(344+12-8, 24+12);
	sprScreenShot[1].SetPosition(344+12-8, 240+12+12);
	manager->AddTop(&sprScreenShot[0]);
	manager->AddTop(&sprScreenShot[1]);

    // On re-enter, find selected entry
    int sel = 0;
    if( select != NULL ) {
        for( int i = 0; i < num_games; i++ ) {
            if( strcmp(select->GetName(), title_list[i])==0 ) {
                sel = i;
                break;
            }
        }
    }

    // Add selection list
    ShowSelection(title_list, num_games, sel, 24, 34,
                  30, 34, 12, 264+12);

    // Release UI
    manager->Unlock();

    // Menu loop
    do {
        sel = DoSelection();

        if( sel >= 0 ) {
            returnValue = games.GetGame(sel);
            // confirmation
            char str[256];
            strcpy(str, "Do you want to start\n\"");
            strcat(str, returnValue->GetName());
            strcat(str, "\"");
            GuiMessageBox *msgbox = new GuiMessageBox(manager);
            bool ok = msgbox->Show(str, NULL, true, 192);
            msgbox->Remove();
            delete msgbox;
            if( ok ) {
                break;
            }
        }else{
            returnValue = NULL;
        }
    }while(sel >= 0);

    // Claim UI
    manager->Lock();

    // Remove UI elements
    RemoveSelection();
	manager->Remove(&sprScreenShot[0]);
	manager->Remove(&sprScreenShot[1]);
	manager->Remove(grWinList.GetLayer());
	manager->Remove(grWinTitle.GetLayer());
	manager->Remove(grWinPlay.GetLayer());

    // Release UI
    manager->Unlock();

    if( returnValue != NULL ) {
        GameElement *game = new GameElement(returnValue);
        games.Clear();
        return game;
    }else{
        games.Clear();
        return NULL;
    }
}

GuiGameSelect::GuiGameSelect(GuiManager *man) : GuiSelectionList(man, NUM_LIST_ITEMS)
{
    manager = man;
    title_list = NULL;
}

GuiGameSelect::~GuiGameSelect()
{
    if( title_list ) {
        free(title_list);
    }
}

