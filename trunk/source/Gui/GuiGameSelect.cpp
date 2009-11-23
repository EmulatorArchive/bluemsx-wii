#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "GuiButton.h"
#include "GuiGameSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"

// Resources
#include "GuiImages.h"

#define GAMESEL_FADE_DEFAULT      10
#define GAMESEL_FADE_RESTART      30
#define GAMESEL_DELAY_RESTART     6
#define GAMESEL_FADE_SCREENSHOTS  10
#define GAMESEL_DELAY_SCREENSHOTS 8

void GuiGameSelect::SetScreenShotImage(int index, Image *img)
{
    if( img == NULL ) {
        img = g_imgNoise;
    }
    Sprite *spr = sprScreenShot[index];
    spr->SetImage(img);
    spr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr->SetRefPixelPosition(0, 0);
    spr->SetStretchWidth(screenshotWidth / img->GetWidth());
    spr->SetStretchHeight(screenshotHeigth / img->GetHeight());
}

void GuiGameSelect::OnSetSelected(int index, int selected)
{
    bool first = true;
    last_index = index;
    last_selected = selected;
    // (Re)create screenshot sprites
    if( sprScreenShot[0] != NULL ) {
        first = false;
        manager->RemoveAndDelete(sprScreenShot[0], sprScreenShot[0]->GetImage(),
                                 fade_time, fade_delay);
    }
    if( sprScreenShot[1] != NULL ) {
        first = false;
        manager->RemoveAndDelete(sprScreenShot[1], sprScreenShot[1]->GetImage(),
                                 fade_time, fade_delay);
    }
    sprScreenShot[0] = new Sprite;
    sprScreenShot[0]->SetPosition(344+12-8, screenshotYpos1);
    sprScreenShot[1] = new Sprite;
    sprScreenShot[1]->SetPosition(344+12-8, screenshotYpos2);
    // Update screenshots
    if( selected >= 0 ) {
        GameElement *game = games.GetGame(index+selected);
        SetScreenShotImage(0, new Image(game->GetImage(0)));
        SetScreenShotImage(1, new Image(game->GetImage(1)));
    }
    // Add to screen
    manager->AddOnTopOf(grWinTitle, sprScreenShot[0], fade_time);
    manager->AddOnTopOf(grWinPlay, sprScreenShot[1], fade_time);
    // Free images of games that are not on the screen
    for(int i = 0; i < games.GetNumberOfGames(); i++) {
        GameElement *game = games.GetGame(i);
        if( game != NULL && (i < index || i >= (index + NUM_LIST_ITEMS)) ) {
            game->FreeImage(0);
            game->FreeImage(1);
        }
    }
}

bool GuiGameSelect::OnUpdateCursorPosition(Sprite *sprite)
{
    selected_button = GLEDITSEL_NONE;
    if( grButtonAdd != NULL ) {
        if( grButtonAdd->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_ADD; }
        if( grButtonDel->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_DEL; }
        if( grButtonUp->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_UP; }
        if( grButtonDown->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_DOWN; }
        if( grButtonSettings->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_SETTINGS; }
        if( grButtonDelScr1->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_SCRSHOT_1; }
        if( grButtonDelScr2->Update(sprite) ) { sprite = NULL; selected_button = GLEDITSEL_SCRSHOT_2; }
    }
    return selected_button != GLEDITSEL_NONE;
}

bool GuiGameSelect::Load(const char *dir, const char *filename)
{
    // Load games database
    chdir(dir);
    games.Load(filename);
    num_games = games.GetNumberOfGames();
    if( num_games == 0 ) {
        return false;
    }
    title_list = (const char**)malloc(num_games * sizeof(const char*));
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }
    return true;
}

