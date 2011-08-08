
#include "GuiElmSelectionList.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>

#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiSprite.h"

#include "../GuiElements/GuiElmFrame.h"
#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

#define SELECTION_FADE_TIME  10
#define SELECTION_FADE_DELAY 2
#define SELECTION_EFFECT new GuiEffectFade(SELECTION_FADE_TIME, SELECTION_FADE_DELAY)

//-----------------------

GuiElmListLineDefault::GuiElmListLineDefault(GuiContainer *parent, const char *name,
                                             GXColor fontcol, int fontsz, bool cntr)
                     : GuiElmListLine(parent, name)
{
    text = NULL;
    sprite = NULL;
    fontcolor = fontcol;
    fontsize = fontsz;
    center = cntr;
}

GuiElmListLineDefault::~GuiElmListLineDefault()
{
    if( sprite != NULL ) {
        RemoveAndDelete(sprite);
        sprite = NULL;
    }
}

void GuiElmListLineDefault::Initialize(void *item)
{
    text = (const char *)item;
    sprite = new GuiSprite(this, "textsprite");
    sprite->CreateTextImage(g_fontArial, fontsize, GetWidth(), 0, center, fontcolor, text);
    sprite->SetPosition(0, fontsize/3);
    AddTop(sprite);
}

GuiElmListLine* GuiElmListLineDefault::Create(GuiContainer *parent)
{
    return new GuiElmListLineDefault(parent, text,
                                     fontcolor, fontsize, center);
}

COLL GuiElmListLineDefault::CollidesWith(GuiSprite* spr, bool complete)
{
    assert( sprite != NULL );
    return sprite->CollidesWith(spr, complete);
}


//-----------------------

bool GuiElmSelectionList::ElmSetSelectedOnCollision(GuiSprite *sprite)
{
    bool return_value = false;

    Lock();

    for(int i = upper_index; i <= lower_index; i++) {
        if( visible_items[i] != NULL ) {
            COLL coll = visible_items[i]->CollidesWith(sprite);
            if( coll == COLL_UNKNOWN ) {
                Unlock();
                return false;
            }
            if( coll == COLL_COLLISION ) {
                if( i != selected ) {
                    selected = i;
                }
                return_value = true;
                break;
            }else{
                if( i == 2 ) {
                    i=2;
                }
            }
        }
    }
    SetSelectedItem();

    Unlock();

    return return_value;;
}

void GuiElmSelectionList::ElmSetSelected(bool sel, GuiSprite *pointer, int x, int y)
{
    Lock();

    if( sel ) {
        int s = -1;
        for(int i = upper_index; i <= lower_index; i++) {
            if( visible_items[i] != NULL && pointer != NULL &&
                visible_items[i]->CollidesWith(pointer) == COLL_COLLISION ) {
                s = i;
                break;
            }
        }
        for(int i = upper_index; s < 0 && i <= lower_index; i++) {
            if( visible_items[i] == NULL ||
               (int)visible_items[i]->GetY() + visible_items[i]->GetHeight() / 2 > (unsigned)y )
            {
                break;
            }
            s = i;
        }
        if( s < 0 ) {
            s = upper_index;
        }
        if( s != selected ) {
            selected = s;
        }
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
    }else{
        if( index > 0 ) {
            index--;
        }
    }
}

void GuiElmSelectionList::DoKeyDown(void)
{
    if( selected < lower_index &&
        visible_items[selected+1] != NULL ) {
        selected++;
    }else{
        if( index+selected < num_items-1 ) {
            index++;
        }
    }
}

