/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/

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
#include "../GuiDialogs/GuiDlgGameFileSelect.h"
#include "../GuiDialogs/GuiDlgInputLine.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmBackground.h"
#include "../GuiElements/GuiElmButton.h"
#include "../GuiElements/GuiElmSelectionList.h"

GuiElmGameSelectControl::GuiElmGameSelectControl(GuiElement *_parent, const char *name) :
                         GuiElement(_parent, name)
{
    parent = static_cast<GuiDlgGameSelect*>(_parent);

    grWinControlsEdit = new GuiElmFrame(this, "controlframe", FRAMETYPE_BLUE, 0, 0, 280, 48);
    AddTop(grWinControlsEdit);
    grButtonAdd = new GuiElmButton(this, "add");
    grButtonAdd->CreateImageSelectorButton(g_imgAdd);
    grButtonAdd->SetPosition(352-336, 2);
    AddTop(grButtonAdd);
    grButtonDel = new GuiElmButton(this, "del");
    grButtonDel->CreateImageSelectorButton(g_imgDelete);
    grButtonDel->SetPosition(402-336, 2);
    AddTop(grButtonDel);
    grButtonUp = new GuiElmButton(this, "up");
    grButtonUp->CreateImageSelectorButton(g_imgUp);
    grButtonUp->SetPosition(452-336, 2);
    AddTop(grButtonUp);
    grButtonDown = new GuiElmButton(this, "down");
    grButtonDown->CreateImageSelectorButton(g_imgDown);
    grButtonDown->SetPosition(502-336, 2);
    AddTop(grButtonDown);
    grButtonSettings = new GuiElmButton(this, "settings");
    grButtonSettings->CreateImageSelectorButton(g_imgSettings);
    grButtonSettings->SetPosition(552-336, 2);
    AddTop(grButtonSettings);
}

GuiElmGameSelectControl::~GuiElmGameSelectControl()
{
    RemoveAndDelete(grButtonAdd);
    RemoveAndDelete(grButtonDel);
    RemoveAndDelete(grButtonUp);
    RemoveAndDelete(grButtonDown);
    RemoveAndDelete(grButtonSettings);
    RemoveAndDelete(grWinControlsEdit);
}

bool GuiElmGameSelectControl::OnKey(GuiDialog *dlg, BTN key, bool pressed)
{
    bool handled = false;
    GuiElement *elm = GetFocusElement();
    int selected_game = ((GuiDlgGameSelect *)dlg)->list->GetSelectedItem();

    if( pressed ) {
        switch( key ) {
            case BTN_JOY1_WIIMOTE_A:
            case BTN_JOY2_WIIMOTE_A:
            case BTN_JOY1_CLASSIC_A:
            case BTN_JOY2_CLASSIC_A:
            case BTN_RETURN:
            case BTN_SPACE:
                if( elm == grButtonDel ) {
                    bool ok = GuiDlgMessageBox::ShowModal(parent, "wantdelete",
                                                          MSGT_YESNO, NULL, 0.75f,
                                                          parent->effectDefault, parent->effectDefault,
                                                          "Do you want to delete\n\"%s\"", parent->games.GetGame(selected_game)->GetName()) == MSGBTN_YES;
                    if( ok ) {
                        parent->games.DeleteItem(selected_game);
                        parent->num_games--;
                        parent->UpdateList();
                    }
                    handled = true;
                    break;
                }
                if( elm == grButtonAdd ) {
                    parent->update_screenshots = false; // prevent updating screenshot images
                    GuiDlgGameFileSelect *filesel = new GuiDlgGameFileSelect(parent, "filesel", ".");
                    if( filesel->Create() ) {
                        parent->AddTop(filesel, GuiEffectFade(10));
                        (void)filesel->DoModal();
                        parent->Remove(filesel, GuiEffectFade(10));
                    }
                    Delete(filesel);
                    parent->update_screenshots = true;
                    handled = true;
                    break;
                }
                if( elm == grButtonUp ) {
                    parent->games.MoveUp(selected_game);
                    parent->UpdateList();
                    handled = true;
                    break;
                }
                if( elm == grButtonDown ) {
                    parent->games.MoveDown(selected_game);
                    parent->UpdateList();
                    handled = true;
                    break;
                }
                if( elm == grButtonSettings ) {
                    parent->update_screenshots = false; // prevent updating screenshot images
                    GuiDlgInputLine *input = new GuiDlgInputLine(parent, "input");
                    parent->AddTop(input, GuiEffectFade(10));
                    (void)input->DoModal();
                    parent->RemoveAndDelete(input, GuiEffectFade(10));
                    parent->update_screenshots = true;
                    handled = true;
                    break;
                }
                break;
            default:
                break;
        }
    }
    if( !handled ) {
        handled = GuiElement::OnKey(dlg, key, pressed);
    }
    return handled;
}

