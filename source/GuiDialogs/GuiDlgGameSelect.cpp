
#include "GuiDlgGameSelect.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef WII
#include <direct.h>
#endif

#include "GuiDlgMessageBox.h"
#include "../Gui/GameElement.h"
#include "../Gui/GuiImages.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiEffectZoom.h"
#include "../GuiLayers/GuiLayFrame.h"
#include "../GuiElements/GuiElmBackground.h"
#include "../GuiElements/GuiElmButton.h"
#include "../GuiElements/GuiElmSelectionList.h"

#define GAMESEL_EFFECT_DEFAULT     new GuiEffectZoom(10)
#define GAMESEL_EFFECT_RESTART     new GuiEffectFade(30, 6)
#define GAMESEL_EFFECT_SCREENSHOTS new GuiEffectFade(10, 8)

void GuiDlgGameSelect::SetSelectedGame(int index, int selected, bool restart)
{
    bool first = true;

    last_index = index;
    last_selected = selected;

    // (Re)create screenshot sprites
    if( sprScreenShot[0] != NULL ) {
        first = false;
        RemoveAndDelete(sprScreenShot[0], GAMESEL_EFFECT_SCREENSHOTS);
    }
    if( sprScreenShot[1] != NULL ) {
        first = false;
        RemoveAndDelete(sprScreenShot[1], GAMESEL_EFFECT_SCREENSHOTS);
    }
    sprScreenShot[0] = new GuiSprite;
    sprScreenShot[0]->SetPosition(344+12-8, screenshotYpos1);
    RegisterForDelete(sprScreenShot[0]);
    sprScreenShot[1] = new GuiSprite;
    sprScreenShot[1]->SetPosition(344+12-8, screenshotYpos2);
    RegisterForDelete(sprScreenShot[1]);
    // Update screenshots
    if( selected >= 0 ) {
        GameElement *game = games.GetGame(index+selected);
        for(int i = 0; i < 2; i++) {
            GuiSprite *spr = sprScreenShot[i];
            GuiImage *img = game->GetImage(i);
            if( img == NULL ) {
                img = g_imgNoise;
            }
            spr->SetImage(img);
            spr->SetRefPixelPosition(0, 0);
            spr->SetStretchWidth(screenshotWidth / img->GetWidth());
            spr->SetStretchHeight(screenshotHeigth / img->GetHeight());
        }
    }
    // Add to screen
    AddOnTopOf(grWinTitle, sprScreenShot[0], restart? GAMESEL_EFFECT_RESTART : NULL);
    AddOnTopOf(grWinPlay, sprScreenShot[1], restart? GAMESEL_EFFECT_RESTART : NULL);
    // Free images of games that are not on the screen
    for(int i = 0; i < games.GetNumberOfGames(); i++) {
        GameElement *game = games.GetGame(i);
        if( game != NULL && (i < index || i >= (index + NUM_LIST_ITEMS)) ) {
            game->FreeImage(0);
            game->FreeImage(1);
        }
    }
}

void GuiDlgGameSelect::OnUpdateScreen(void)
{
    int sel = list->GetSelectedItem();
    if( sel >= 0 && sel != last_selected ) {
        SetSelectedGame(last_index, sel, false);
    }
}

void GuiDlgGameSelect::OnKey(BTN key, bool pressed)
{
    GuiElement *elm = GetSelected();
    int selected_game = list->GetSelectedItem();

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
                GameElement *game = games.GetGame(selected_game);
                bool ok = GuiDlgMessageBox::ShowModal(this, MSGT_YESNO, NULL, 192, GAMESEL_EFFECT_DEFAULT, GAMESEL_EFFECT_DEFAULT,
                                                   "Do you want to start\n\"%s\"", game->GetName()) == MSGBTN_YES;
                if( ok ) {
                    Leave(game);
                }
            }
            if( elm == grButtonDel ) {
                bool ok = GuiDlgMessageBox::ShowModal(this, MSGT_YESNO, NULL, 192, GAMESEL_EFFECT_DEFAULT, GAMESEL_EFFECT_DEFAULT,
                                                   "Do you want to delete\n\"%s\"", games.GetGame(selected_game)->GetName()) == MSGBTN_YES;
                if( ok ) {
                    games.DeleteItem(selected_game);
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
            Leave(NULL);
            break;
        default:
            break;
    }
}

