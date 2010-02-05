#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include <fat.h>
#include <wiiuse/wpad.h>

#include "kbdlib.h"
#include "GuiRunner.h"
#include "GuiSelectionList.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

#define SELECTION_FADE_TIME  10
#define SELECTION_FADE_DELAY 4

//-----------------------

void GuiSelectionList::ElmAddLayers(GuiManager *manager, int index, bool fix, int fade, int delay)
{
    if( !is_showing ) {
        // Title list
        ClearTitleList();
        InitTitleList(g_fontArial, xpos, ypos, xsize-2*xspacing, ypitch, fade);

        // Titles
        for(int i = 0; i < num_item_rows; i++) {
            manager->AddIndex(index++, titleTxtSprite[i], false, fade, delay);
        }

        // Arrows
        manager->AddIndex(index++, sprArrowUp, false, fade);
        manager->AddIndex(index++, sprArrowDown, false, fade);

        SetSelected(fade, delay);
        is_showing = true;
    }
}

void GuiSelectionList::ElmRemoveLayers(GuiManager *manager, bool del, int fade, int delay)
{
    if( is_showing ) {
        // Titles
        for(int i = 0; i < num_item_rows; i++) {
            manager->RemoveAndDelete(titleTxtSprite[i], titleTxtSprite[i]->GetImage(), fade, delay);
            titleTxtSprite[i] = NULL;
        }

        // Arrows
        manager->RemoveAndDelete(sprArrowUp, NULL, fade, delay);
        sprArrowUp = NULL;
        manager->RemoveAndDelete(sprArrowDown, NULL, fade, delay);
        sprArrowDown = NULL;

        // Title list
        RemoveTitleList(fade, delay);
        manager->RemoveAndDelete(sprSelector, NULL, fade, delay);
        sprSelector = NULL;

        is_showing = false;
    }
}

wsp::Layer* GuiSelectionList::ElmGetTopLayer(void)
{
    return titleTxtSprite[0];
}

wsp::Layer* GuiSelectionList::ElmGetBottomLayer(void)
{
    return sprArrowDown;
}


//-----------------------

bool GuiSelectionList::ElmSetSelectedOnCollision(GuiRunner *runner, Sprite *sprite)
{
    for(int i = upper_index; i <= lower_index; i++) {
        if( strlen(visible_items[i]) &&
            sprite->CollidesWith(titleTxtSprite[i]) ) {
            if( i != selected ) {
                selected = i;
                SetSelected();
            }
            return true;
        }
    }
    return false;
}

void GuiSelectionList::ElmSetSelected(GuiRunner *runner, bool sel, int x, int y)
{
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
        SetSelected();
    }else{
        // We're never deselected
    }
}

bool GuiSelectionList::ElmGetRegion(GuiRunner *runner, int *px, int *py, int *pw, int *ph)
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

bool GuiSelectionList::ElmHandleKey(GuiRunner *runner, KEY key, bool pressed)
{
    if (runner->GetSelected(false) == NULL && selected >= 0) { /* When nothing selected, we're in charge */
        runner->SetSelected(this, 0, titleTxtSprite[selected]->GetY() + titleTxtSprite[selected]->GetHeight() / 2);
    }
    if( pressed &&
        runner->GetSelected(false) == this )
    {
        switch( key ) {
            case KEY_UP:
            case KEY_JOY1_UP:
            case KEY_JOY2_UP:
                DoKeyUp();
                return true;
            case KEY_DOWN:
            case KEY_JOY1_DOWN:
            case KEY_JOY2_DOWN:
                DoKeyDown();
                return true;
            default:
                break;
        }
    }
    return false;
}

//-----------------------

void GuiSelectionList::DoKeyUp(void)
{
    if( selected > upper_index ) {
        selected--;
        SetSelected();
    }else{
        if( index > 0 ) {
            index--;
            SetSelected();
        }
    }
}

void GuiSelectionList::DoKeyDown(void)
{
    if( selected < lower_index &&
        strlen(visible_items[selected+1]) ) {
        selected++;
        SetSelected();
    }else{
        if( index+selected < num_items-1 ) {
            index++;
            SetSelected();
        }
    }
}