GuiDlgGameSelect::GuiDlgGameSelect(GuiContainer *parent, const char *name, GuiElmBackground *bgr) :
                  GuiDialog(parent, name),
                  elmControl(this, "control"),
                  effectDefault(10, 0, true),
                  effectRestart(30, 0),
                  effectScreenshot(10, 0)
{
    background = bgr;

    update_screenshots = false;
    editMode = false;
    last_index = 0;
    last_selected = -1;
    games_crc = 0;
    games_filename = NULL;
    title_list = NULL;
    sprScreenShotNormal[0] = NULL;
    sprScreenShotNormal[1] = NULL;
    sprScreenShotEdit[0] = NULL;
    sprScreenShotEdit[1] = NULL;

    // Containers (global)
    grWinList = new GuiElmFrame(this, "listframe", FRAMETYPE_BLUE, 32-8, 22, 288, 396);
    AddTop(grWinList);
    // Containers (edit mode)
    containerTitleEdit = new GuiContainer(this, "title_edit", 336+12, 22+10, 220, 147);
    containerPlayEdit = new GuiContainer(this, "play_edit", 336+12, 196+10, 220, 147);
    elmControl.SetPosition(336, 370);
    elmControl.SetWidth(280);
    elmControl.SetHeight(48);
    grWinTitleEdit = new GuiElmFrame(this, "screen1frame", FRAMETYPE_BLUE, 336, 22, 280, 167);
    grWinTitleEdit->SetVisible(false);
    AddTop(grWinTitleEdit);
    grWinPlayEdit = new GuiElmFrame(this, "screen2frame", FRAMETYPE_BLUE, 336, 196, 280, 167);
    grWinPlayEdit->SetVisible(false);
    AddTop(grWinPlayEdit);
    containerTitleEdit->SetVisible(false);
    AddTop(containerTitleEdit);
    containerPlayEdit->SetVisible(false);
    AddTop(containerPlayEdit);
    elmControl.SetVisible(false);
    AddTop(&elmControl);
    // Containers (normal mode)
    containerTitleNormal = new GuiContainer(this, "title_normal", 336+12, 22+12, 252, 168);
    containerPlayNormal = new GuiContainer(this, "play_normal", 336+12, 226+12, 252, 168);
    grWinTitleNormal = new GuiElmFrame(this, "screen1frame", FRAMETYPE_BLUE, 336, 22, 280, 192);
    AddTop(grWinTitleNormal);
    grWinPlayNormal = new GuiElmFrame(this, "screen2frame", FRAMETYPE_BLUE, 336, 226, 280, 192);
    AddTop(grWinPlayNormal);
    AddTop(containerTitleNormal);
    AddTop(containerPlayNormal);
    // Selection list
    list = new GuiElmSelectionList(this, name, NUM_LIST_ITEMS);
    AddTop(list);
    // Buttons
    grButtonDelScr1 = new GuiElmButton(this, "delscr1");
    grButtonDelScr1->CreateImageSelectorButton(g_imgDelete2);
    grButtonDelScr1->SetPosition(569, 22+10);
    grButtonDelScr1->SetVisible(false);
    AddTop(grButtonDelScr1);
    grButtonDelScr2 = new GuiElmButton(this, "delscr2");
    grButtonDelScr2->CreateImageSelectorButton(g_imgDelete2);
    grButtonDelScr2->SetPosition(569, 196+10);
    grButtonDelScr2->SetVisible(false);
    AddTop(grButtonDelScr2);
}