bool GuiDlgGameSelect::Load(const char *dir, const char *filename, GameElement *select)
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

    // Init selection list
    list->InitSelection(title_list, num_games, sel, 22, 31,
                        30, 38, 12, 264+12, false);

    Show(false);

    return true;
}

void GuiDlgGameSelect::UpdateList(void)
{
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }
    list->SetNumberOfItems(num_games);
    list->ClearTitleList();
    list->SetSelectedItem();
}

void GuiDlgGameSelect::Show(bool restart)
{
    #define EFFECT restart? GAMESEL_EFFECT_RESTART : NULL

    // Remove version from background
    background->HideVersion(EFFECT);
    
    // Add selection list
    if( grWinList == NULL ) {
        grWinList = new GuiLayFrame(32-8, 28, 288, 33*12);
        RegisterForDelete(grWinList);
        AddTop(grWinList, EFFECT);
        AddTop(list, EFFECT);
    }
    
    // GUI Elements
    if( editMode ) {
        // Containers
        grWinTitle = new GuiLayFrame(344-8, 28, 264+12, 14*12);
        RegisterForDelete(grWinTitle);
        AddTop(grWinTitle, EFFECT);
        grWinPlay = new GuiLayFrame(344-8, 232-30, 264+12, 14*12);
        RegisterForDelete(grWinPlay);
        AddTop(grWinPlay, EFFECT);
        grWinControls = new GuiLayFrame(344-8, 232-30+14*12+6, 264+12, 4*12);
        RegisterForDelete(grWinControls);
        AddTop(grWinControls, EFFECT);
        // Icons
        grButtonAdd = new GuiElmButton();
        grButtonAdd->CreateImageSelectorButton(g_imgAdd);
        grButtonAdd->SetPosition(344+14, 232-30+14*12+6+8);
        RegisterForDelete(grButtonAdd);
        AddTop(grButtonAdd, EFFECT);
        grButtonDel = new GuiElmButton();
        grButtonDel->CreateImageSelectorButton(g_imgDelete);
        grButtonDel->SetPosition(344+14+50, 232-30+14*12+6+8);
        RegisterForDelete(grButtonDel);
        AddTop(grButtonDel, EFFECT);
        grButtonUp = new GuiElmButton();
        grButtonUp->CreateImageSelectorButton(g_imgUp);
        grButtonUp->SetPosition(344+14+2*50, 232-30+14*12+6+8);
        RegisterForDelete(grButtonUp);
        AddTop(grButtonUp, EFFECT);
        grButtonDown = new GuiElmButton();
        grButtonDown->CreateImageSelectorButton(g_imgDown);
        grButtonDown->SetPosition(344+14+3*50, 232-30+14*12+6+8);
        RegisterForDelete(grButtonDown);
        AddTop(grButtonDown, EFFECT);
        grButtonSettings = new GuiElmButton();
        grButtonSettings->CreateImageSelectorButton(g_imgSettings);
        grButtonSettings->SetPosition(344+14+4*50, 232-30+14*12+6+8);
        RegisterForDelete(grButtonSettings);
        AddTop(grButtonSettings, EFFECT);
        grButtonDelScr1 = new GuiElmButton();
        grButtonDelScr1->CreateImageSelectorButton(g_imgDelete2);
        grButtonDelScr1->SetPosition(344+8+264+12-54, 28+16);
        RegisterForDelete(grButtonDelScr1);
        AddTop(grButtonDelScr1, EFFECT);
        grButtonDelScr2 = new GuiElmButton();
        grButtonDelScr2->CreateImageSelectorButton(g_imgDelete2);
        grButtonDelScr2->SetPosition(344+8+264+12-54, 232-30+16);
        RegisterForDelete(grButtonDelScr2);
        AddTop(grButtonDelScr2, EFFECT);
        // Screenshot coordinates
        screenshotWidth = (252.0f/16.0f)*14.0f;
        screenshotHeigth = (168.0f/16.0f)*14.0f;
        screenshotYpos1 = 24+16;
        screenshotYpos2 = 228+16-30;
    }else{
        // Containers
        grWinTitle = new GuiLayFrame(344-8, 28, 264+12, 16*12);
        RegisterForDelete(grWinTitle);
        AddTop(grWinTitle, EFFECT);
        grWinPlay = new GuiLayFrame(344-8, 232, 264+12, 16*12);
        RegisterForDelete(grWinPlay);
        AddTop(grWinPlay, EFFECT);
        // Screenshot coordinates
        screenshotWidth = 252.0f;
        screenshotHeigth = 168.0f;
        screenshotYpos1 = 24+16;
        screenshotYpos2 = 228+16;
    }
    SetSelectedGame(last_index, last_selected, restart);
    is_showing = true;
}

