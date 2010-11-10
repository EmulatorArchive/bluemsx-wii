
#include "GuiElmSelectionList.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>

#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiSprite.h"

#include "../GuiLayers/GuiLayFrame.h"
#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

#define SELECTION_FADE_TIME  10
#define SELECTION_FADE_DELAY 4
#define SELECTION_EFFECT new GuiEffectFade(SELECTION_FADE_TIME, SELECTION_FADE_DELAY)

//-----------------------

bool GuiElmSelectionList::ElmSetSelectedOnCollision(GuiSprite *sprite)
{
    Lock();
    for(int i = upper_index; i <= lower_index; i++) {
        if( strlen(visible_items[i]) &&
            sprite->CollidesWith(titleTxtSprite[i]) ) {
            if( i != selected ) {
                selected = i;
                SetSelectedItem();
            }
            Unlock();
            return true;
        }
    }
    Unlock();
    return false;
}

void GuiElmSelectionList::ElmSetSelected(bool sel, int x, int y)
{
    Lock();
    if( sel ) {
        int s = upper_index;
        for(int i = upper_index; i <= lower_index; i++) {
            int yy = titleTxtSprite[i]->GetY() + titleTxtSprite[i]->GetHeight() / 2;
            if( yy > y ) {
                break;
            }
            s = i;
        }
        selected = s;
        SetSelectedItem();
    }else{
        // We're never deselected, just inactive
        is_active = false;
    }
    Unlock();
}

bool GuiElmSelectionList::ElmGetRegion(int *px, int *py, int *pw, int *ph)
{
    if( is_showing ) {
        *px = xpos;
        *py = ypos;
        *pw = xsize;
        *ph = ypitch*num_item_rows;
        return true;
    }else{
        return false;
    }
}

bool GuiElmSelectionList::ElmHandleKey(GuiDialog *dlg, BTN key, bool pressed)
{
    if (dlg->GetSelected(false) == NULL && selected >= 0) { /* When nothing selected, we're in charge */
        dlg->SetSelected(this, 0, titleTxtSprite[selected]->GetY() + titleTxtSprite[selected]->GetHeight() / 2);
    }
    if( pressed &&
        dlg->GetSelected(false) == this )
    {
        switch( key ) {
            case BTN_UP:
            case BTN_JOY1_UP:
            case BTN_JOY2_UP:
                DoKeyUp();
                return true;
            case BTN_DOWN:
            case BTN_JOY1_DOWN:
            case BTN_JOY2_DOWN:
                DoKeyDown();
                return true;
            default:
                break;
        }
    }
    return false;
}

//-----------------------

void GuiElmSelectionList::DoKeyUp(void)
{
    if( selected > upper_index ) {
        selected--;
        SetSelectedItem();
    }else{
        if( index > 0 ) {
            index--;
            SetSelectedItem();
        }
    }
}

void GuiElmSelectionList::DoKeyDown(void)
{
    if( selected < lower_index &&
        strlen(visible_items[selected+1]) ) {
        selected++;
        SetSelectedItem();
    }else{
        if( index+selected < num_items-1 ) {
            index++;
            SetSelectedItem();
        }
    }
}

void GuiElmSelectionList::CleanUp(void)
{
    Lock();
    if( is_showing ) {
        // Titles
        for(int i = 0; i < num_item_rows; i++) {
            RemoveAndDelete(titleTxtSprite[i], SELECTION_EFFECT);
            titleTxtSprite[i] = NULL;
        }

        // Arrows
        RemoveAndDelete(sprArrowUp, SELECTION_EFFECT);
        sprArrowUp = NULL;
        RemoveAndDelete(sprArrowDown, SELECTION_EFFECT);
        sprArrowDown = NULL;

        // Title list
        RemoveTitleList(0, 0);
        RemoveAndDelete(sprSelector, SELECTION_EFFECT);
        sprSelector = NULL;

        is_showing = false;
    }
    Unlock();
}

void GuiElmSelectionList::RemoveTitleList(int fade, int delay)
{
}

void GuiElmSelectionList::ClearTitleList(void)
{
    current_index = -1;
}

int GuiElmSelectionList::GetSelectedItem(void)
{
    return selected;
}

void GuiElmSelectionList::SetNumberOfItems(int num)
{
    num_items = num;
}

