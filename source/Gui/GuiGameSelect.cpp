#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef WII
#include <direct.h>
#endif

#include "GuiRunner.h"
#include "GuiBackground.h"
#include "GuiButton.h"
#include "GuiGameSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"
#include "GuiSelectionList.h"
#include "GameElement.h"

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

void GuiGameSelect::SetSelected(int index, int selected, bool restart)
{
    int fade_time, fade_delay;
    bool first = true;

    last_index = index;
    last_selected = selected;

    fade_time = restart? GAMESEL_FADE_RESTART : GAMESEL_FADE_DEFAULT;
    fade_delay = restart? GAMESEL_DELAY_RESTART : 0;

    // (Re)create screenshot sprites
    if( sprScreenShot[0] != NULL ) {
        first = false;
        fade_time = GAMESEL_FADE_SCREENSHOTS;
        fade_delay = GAMESEL_DELAY_SCREENSHOTS;
        manager->RemoveAndDelete(sprScreenShot[0], sprScreenShot[0]->GetImage(),
                                 fade_time, fade_delay);
    }
    if( sprScreenShot[1] != NULL ) {
        first = false;
        fade_time = GAMESEL_FADE_SCREENSHOTS;
        fade_delay = GAMESEL_DELAY_SCREENSHOTS;
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

void GuiGameSelect::OnUpdateScreen(GuiRunner *runner)
{
    int sel = list->GetSelected();
    if( sel >= 0 && sel != last_selected ) {
        SetSelected(last_index, sel, false);
    }
}

void GuiGameSelect::OnKey(GuiRunner *runner, BTN key, bool pressed)
{
    GuiElement *elm = runner->GetSelected();
    int selected_game = list->GetSelected();

    if( !pressed ) {
        return;
    }
    switch( key ) {
        case BTN_JOY1_WIIMOTE_A:
        case BTN_JOY2_WIIMOTE_A:
        case BTN_JOY1_CLASSIC_A:
        case BTN_JOY2_CLASSIC_A:
        case BTN_RETURN:
        case BTN_SPACE:
            if( elm == list && list->IsActive() ) {
                // confirmation
                char str[256];
                GameElement *game = games.GetGame(selected_game);
                strcpy(str, "Do you want to start\n\"");
                strcat(str, game->GetName());
                strcat(str, "\"");
                GuiMessageBox *msgbox = new GuiMessageBox(manager);
                bool ok = msgbox->Show(str, NULL, MSGT_YESNO, 192) == MSGBTN_YES;
                msgbox->Remove();
                delete msgbox;
                if( ok ) {
                    runner->Leave(game);
                }
            }
            if( elm == grButtonDel ) {
                char str[256];
                strcpy(str, "Do you want to delete\n\"");
                strcat(str, games.GetGame(selected_game)->GetName());
                strcat(str, "\"");
                GuiMessageBox *msgbox = new GuiMessageBox(manager);
                bool ok = msgbox->Show(str, NULL, MSGT_YESNO, 192) == MSGBTN_YES;
                msgbox->Remove();
                delete msgbox;
                if( ok ) {
                    games.Delete(selected_game);
                    num_games--;
                    UpdateList();
                }
                break;
            }
            if( elm == grButtonUp ) {
                games.MoveUp(selected_game);
                list->DoKeyUp();
                UpdateList();
                break;
            }
            if( elm == grButtonDown ) {
                games.MoveDown(selected_game);
                list->DoKeyDown();
                UpdateList();
                break;
            }
            if( elm == grButtonDelScr1 ) {
                games.GetGame(selected_game)->DeleteImage(0);
                UpdateList();
                break;
            }
            if( elm == grButtonDelScr2 ) {
                games.GetGame(selected_game)->DeleteImage(1);
                UpdateList();
            }
            break;
        case BTN_JOY1_WIIMOTE_PLUS:
        case BTN_JOY2_WIIMOTE_PLUS:
        case BTN_F11:
            Hide(true);
            editMode = !editMode;
            Show(true);
            break;
        case BTN_JOY1_WIIMOTE_B:
        case BTN_JOY2_WIIMOTE_B:
        case BTN_JOY1_CLASSIC_B:
        case BTN_JOY2_CLASSIC_B:
        case BTN_ESCAPE:
            runner->Leave(NULL);
            break;
        default:
            break;
    }
}

bool GuiGameSelect::Load(const char *dir, const char *filename)
{
    // Load games database
    games_filename = strdup(filename);
#ifdef WII
        chdir(dir);
#else
        _chdir(dir);
#endif
    games.Load(games_filename);
    num_games = games.GetNumberOfGames();
    if( num_games == 0 ) {
        return false;
    }
    title_list = (const char**)malloc(num_games * sizeof(const char*));
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }
    games_crc = games.CalcCRC();
    return true;
}

void GuiGameSelect::UpdateList(void)
{
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }
    list->SetNumberOfItems(num_games);
    list->ClearTitleList();
    list->SetSelected();
}