GameElement *GuiGameSelect::DoModal(GameElement *select)
{
    GameElement *returnValue = NULL;
    bool editMode = false;

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

    // Init fade parameters
    fade_time = GAMESEL_FADE_DEFAULT;
    fade_delay = 0;

    // Init selection list
    InitSelection(title_list, num_games, sel, 22, 31,
                  30, 38, 12, 264+12, false);

    // Outer loop
    bool restart = false;
    do {
        // Claim UI
        manager->Lock();

        // Remove version from background
        background->HideVersion(fade_time);

        // Add selection list
        GuiContainer *grWinList;
        if( !restart ) {
            grWinList = new GuiContainer(32-8, 28, 288, 33*12);
            manager->AddTop(grWinList, fade_time);
            ShowSelection(fade_time);
        }
        restart = false;

        // GUI Elements
        if( editMode ) {
            // Containers
            grWinTitle = new GuiContainer(344-8, 28, 264+12, 14*12);
            manager->AddTop(grWinTitle, fade_time);
            grWinPlay = new GuiContainer(344-8, 232-30, 264+12, 14*12);
            manager->AddTop(grWinPlay, fade_time);
            grWinControls = new GuiContainer(344-8, 232-30+14*12+6, 264+12, 4*12);
            manager->AddTop(grWinControls, fade_time);
            // Icons
            grButtonAdd = new GuiButton(manager);
            grButtonAdd->ShowImageButton(g_imgAdd, 344+14, 232-30+14*12+6+8, fade_time);
            grButtonDel = new GuiButton(manager);
            grButtonDel->ShowImageButton(g_imgDelete, 344+14+50, 232-30+14*12+6+8, fade_time);
            grButtonUp = new GuiButton(manager);
            grButtonUp->ShowImageButton(g_imgUp, 344+14+2*50, 232-30+14*12+6+8, fade_time);
            grButtonDown = new GuiButton(manager);
            grButtonDown->ShowImageButton(g_imgDown, 344+14+3*50, 232-30+14*12+6+8, fade_time);
            grButtonSettings = new GuiButton(manager);
            grButtonSettings->ShowImageButton(g_imgSettings, 344+14+4*50, 232-30+14*12+6+8, fade_time);
            grButtonDelScr1 = new GuiButton(manager);
            grButtonDelScr1->ShowImageButton(g_imgDelete2, 344+8+264+12-54, 28+16, fade_time);
            grButtonDelScr2 = new GuiButton(manager);
            grButtonDelScr2->ShowImageButton(g_imgDelete2, 344+8+264+12-54, 232-30+16, fade_time);
            // Screenshot coordinates
            screenshotWidth = (252.0f/16.0f)*14.0f;
            screenshotHeigth = (168.0f/16.0f)*14.0f;
            screenshotYpos1 = 24+16;
            screenshotYpos2 = 228+16-30;
        }else{
            // Containers
            grWinTitle = new GuiContainer(344-8, 28, 264+12, 16*12);
            manager->AddTop(grWinTitle, fade_time);
            grWinPlay = new GuiContainer(344-8, 232, 264+12, 16*12);
            manager->AddTop(grWinPlay, fade_time);
            // Screenshot coordinates
            screenshotWidth = 252.0f;
            screenshotHeigth = 168.0f;
            screenshotYpos1 = 24+16;
            screenshotYpos2 = 228+16;
        }
        OnSetSelected(last_index, last_selected);

        // Release UI
        manager->Unlock();

        // Menu loop
        do {
            fade_time = GAMESEL_FADE_SCREENSHOTS;
            fade_delay = GAMESEL_DELAY_SCREENSHOTS;

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
                switch( sel ) {
                    case SELRET_KEY_B:
                    case SELRET_KEY_HOME:
                        returnValue = NULL;
                        break;
                    case SELRET_KEY_PLUS:
                        editMode = !editMode;
                        restart = true;
                        break;
                    case SELRET_CUSTOM: /* User clicked in some of the buttons */
                        {
                            GuiMessageBox *msgbox = new GuiMessageBox(manager);
                            (void)msgbox->Show("Hey, did you clicked something?", NULL, true, 192);
                            msgbox->Remove();
                            delete msgbox;
                        }
                        //...
                        sel = 0; /* Don't leave */
                        break;
                }
            }
        }while(sel >= 0);

        if( restart ) {
            fade_time = GAMESEL_FADE_RESTART;
            fade_delay = GAMESEL_DELAY_RESTART;
        }else{
            fade_time = GAMESEL_FADE_DEFAULT;
            fade_delay = 0;
        }

        // Claim UI
        manager->Lock();

        // Remove UI elements
        if( !restart ) {
            manager->RemoveAndDelete(grWinList, NULL, fade_time, fade_delay);
            RemoveSelection(fade_time, fade_delay);
        }
        manager->RemoveAndDelete(sprScreenShot[0], sprScreenShot[0]->GetImage(), fade_time, fade_delay);
        sprScreenShot[0] = NULL;
        manager->RemoveAndDelete(sprScreenShot[1], sprScreenShot[1]->GetImage(), fade_time, fade_delay);
        sprScreenShot[1] = NULL;
        manager->RemoveAndDelete(grWinTitle, NULL, fade_time, fade_delay);
        manager->RemoveAndDelete(grWinPlay, NULL, fade_time, fade_delay);
        manager->RemoveAndDelete(grWinControls, NULL, fade_time, fade_delay);
        grWinTitle = grWinPlay = grWinControls = NULL;
        if( grButtonAdd != NULL ) {
            grButtonAdd->Remove(fade_time, fade_delay);
            grButtonDel->Remove(fade_time, fade_delay);
            grButtonUp->Remove(fade_time, fade_delay);
            grButtonDown->Remove(fade_time, fade_delay);
            grButtonSettings->Remove(fade_time, fade_delay);
            grButtonDelScr1->Remove(fade_time, fade_delay);
            grButtonDelScr2->Remove(fade_time, fade_delay);
            delete grButtonAdd;
            delete grButtonDel;
            delete grButtonUp;
            delete grButtonDown;
            delete grButtonSettings;
            delete grButtonDelScr1;
            delete grButtonDelScr2;
            grButtonAdd = NULL;
            grButtonDel = NULL;
            grButtonUp = NULL;
            grButtonDown = NULL;
            grButtonSettings = NULL;
            grButtonDelScr1 = NULL;
            grButtonDelScr2 = NULL;
        }

        // Show version on background again
        background->ShowVersion(fade_time);

        // Release UI
        manager->Unlock();
    }while( restart );

    if( returnValue != NULL ) {
        GameElement *game = new GameElement(returnValue);
        games.Clear();
        return game;
    }else{
        games.Clear();
        return NULL;
    }
}

GuiGameSelect::GuiGameSelect(GuiManager *man, GuiBackground *bgr) : GuiSelectionList(man, NUM_LIST_ITEMS)
{
    manager = man;
    background = bgr;
    title_list = NULL;
    sprScreenShot[0] = NULL;
    sprScreenShot[1] = NULL;

    grWinTitle = NULL;
    grWinPlay = NULL;
    grWinControls = NULL;

    grButtonAdd = NULL;
    grButtonDel = NULL;
    grButtonUp = NULL;
    grButtonDown = NULL;
    grButtonSettings = NULL;
    grButtonDelScr1 = NULL;
    grButtonDelScr2 = NULL;
}

GuiGameSelect::~GuiGameSelect()
{
    if( title_list ) {
        free(title_list);
    }
}