void GuiElmSelectionList::CleanUp(void)
{
    Lock();

    if( listline != NULL ) {
        GetParentContainer()->Delete(listline);
        listline = NULL;
    }
    if( is_showing ) {
        // Titles
        for(int i = 0; i < num_item_rows; i++) {
            if( visible_items[i] != NULL ) {
                RemoveAndDelete(visible_items[i], SELECTION_EFFECT);
                visible_items[i] = NULL;
            }
        }

        // Arrows
        if( sprArrowUp != NULL ) {
            RemoveAndDelete(sprArrowUp, SELECTION_EFFECT);
            sprArrowUp = NULL;
        }
        if( sprArrowDown != NULL ) {
            RemoveAndDelete(sprArrowDown, SELECTION_EFFECT);
            sprArrowDown = NULL;
        }

        // Title list
        if( sprSelector != NULL ) {
            RemoveAndDelete(sprSelector, SELECTION_EFFECT);
            sprSelector = NULL;
        }

        is_showing = false;
    }

    Unlock();
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
    Lock();

    // Render text
    if( index == current_index && current_index != -1 ) {
        // Index not changed, nothing to update
    }else
    if( index == current_index+1 && current_index != -1 ) {
        // move list up
        RemoveAndDelete(visible_items[0]);
        for(int i = 0, yy = ypos; i < num_item_rows-1; i++, yy += ypitch) {
            visible_items[i] = visible_items[i+1];
            visible_items[i]->SetPosition(xpos + xspacing, yy);
            visible_items[i]->SetVisible(true);
        }
        GuiElmListLine* line = listline->Create(this);
        line->SetWidth(xsize-2*xspacing);
        line->SetHeight(ypitch);
        line->Initialize(item_list[num_item_rows-1+index]);
        line->SetPosition(xpos + xspacing, ypos + (num_item_rows-1)*ypitch);
        AddOnTopOf(visible_items[num_item_rows-1], line, SELECTION_EFFECT);
        visible_items[num_item_rows-1] = line;
    }else
    if( index == current_index-1 ) {
        // move list down
        if( visible_items[num_item_rows-1] ) {
            RemoveAndDelete(visible_items[num_item_rows-1]);
        }
        for(int i = num_item_rows-1, yy = ypos+(num_item_rows-1)*ypitch; i > 0; i--, yy -= ypitch) {
            visible_items[i] = visible_items[i-1];
            visible_items[i]->SetPosition(xpos + xspacing, yy);
            visible_items[i]->SetVisible(true);
        }
        GuiElmListLine* line = listline->Create(this);
        line->SetWidth(xsize-2*xspacing);
        line->SetHeight(ypitch);
        line->Initialize(item_list[0+index]);
        line->SetPosition(xpos + xspacing, ypos);
        AddOnTopOf(visible_items[0], line, SELECTION_EFFECT);
        visible_items[0] = line;
    }else{
        // rebuild
        for(int i = 0, yy = ypos; i < num_item_rows; i++, yy += ypitch) {
            if( i < num_items ) {
                GuiElmListLine* line = listline->Create(this);
                line->SetWidth(xsize-2*xspacing);
                line->SetHeight(ypitch);
                line->Initialize(item_list[i+index]);
                line->SetPosition(xpos + xspacing, yy);
                line->SetVisible(true);
                if( visible_items[i] != NULL ) {
                    AddOnTopOf(visible_items[i], line, SELECTION_EFFECT);
                    RemoveAndDelete(visible_items[i], SELECTION_EFFECT);
                }else{
                    AddTop(line, SELECTION_EFFECT);
                }
                visible_items[i] = line;
            }else{
                visible_items[i] = NULL;
            }
        }
    }
    // Up button
    if( index > 0  ) {
        if( visible_items[0] != NULL ) {
            visible_items[0]->SetVisible(false);
        }
        sprArrowUp->SetVisible(true);
        upper_index = 1;
    }else{
        if( visible_items[0] != NULL ) {
            visible_items[0]->SetVisible(true);
        }
        sprArrowUp->SetVisible(false);
        upper_index = 0;
    }
    // Down button
    if( index+num_item_rows < num_items ) {
        if( visible_items[num_item_rows-1] != NULL ) {
            visible_items[num_item_rows-1]->SetVisible(false);
        }
        sprArrowDown->SetVisible(true);
        lower_index = num_item_rows-2;
    }else{
        if( visible_items[num_item_rows-1] != NULL ) {
            visible_items[num_item_rows-1]->SetVisible(true);
        }
        sprArrowDown->SetVisible(false);
        lower_index = num_item_rows-1;
    }
    // Update seletion
    if( selected != prev_selected ) {
        if( sprSelector != NULL ) {
            RemoveAndDelete(sprSelector, new GuiEffectFade(
                                         (fade > 0)? fade : SELECTION_FADE_TIME,
                                         (delay > 0)? delay : SELECTION_FADE_DELAY));
            sprSelector = NULL;
        }
        if( selected >= 0 ) {
            GuiLayer *selectedline = visible_items[selected];
            sprSelector = new GuiSprite(this, "selector");
            sprSelector->SetImage(g_imgSelector);
            sprSelector->SetRefPixelPosition(0, 0);
            sprSelector->SetPosition(selectedline->GetX()-xspacing,selectedline->GetY());
            sprSelector->SetStretchWidth((float)xsize / sprSelector->GetWidth());
            sprSelector->SetStretchHeight((float)ypitch * 1.15f / sprSelector->GetHeight());
            AddBottom(sprSelector, new GuiEffectFade((fade > 0)? fade : SELECTION_FADE_TIME));
            is_active = true;
        }
        prev_selected = selected;
    }
    current_index = index;

    Unlock();
}