bool GuiElmSelectionList::IsShowing(void)
{
    return is_showing;
}

int GuiElmSelectionList::IsActive(void)
{
    return is_showing && is_active;
}

void GuiElmSelectionList::SetSelectedItem(int fade, int delay)
{
    GXColor white = {255,255,255,255};
    Lock();
    // Update dir info
    for(int i = 0; i < num_item_rows; i++) {
        if( i+index < num_items ) {
            visible_items[i] = item_list[i+index];
        }else{
            visible_items[i] = "";
        }
    }
    // Render text
    if( index == current_index+1 && current_index != -1 ) {
        // move list up
        RemoveAndDelete(titleTxtSprite[0]);
        for(int i = 0, yy = ypos; i < num_item_rows-1; i++, yy += ypitch) {
            titleTxtSprite[i] = titleTxtSprite[i+1];
            titleTxtSprite[i]->SetPosition(xpos + xspacing, yy + fontsize/3);
            titleTxtSprite[i]->SetVisible(true);
        }
        GuiSprite* spr = new GuiSprite();
        spr->CreateTextImage(g_fontArial, fontsize, xsize-2*xspacing, 0, center, white, visible_items[num_item_rows-1]);
        spr->SetPosition(xpos + xspacing, ypos + (num_item_rows-1)*ypitch + fontsize/3);
        RegisterForDelete(spr);
        AddOnTopOf(titleTxtSprite[num_item_rows-1], spr);
        titleTxtSprite[num_item_rows-1] = spr;
    }else
    if( index == current_index-1 ) {
        // move list down
        if( titleTxtSprite[num_item_rows-1] ) {
            RemoveAndDelete(titleTxtSprite[num_item_rows-1]);
        }
        for(int i = num_item_rows-1, yy = ypos+(num_item_rows-1)*ypitch; i > 0; i--, yy -= ypitch) {
            titleTxtSprite[i] = titleTxtSprite[i-1];
            titleTxtSprite[i]->SetPosition(xpos + xspacing, yy + fontsize/3);
            titleTxtSprite[i]->SetVisible(true);
        }
        GuiSprite* spr = new GuiSprite();
        spr->CreateTextImage(g_fontArial, fontsize, xsize-2*xspacing, 0, center, white, visible_items[0]);
        spr->SetPosition(xpos + xspacing, ypos + fontsize/3);
        RegisterForDelete(spr);
        AddOnTopOf(titleTxtSprite[0], spr);
        titleTxtSprite[0] = spr;
    }else{
        // rebuild
        for(int i = 0, yy = ypos; i < num_item_rows; i++, yy += ypitch) {
            GuiSprite* spr = new GuiSprite();
            spr->CreateTextImage(g_fontArial, fontsize, xsize-2*xspacing, 0, center, white, visible_items[i]);
            spr->SetPosition(xpos + xspacing, yy + fontsize/3);
            spr->SetVisible(true);
            RegisterForDelete(spr);
            if( titleTxtSprite[i] ) {
                AddOnTopOf(titleTxtSprite[i], spr);
                RemoveAndDelete(titleTxtSprite[i]);
            }else{
                AddTop(spr, SELECTION_EFFECT);
            }
            titleTxtSprite[i] = spr;
        }
    }
    // Update sprite positions
//    for(int i = 0, yy = ypos; i < num_item_rows; i++) {
//        titleTxtSprite[i]->SetPosition(xpos + xspacing, yy + fontsize/3);
//        img->CreateImage((xsize - 2*xspacing + 3) & ~3, ((fontsize * 3) / 2) & ~3);
//        yy += ypitch;
//    }
    // Up button
    if( index > 0  ) {
        titleTxtSprite[0]->SetVisible(false);
        sprArrowUp->SetVisible(true);
        upper_index = 1;
    }else{
        titleTxtSprite[0]->SetVisible(true);
        sprArrowUp->SetVisible(false);
        upper_index = 0;
    }
    // Down button
    if( index+num_item_rows < num_items ) {
        titleTxtSprite[num_item_rows-1]->SetVisible(false);
        sprArrowDown->SetVisible(true);
        lower_index = num_item_rows-2;
    }else{
        titleTxtSprite[num_item_rows-1]->SetVisible(true);
        sprArrowDown->SetVisible(false);
        lower_index = num_item_rows-1;
    }
    // Update seletion
    if( sprSelector != NULL ) {
        RemoveAndDelete(sprSelector, new GuiEffectFade(
                                     (fade > 0)? fade : SELECTION_FADE_TIME,
                                     (delay > 0)? delay : SELECTION_FADE_DELAY));
        sprSelector = NULL;
    }
    if( selected >= 0 ) {
        GuiSprite *selectedsprite = titleTxtSprite[selected];
        sprSelector = new GuiSprite;
        sprSelector->SetImage(g_imgSelector);
        sprSelector->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprSelector->SetRefPixelPosition(0, 0);
        sprSelector->SetPosition(selectedsprite->GetX()-xspacing,selectedsprite->GetY()-fontsize/3);
        sprSelector->SetStretchWidth((float)xsize / 4);
        sprSelector->SetStretchHeight((float)fontsize * 1.8f / 44);
        RegisterForDelete(sprSelector);
        AddBehind(selectedsprite, sprSelector, new GuiEffectFade((fade > 0)? fade : SELECTION_FADE_TIME));
        is_active = true;
    }
    current_index = index;
    Unlock();
}