GuiDlgGameSelect::~GuiDlgGameSelect()
{
    if( sprScreenShotNormal[0] != NULL ) {
        containerTitleEdit->RemoveAndDelete(sprScreenShotEdit[0]);
        containerTitleNormal->RemoveAndDelete(sprScreenShotNormal[0]);
        containerPlayEdit->RemoveAndDelete(sprScreenShotEdit[1]);
        containerPlayNormal->RemoveAndDelete(sprScreenShotNormal[1]);
    }
    Remove(&elmControl);
    RemoveAndDelete(grWinList);
    RemoveAndDelete(grWinTitleNormal);
    RemoveAndDelete(grWinTitleEdit);
    RemoveAndDelete(grWinPlayNormal);
    RemoveAndDelete(grWinPlayEdit);
    RemoveAndDelete(grButtonDelScr1);
    RemoveAndDelete(grButtonDelScr2);
    RemoveAndDelete(containerTitleNormal);
    RemoveAndDelete(containerPlayNormal);
    RemoveAndDelete(containerTitleEdit);
    RemoveAndDelete(containerPlayEdit);

    if( title_list ) {
        free(title_list);
    }
    if( games_filename ) {
        free(games_filename);
    }

    RemoveAndDelete(list);
}

void GuiDlgGameSelect::SetSelectedGame(int selected)
{
    bool first = false;
    last_selected = selected;

    // (Re)create screenshot sprites
    if( sprScreenShotNormal[0] == NULL ) {
        first = true;
    }else{
        containerTitleEdit->RemoveAndDelete(sprScreenShotEdit[0], effectScreenshot);
        containerTitleNormal->RemoveAndDelete(sprScreenShotNormal[0], effectScreenshot);
        containerPlayEdit->RemoveAndDelete(sprScreenShotEdit[1], effectScreenshot);
        containerPlayNormal->RemoveAndDelete(sprScreenShotNormal[1], effectScreenshot);
    }
    sprScreenShotNormal[0] = new GuiSprite(this, "screen1");
    sprScreenShotNormal[1] = new GuiSprite(this, "screen2");
    sprScreenShotEdit[0] = new GuiSprite(this, "screen1");
    sprScreenShotEdit[1] = new GuiSprite(this, "screen2");
    // Update screenshots
    if( selected >= 0 ) {
        GameElement *game = games.GetGame(last_index+selected);
        for(int i = 0; i < 2; i++) {
            GuiImage *img = game->GetImage(i);
            if( img == NULL ) {
                img = g_imgNoise;
            }
            sprScreenShotNormal[i]->SetImage(img);
            sprScreenShotNormal[i]->SetScaledWidth(252.0f);
            sprScreenShotNormal[i]->SetScaledHeight(168.0f);
            sprScreenShotEdit[i]->SetImage(img);
            sprScreenShotEdit[i]->SetScaledWidth(220.0f);
            sprScreenShotEdit[i]->SetScaledHeight(147.0f);
        }
    }
    // Add to screen
    if( first ) {
        containerTitleNormal->AddTop(sprScreenShotNormal[0]);
        containerPlayNormal->AddTop(sprScreenShotNormal[1]);
        containerTitleEdit->AddTop(sprScreenShotEdit[0]);
        containerPlayEdit->AddTop(sprScreenShotEdit[1]);
    }else{
        containerTitleNormal->AddTop(sprScreenShotNormal[0], effectScreenshot);
        containerPlayNormal->AddTop(sprScreenShotNormal[1], effectScreenshot);
        containerTitleEdit->AddTop(sprScreenShotEdit[0], effectScreenshot);
        containerPlayEdit->AddTop(sprScreenShotEdit[1], effectScreenshot);
    }
    // Free images of games that are not on the screen
    for(int i = 0; i < games.GetNumberOfGames(); i++) {
        GameElement *game = games.GetGame(i);
        if( game != NULL && (i < last_index || i >= (last_index + NUM_LIST_ITEMS)) ) {
            game->FreeImage(0);
            game->FreeImage(1);
        }
    }
}

void GuiDlgGameSelect::OnUpdateScreen(void)
{
    if( update_screenshots ) {
        int sel = list->GetSelectedItem();
        if( sel >= 0 && sel != last_selected ) {
            SetSelectedGame(sel);
        }
    }
}