void GuiDlgGameSelect::Hide(bool restart)
{
    #define EFFECT restart? GAMESEL_EFFECT_RESTART : NULL

    if( !is_showing ) {
        return;
    }
    if( !restart ) {
        if( grWinList != NULL ) {
            RemoveAndDelete(grWinList, EFFECT);
            grWinList = NULL;
        }
        Remove(list, EFFECT);
    }
    if( sprScreenShot[0] != NULL ) {
        RemoveAndDelete(sprScreenShot[0], EFFECT);
        sprScreenShot[0] = NULL;
        RemoveAndDelete(sprScreenShot[1], EFFECT);
        sprScreenShot[1] = NULL;
    }
    RemoveAndDelete(grWinTitle, EFFECT);
    RemoveAndDelete(grWinPlay, EFFECT);
    if( grWinControls != NULL ) {
        RemoveAndDelete(grWinControls, EFFECT);
    }
    grWinTitle = grWinPlay = grWinControls = NULL;
    if( grButtonAdd != NULL ) {
        RemoveAndDelete(grButtonAdd, EFFECT);
        RemoveAndDelete(grButtonDel, EFFECT);
        RemoveAndDelete(grButtonUp, EFFECT);
        RemoveAndDelete(grButtonDown, EFFECT);
        RemoveAndDelete(grButtonSettings, EFFECT);
        RemoveAndDelete(grButtonDelScr1, EFFECT);
        RemoveAndDelete(grButtonDelScr2, EFFECT);
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
        background->ShowVersion(EFFECT);
    }
    is_showing = false;
}

GameElement *GuiDlgGameSelect::DoModal(void)
{
    GameElement *returnValue = NULL;

    // Outer loop
    bool restart;
    do {
        restart = false;

        // Run GUI
        SetSelected(list);
        returnValue = (GameElement*)Run();

        if( !restart && (games_crc != games.CalcCRC()) ) {
            // Gamelist changed, ask to save
            MSGBTN btn = GuiDlgMessageBox::ShowModal(this, MSGT_YESNOCANCEL, NULL, 192, GAMESEL_EFFECT_DEFAULT,
                                                     GAMESEL_EFFECT_DEFAULT, "Save changes?");
            if( btn == MSGBTN_YES ) {
                games.Save(games_filename);
                (void)GuiDlgMessageBox::ShowModal(this, MSGT_OK, NULL, 192, GAMESEL_EFFECT_DEFAULT,
                                                  GAMESEL_EFFECT_DEFAULT, "Changes saved");
            }
            if( btn == MSGBTN_CANCEL ) {
                restart = true;
            }
        }

        // Remove UI elements
        if( restart ) {
            Hide(true);
            Show(false);
        }
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

GuiDlgGameSelect::GuiDlgGameSelect(GuiContainer *cntr, GuiElmBackground *bgr)
                 :GuiDialog(cntr)
{
    list = new GuiElmSelectionList(this, NUM_LIST_ITEMS);
    RegisterForDelete(list);

    background = bgr;

    is_showing = false;
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

GuiDlgGameSelect::~GuiDlgGameSelect()
{
    Hide(false);
    if( title_list ) {
        free(title_list);
    }
    if( games_filename ) {
        free(games_filename);
    }
    Delete(list);
}