void GuiGameSelect::Show(bool restart)
{
    int fade_time = restart? GAMESEL_FADE_RESTART : GAMESEL_FADE_DEFAULT;

    // Claim UI
    manager->Lock();
    
    // Remove version from background
    background->HideVersion(fade_time);
    
    // Add selection list
    if( grWinList == NULL ) {
        grWinList = new GuiContainer(32-8, 28, 288, 33*12);
        manager->AddTop(grWinList, fade_time);
        runner->AddTop(list, fade_time);
    }
    
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
        grButtonAdd->CreateImageSelectorButton(g_imgAdd, 344+14, 232-30+14*12+6+8);
        runner->AddTop(grButtonAdd, fade_time);
        grButtonDel = new GuiButton(manager);
        grButtonDel->CreateImageSelectorButton(g_imgDelete, 344+14+50, 232-30+14*12+6+8);
        runner->AddTop(grButtonDel, fade_time);
        grButtonUp = new GuiButton(manager);
        grButtonUp->CreateImageSelectorButton(g_imgUp, 344+14+2*50, 232-30+14*12+6+8);
        runner->AddTop(grButtonUp, fade_time);
        grButtonDown = new GuiButton(manager);
        grButtonDown->CreateImageSelectorButton(g_imgDown, 344+14+3*50, 232-30+14*12+6+8);
        runner->AddTop(grButtonDown, fade_time);
        grButtonSettings = new GuiButton(manager);
        grButtonSettings->CreateImageSelectorButton(g_imgSettings, 344+14+4*50, 232-30+14*12+6+8);
        runner->AddTop(grButtonSettings, fade_time);
        grButtonDelScr1 = new GuiButton(manager);
        grButtonDelScr1->CreateImageSelectorButton(g_imgDelete2, 344+8+264+12-54, 28+16);
        runner->AddTop(grButtonDelScr1, fade_time);
        grButtonDelScr2 = new GuiButton(manager);
        grButtonDelScr2->CreateImageSelectorButton(g_imgDelete2, 344+8+264+12-54, 232-30+16);
        runner->AddTop(grButtonDelScr2, fade_time);
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
    SetSelected(last_index, last_selected, restart);
    
    // Release UI
    manager->Unlock();
}

void GuiGameSelect::Hide(bool restart)
{
    int fade_time, fade_delay;

    fade_time = restart? GAMESEL_FADE_RESTART : GAMESEL_FADE_DEFAULT;
    fade_delay = restart? GAMESEL_DELAY_RESTART : 0;

    // Claim UI
    manager->Lock();
    
    if( !restart ) {
        manager->RemoveAndDelete(grWinList, NULL, fade_time, fade_delay);
        grWinList = NULL;
        runner->Remove(list, fade_time, fade_delay);
    }
    if( sprScreenShot[0] != NULL ) {
        manager->RemoveAndDelete(sprScreenShot[0], sprScreenShot[0]->GetImage(), fade_time, fade_delay);
        sprScreenShot[0] = NULL;
        manager->RemoveAndDelete(sprScreenShot[1], sprScreenShot[1]->GetImage(), fade_time, fade_delay);
        sprScreenShot[1] = NULL;
    }
    manager->RemoveAndDelete(grWinTitle, NULL, fade_time, fade_delay);
    manager->RemoveAndDelete(grWinPlay, NULL, fade_time, fade_delay);
    manager->RemoveAndDelete(grWinControls, NULL, fade_time, fade_delay);
    grWinTitle = grWinPlay = grWinControls = NULL;
    if( grButtonAdd != NULL ) {
        runner->Remove(grButtonAdd, fade_time, fade_delay);
        runner->Remove(grButtonDel, fade_time, fade_delay);
        runner->Remove(grButtonUp, fade_time, fade_delay);
        runner->Remove(grButtonDown, fade_time, fade_delay);
        runner->Remove(grButtonSettings, fade_time, fade_delay);
        runner->Remove(grButtonDelScr1, fade_time, fade_delay);
        runner->Remove(grButtonDelScr2, fade_time, fade_delay);
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
    if( !restart ) {
        background->ShowVersion(fade_time);
    }
    
    // Release UI
    manager->Unlock();
}

GameElement *GuiGameSelect::DoModal(GameElement *select)
{
    GameElement *returnValue = NULL;
    int selected = 0;

    // On re-enter, find selected entry
    if( select != NULL ) {
        for( int i = 0; i < num_games; i++ ) {
            if( strcmp(select->GetName(), title_list[i])==0 ) {
                selected = i;
                break;
            }
        }
    }

    // Init selection list
    list->InitSelection(title_list, num_games, selected, 22, 31,
                        30, 38, 12, 264+12, false);

    // Outer loop
    bool restart;
    do {
        restart = false;
        Show(false);

        // Run GUI
        runner->SetSelected(list);
        returnValue = (GameElement*)runner->Run();

        if( !restart && (games_crc != games.CalcCRC()) ) {
            // Gamelist changed, ask to save
            GuiMessageBox *msgbox = new GuiMessageBox(manager);
            MSGBTN btn = msgbox->Show("Save changes?", NULL, MSGT_YESNOCANCEL, 192);
            msgbox->Remove();
            if( btn == MSGBTN_YES ) {
                games.Save(games_filename);
                (void)msgbox->Show("Changes saved", NULL, MSGT_OK, 192);
                msgbox->Remove();
            }
            if( btn == MSGBTN_CANCEL ) {
                restart = true;
            }
            delete msgbox;
        }

        // Remove UI elements
        Hide(restart);
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

GuiGameSelect::GuiGameSelect(GuiManager *man, GuiBackground *bgr)
{
    runner = new GuiRunner(man, this);
    list = new GuiSelectionList(man, NUM_LIST_ITEMS);
    manager = man;
    background = bgr;
    editMode = false;
    last_index = 0;
    last_selected = 0;
    games_crc = 0;
    games_filename = NULL;
    title_list = NULL;
    sprScreenShot[0] = NULL;
    sprScreenShot[1] = NULL;

    grWinList = NULL;
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
    if( games_filename ) {
        free(games_filename);
    }
    delete list;
    delete runner;
}