bool GuiDlgGameSelect::OnKey(GuiDialog *dlg, BTN key, bool pressed)
{
    bool handled = false;
    GuiElement *elm = GetFocusElement();
    int selected_game = list->GetSelectedItem();

    if( pressed ) {
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
                    bool ok = GuiDlgMessageBox::ShowModal(this, "wantstart",
                                                          MSGT_YESNO, NULL, 0.75f,
                                                          effectDefault, effectDefault,
                                                          "Do you want to start\n\"%s\"", game->GetName()) == MSGBTN_YES;
                    if( ok ) {
                        Leave(game);
                    }
                    handled = true;
                }
                if( elm == grButtonDelScr1 ) {
                    games.GetGame(selected_game)->DeleteImage(0);
                    UpdateList();
                    handled = true;
                }
                if( elm == grButtonDelScr2 ) {
                    games.GetGame(selected_game)->DeleteImage(1);
                    UpdateList();
                    handled = true;
                }
                break;
            case BTN_JOY1_WIIMOTE_PLUS:
            case BTN_JOY2_WIIMOTE_PLUS:
            case BTN_F11:
                editMode = !editMode;
                ShowElements();
                handled = true;
                break;
            case BTN_JOY1_WIIMOTE_B:
            case BTN_JOY2_WIIMOTE_B:
            case BTN_JOY1_CLASSIC_B:
            case BTN_JOY2_CLASSIC_B:
            case BTN_ESCAPE:
                handled = true;
                Leave(NULL);
                break;
            default:
                break;
        }
    }
    if( !handled ) {
        handled = GuiDialog::OnKey(dlg, key, pressed);
    }
    return handled;
}

bool GuiDlgGameSelect::Load(const char *dir, const char *filename, GameElement *select)
{
    // Load games database
    games_filename = (char*)malloc(256);
    strcpy(games_filename, dir);
    strcat(games_filename, "/");
    strcat(games_filename, filename);
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
    GXColor white = {255, 255, 255, 255};
    list->InitSelection(new GuiElmListLineDefault(this, "defaultline", white, 22, 12, false),
                        (void**)title_list, num_games, sel, 31,
                        30, 30, 276);

    update_screenshots = true;

    return true;
}

void GuiDlgGameSelect::UpdateList(void)
{
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }
    list->SetNumberOfItems(num_games);
    list->UpdateList();
}

void GuiDlgGameSelect::ShowElements(void)
{
    // GUI Elements
    if( editMode ) {
        Show(&elmControl, effectRestart, false, 0, elmControl.GetHeight(), 1.0f, 0.0f);
        Morph(containerTitleNormal, containerTitleEdit, effectRestart, effectRestart);
        Morph(containerPlayNormal, containerPlayEdit, effectRestart, effectRestart);
        Morph(grWinTitleNormal, grWinTitleEdit, effectRestart, effectRestart);
        Morph(grWinPlayNormal, grWinPlayEdit, effectRestart, effectRestart);
        Show(grButtonDelScr1, effectRestart, false, 25, 5, 0.0f, 0.0f);
        Show(grButtonDelScr2, effectRestart, false, 25, 30, 0.0f, 0.0f);
    }else{
        Hide(&elmControl, effectRestart, false, 0, elmControl.GetHeight(), 1.0f, 0.0f);
        Morph(containerTitleEdit, containerTitleNormal, effectRestart, effectRestart);
        Morph(containerPlayEdit, containerPlayNormal, effectRestart, effectRestart);
        Morph(grWinTitleEdit, grWinTitleNormal, effectRestart, effectRestart);
        Morph(grWinPlayEdit, grWinPlayNormal, effectRestart, effectRestart);
        Hide(grButtonDelScr1, effectRestart, false, 25, 5, 0.0f, 0.0f);
        Hide(grButtonDelScr2, effectRestart, false, 25, 30, 0.0f, 0.0f);
    }
}

GameElement *GuiDlgGameSelect::DoModal(void)
{
    GameElement *returnValue = NULL;

    background->HideVersion();

    // Outer loop
    bool restart;
    do {
        restart = false;

        // Run GUI
        list->SetFocus(true);
        returnValue = (GameElement*)Run(false);

        if( !restart && (games_crc != games.CalcCRC()) ) {
            // Gamelist changed, ask to save
            MSGBTN btn = GuiDlgMessageBox::ShowModal(this, "savechanges",
                                                     MSGT_YESNOCANCEL, NULL, 0.75f, effectDefault,
                                                     effectDefault, "Save changes?");
            if( btn == MSGBTN_YES ) {
                games.Save(games_filename);
                (void)GuiDlgMessageBox::ShowModal(this, "changessaved",
                                                  MSGT_OK, NULL, 0.75f, effectDefault,
                                                  effectDefault, "Changes saved");
            }
            if( btn == MSGBTN_CANCEL ) {
                restart = true;
            }
        }

        // Update UI elements
        if( restart ) {
            ShowElements();
        }
    }while( restart );

    if( returnValue != NULL ) {
        GameElement *game = new GameElement(returnValue);
        games.Clear();
        background->ShowVersion();
        return game;
    }else{
        games.Clear();
        background->ShowVersion();
        return NULL;
    }
}