void GuiElmSelectionList::InitSelection(const char **items, int num, int select, int fontsz, int pitchy,
                                     int posx, int posy, int xspace, int width, bool centr)
{
    xpos = posx;
    ypos = posy;
    xsize = width;
    xspacing = xspace;
    fontsize = fontsz;
    ypitch = pitchy;
    center = centr;

    CleanUp();

    // Init items
    item_list = items;
    num_items = num;

    // Selected initial entry
    if( select ) {
        if( select < num_item_rows-1 ) {
            index = 0;
            selected = select;
        }else{
            index = select-1;
            selected = 1;
            if( index + num_item_rows - num_items > 0 ) {
                selected += index + num_item_rows - num_items;
                index -= index + num_item_rows - num_items;
            }
        }
    }else{
        selected = 0;
        index = 0;
    }

    // Title list
    ClearTitleList();

    // Arrows
    sprArrowUp = new GuiSprite;
    sprArrowUp->SetImage(g_imgArrow);
    sprArrowUp->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprArrowUp->SetRefPixelPosition(g_imgArrow->GetWidth()/2, g_imgArrow->GetHeight()/2);
    sprArrowUp->SetPosition(xpos + xsize/2, ypos+ypitch/2+fontsize/3);
    sprArrowUp->SetStretchWidth((float)(width/2)/g_imgArrow->GetWidth());
    sprArrowUp->SetStretchHeight(((float)fontsize/g_imgArrow->GetHeight())*0.8f);
    sprArrowUp->SetRotation(180.0f);
    sprArrowUp->SetVisible(false);
    RegisterForDelete(sprArrowUp);

    sprArrowDown = new GuiSprite;
    sprArrowDown->SetImage(g_imgArrow);
    sprArrowDown->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprArrowDown->SetRefPixelPosition(g_imgArrow->GetWidth()/2, g_imgArrow->GetHeight()/2);
    sprArrowDown->SetPosition(xpos + xsize/2, ypos+(num_item_rows-1)*ypitch+ypitch/2+fontsize/3);
    sprArrowDown->SetStretchWidth((float)(width/2)/g_imgArrow->GetWidth());
    sprArrowDown->SetStretchHeight(((float)fontsize/g_imgArrow->GetHeight())*0.8f);
    sprArrowDown->SetVisible(false);
    RegisterForDelete(sprArrowDown);

    current_index = -1;
    SetSelectedItem(0, 0);

    // Arrows
    AddTop(sprArrowUp, SELECTION_EFFECT);
    AddTop(sprArrowDown, SELECTION_EFFECT);

    is_showing = true;
}

GuiElmSelectionList::GuiElmSelectionList(GuiContainer *man, int rows)
{
    num_item_rows = rows;
    is_showing = false;
    is_active = false;
    sprSelector = NULL;
    sprCursor = NULL;
    visible_items = new const char*[rows];
    memset(visible_items, 0, rows * sizeof(const char*));
    titleTxtSprite = new GuiSprite*[rows];
    memset(titleTxtSprite, 0, rows * sizeof(GuiSprite*));
}

GuiElmSelectionList::~GuiElmSelectionList()
{
    CleanUp();
    delete[] titleTxtSprite;
    delete[] visible_items;
}