void GuiElmSelectionList::InitSelection(GuiElmListLine *listln, void **items, int num, int select,
                                        int pitchy, int posx, int posy, int xspace, int width)
{
    Lock();

    CleanUp();

    assert( listln != NULL );
    listline = listln;
    xpos = posx;
    ypos = posy;
    xsize = width;
    xspacing = xspace;
    ypitch = pitchy;

    // Init items
    item_list = items;
    num_items = num;

    // Selected initial entry
    int new_selected = 0;
    if( select >= 0 ) {
        if( select < num_item_rows-1 ) {
            index = 0;
            new_selected = select;
        }else{
            index = select-1;
            new_selected = 1;
            if( index + num_item_rows - num_items > 0 ) {
                new_selected += index + num_item_rows - num_items;
                index -= index + num_item_rows - num_items;
            }
        }
    }else{
        index = 0;
    }

    // Title list
    ClearTitleList();

    // Arrows
    sprArrowUp = new GuiSprite(this, "arrow_up");
    sprArrowUp->SetImage(g_imgArrow);
    sprArrowUp->SetRefPixelPosition((f32)(g_imgArrow->GetWidth()/2),
		                            (f32)(g_imgArrow->GetHeight()/2));
    sprArrowUp->SetPosition(xpos + xsize/2, ypos+ypitch/2);
    sprArrowUp->SetStretchWidth(((float)width / 2) / g_imgArrow->GetWidth());
    sprArrowUp->SetStretchHeight(((float)ypitch / 2) / g_imgArrow->GetHeight());
    sprArrowUp->SetRotation(180.0f);
    sprArrowUp->SetVisible(false);

    sprArrowDown = new GuiSprite(this, "arrow_down");
    sprArrowDown->SetImage(g_imgArrow);
    sprArrowDown->SetRefPixelPosition((f32)(g_imgArrow->GetWidth()/2),
		                              (f32)(g_imgArrow->GetHeight()/2));
    sprArrowDown->SetPosition(xpos + xsize/2, ypos+(num_item_rows-1)*ypitch+ypitch/2);
    sprArrowDown->SetStretchWidth(((float)width / 2) / g_imgArrow->GetWidth());
    sprArrowDown->SetStretchHeight(((float)ypitch / 2) / g_imgArrow->GetHeight());
    sprArrowDown->SetVisible(false);

    current_index = -1;
    prev_selected = -1;
    selected = -1;
    SetSelectedItem();
    selected = new_selected;

    // Arrows
    AddTop(sprArrowUp, SELECTION_EFFECT);
    AddTop(sprArrowDown, SELECTION_EFFECT);

    is_showing = true;

    Unlock();
}

GuiElmSelectionList::GuiElmSelectionList(GuiContainer *parent, const char *name, int rows)
                   : GuiElement(parent, name)
{
    num_item_rows = rows;
    is_showing = false;
    is_active = false;
    listline = NULL;
    sprSelector = NULL;
    sprCursor = NULL;
    visible_items = new GuiElmListLine*[rows];
    memset(visible_items, 0, rows * sizeof(GuiElmListLine*));
}

GuiElmSelectionList::~GuiElmSelectionList()
{
    CleanUp();
    delete[] visible_items;
}