void GuiSelectionList::InitTitleList(TextRender *fontArial,
                                     int x, int y, int width, int ypitch, int fade)
{
    // Fill titles
    for(int i = 0, yy = y; i < num_item_rows; i++) {
        DrawableImage *img = new DrawableImage;
        img->CreateImage((width + 3) & ~3, ((fontsize * 3) / 2) & ~3);
        img->SetFont(fontArial);
        img->SetColor((GXColor){255,255,255,255});
        img->SetSize(fontsize);
        titleTxtImgPtr[i] = img;
        titleTxtSprite[i] = new Sprite;
        titleTxtSprite[i]->SetImage(img);
        titleTxtSprite[i]->SetPosition(x + xspacing, yy + fontsize/5);
        yy += ypitch;
    }

    // Arrows
    sprArrowUp = new Sprite;
    sprArrowUp->SetImage(g_imgArrow);
    sprArrowUp->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprArrowUp->SetRefPixelPosition(g_imgArrow->GetWidth()/2, g_imgArrow->GetHeight()/2);
    sprArrowUp->SetPosition(x + xspacing + width/2, y+ypitch/2+fontsize/5);
    sprArrowUp->SetStretchWidth((float)(width/2)/g_imgArrow->GetWidth());
    sprArrowUp->SetStretchHeight(((float)fontsize/g_imgArrow->GetHeight())*0.8f);
    sprArrowUp->SetRotation(180.0f/2);
    sprArrowUp->SetVisible(false);

    sprArrowDown = new Sprite;
    sprArrowDown->SetImage(g_imgArrow);
    sprArrowDown->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprArrowUp->SetRefPixelPosition(g_imgArrow->GetWidth()/2, g_imgArrow->GetHeight()/2);
    sprArrowDown->SetPosition(x + xspacing + width/2, y+(num_item_rows-1)*ypitch+ypitch/2+fontsize/5);
    sprArrowDown->SetStretchWidth((float)(width/2)/g_imgArrow->GetWidth());
    sprArrowDown->SetStretchHeight(((float)fontsize/g_imgArrow->GetHeight())*0.8f);
    sprArrowDown->SetVisible(false);

    current_index = -1;
}

void GuiSelectionList::RemoveTitleList(int fade, int delay)
{
}

void GuiSelectionList::ClearTitleList(void)
{
    current_index = -1;
}

int GuiSelectionList::GetSelected(void)
{
    return selected;
}

void GuiSelectionList::SetSelected(int fade, int delay)
{
    // Claim UI
    manager->Lock();
    // Update dir info
    for(int i = 0; i < num_item_rows; i++) {
        if( i+index < num_items ) {
            visible_items[i] = item_list[i+index];
        }else{
            visible_items[i] = "";
        }
    }
    // Render text (slow)
    if( index == current_index+1 && current_index != -1 ) {
        DrawableImage *p = titleTxtImgPtr[0];
        for(int i = 0; i < num_item_rows-1; i++) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i+1];
        }
        titleTxtImgPtr[num_item_rows-1] = p;
        titleTxtImgPtr[num_item_rows-1]->RenderText(center, visible_items[num_item_rows-1]);
    }else
    if( index == current_index-1 ) {
        DrawableImage *p = titleTxtImgPtr[num_item_rows-1];
        for(int i = num_item_rows-1; i > 0; i--) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i-1];
        }
        titleTxtImgPtr[0] = p;
        titleTxtImgPtr[0]->RenderText(center, visible_items[0]);
    }else
    for(int i = 0; i < num_item_rows; i++) {
        titleTxtImgPtr[i]->RenderText(center, visible_items[i]);
    }
    current_index = index;
    // Update sprites
    for(int i = 0; i < num_item_rows; i++) {
        titleTxtSprite[i]->SetImage(titleTxtImgPtr[i]->GetImage());
    }
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
        manager->RemoveAndDelete(sprSelector, NULL, (fade != -1)? fade : SELECTION_FADE_TIME,
                                                    (delay != -1)? delay : SELECTION_FADE_DELAY);
        sprSelector = NULL;
    }
    if( selected >= 0 ) {
        Sprite *selectedsprite = titleTxtSprite[selected];
        sprSelector = new Sprite;
        sprSelector->SetImage(g_imgSelector);
        sprSelector->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprSelector->SetRefPixelPosition(0, 0);
        sprSelector->SetPosition(selectedsprite->GetX()-xspacing,selectedsprite->GetY()-fontsize/5);
        sprSelector->SetStretchWidth((float)xsize / 4);
        sprSelector->SetStretchHeight((float)fontsize * 1.8f / 44);
        manager->AddBehind(selectedsprite, sprSelector, (fade != -1)? fade : SELECTION_FADE_TIME);
    }
    // Release UI
    manager->Unlock();
}


void GuiSelectionList::InitSelection(const char **items, int num, int select, int fontsz, int pitchy,
                                     int posx, int posy, int xspace, int width, bool centr)
{
    xpos = posx;
    ypos = posy;
    xsize = width;
    xspacing = xspace;
    fontsize = fontsz;
    ypitch = pitchy;
    center = centr;

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
}

void GuiSelectionList::SetNumberOfItems(int num)
{
    num_items = num;
}

bool GuiSelectionList::IsShowing(void)
{
    return is_showing;
}

GuiSelectionList::GuiSelectionList(GuiManager *man, int rows)
{
    manager = man;
    num_item_rows = rows;
    is_showing = false;
    sprSelector = NULL;
    sprCursor = NULL;
    visible_items = new const char*[rows];
    titleTxtSprite = new Sprite*[rows];
    titleTxtImgPtr = new DrawableImage*[rows];
}

GuiSelectionList::~GuiSelectionList()
{
    assert( !is_showing );
    delete[] titleTxtImgPtr;
    delete[] titleTxtSprite;
    delete[] visible_items;
}

