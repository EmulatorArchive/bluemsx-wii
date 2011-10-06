
#include "GuiElmSelectionList.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>

#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiRect.h"
#include "../GuiBase/GuiSprite.h"

#include "../GuiElements/GuiElmFrame.h"
#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

#define SELECTION_FADE_TIME  15
#define SELECTION_FADE_DELAY 5

//-----------------------

GuiElmListLineDefault::GuiElmListLineDefault(GuiElement *parent, const char *name,
                                             GXColor fontcol, int fontsz, float xspace, bool cntr) :
                       GuiElmListLine(parent, name)
{
    text = NULL;
    sprite = NULL;
    selector = NULL;
    m_bEnableEffect = false;
    fontcolor = fontcol;
    fontsize = fontsz;
    xspacing = xspace;
    center = cntr;
}

GuiElmListLineDefault::~GuiElmListLineDefault()
{
    if( sprite != NULL ) {
        RemoveAndDelete(sprite);
    }
    if( selector != NULL ) {
        RemoveAndDelete(selector);
    }
}

GuiElmListLine* GuiElmListLineDefault::Create(GuiElement *parent)
{
    return new GuiElmListLineDefault(parent, text,
                                     fontcolor, fontsize, xspacing, center);
}

void GuiElmListLineDefault::Initialize(void *item)
{
    text = (const char *)item;

    sprite = new GuiSprite(this, "textsprite");
    sprite->CreateTextImage(g_fontArial, fontsize, (int)GetWidth(), 0, center, fontcolor, text);
    sprite->SetPosition(xspacing, (float)fontsize/3);
    AddTop(sprite);
}

void GuiElmListLineDefault::EnableEffect(bool enable)
{
    m_bEnableEffect = enable;
}

bool GuiElmListLineDefault::OnTestActiveArea(float x, float y)
{
    return IsInVisibleArea(x, y);
}

void GuiElmListLineDefault::OnActive(bool active)
{
    if( active && sprite != NULL ) {
        selector = new GuiElmFrame(this, "selector", FRAMETYPE_SELECTOR, 0, 0, GetWidth(), GetHeight());
        selector->SetScaledWidth(GetWidth());
        selector->SetScaledHeight(GetHeight());
        AddBehind(sprite, selector, m_bEnableEffect? new GuiEffectFade(SELECTION_FADE_TIME) : NULL);
    }
    if( !active && selector != NULL ) {
        RemoveAndDelete(selector, m_bEnableEffect? new GuiEffectFade(SELECTION_FADE_TIME, SELECTION_FADE_DELAY) : NULL);
        selector = NULL;
    }
}

//-----------------------

GuiElmSelectionList::GuiElmSelectionList(GuiElement *parent, const char *name, int rows) :
                     GuiElement(parent, name),
                     effectDefault(SELECTION_FADE_TIME, SELECTION_FADE_DELAY)
{
    num_item_rows = rows;
    is_showing = false;
    listline = NULL;
    sprCursor = NULL;
    selected_item = -1;
    visible_items = new GuiElmListLine*[rows];
    memset(visible_items, 0, rows * sizeof(GuiElmListLine*));
}

GuiElmSelectionList::~GuiElmSelectionList()
{
    CleanUp();
    delete[] visible_items;
}

//-----------------------

void GuiElmSelectionList::OnSelect(GuiElement *element)
{
    for(int i = 0; i < num_item_rows; i++) {
        if( visible_items[i] == element ) {
            selected_item = index + i;
            break;
        }
    }
}

bool GuiElmSelectionList::OnKey(GuiDialog *dlg, BTN key, bool pressed)
{
    bool handled = false;

    // Handle selection movement (arrow keys)
    if( HasFocus() && pressed ) {
        switch( key ) {
            case BTN_UP:
            case BTN_JOY1_UP:
            case BTN_JOY2_UP:
                if( GetFocusElement() == visible_items[1] && index > 0 ) {
                    UpdateList(UPDATELIST_SCROLL_UP);
                    handled = true;
                }
                break;
            case BTN_DOWN:
            case BTN_JOY1_DOWN:
            case BTN_JOY2_DOWN:
                if( GetFocusElement() == visible_items[num_item_rows-2] && index + num_item_rows < num_items ) {
                    UpdateList(UPDATELIST_SCROLL_DOWN);
                    handled = true;
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

//-----------------------

void GuiElmSelectionList::CleanUp(void)
{
    if( listline != NULL ) {
        Delete(listline);
        listline = NULL;
    }
    if( is_showing ) {
        // Titles
        for(int i = 0; i < num_item_rows; i++) {
            if( visible_items[i] != NULL ) {
                RemoveAndDelete(visible_items[i], effectDefault);
                visible_items[i] = NULL;
            }
        }

        // Arrows
        if( sprArrowUp != NULL ) {
            RemoveAndDelete(sprArrowUp, effectDefault);
            sprArrowUp = NULL;
        }
        if( sprArrowDown != NULL ) {
            RemoveAndDelete(sprArrowDown, effectDefault);
            sprArrowDown = NULL;
        }

        is_showing = false;
    }
}

int GuiElmSelectionList::GetSelectedItem(void)
{
    return selected_item;
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
    return is_showing;
}

void GuiElmSelectionList::UpdateList(UPDATELIST update)
{
    Lock();

    int selected = GetSelectedItem();
    if( selected >= 0 ) {
        selected -= index;
    }

    if( update == UPDATELIST_SCROLL_DOWN ) {
        GuiElmListLine *restore_element = NULL;
        if( selected >= 0 ) {
            restore_element = visible_items[selected];
            restore_element->EnableEffect(false);
        }
        // move list up
        index++;
        RemoveAndDelete(visible_items[0]);
        float yy = 0.0f;
        for(int i = 0; i < num_item_rows-1; i++) {
            visible_items[i] = visible_items[i+1];
            visible_items[i]->SetPosition(0.0f, yy);
            visible_items[i]->SetVisible(true);
            yy += ypitch;
        }
        GuiElmListLine* line = listline->Create(this);
        line->SetWidth(GetWidth());
        line->SetHeight(rowheight);
        line->Initialize(item_list[num_item_rows-1+index]);
        line->SetPosition(0.0f, (num_item_rows-1)*ypitch);
        line->EnableEffect(true);
        AddOnTopOf(visible_items[num_item_rows-1], line, effectDefault);
        visible_items[num_item_rows-1] = line;
        // preserve current selection
        if( restore_element != NULL ) {
            visible_items[selected]->EnableEffect(false);
            visible_items[selected]->SetFocus(true);
            visible_items[selected]->EnableEffect(true);
            restore_element->EnableEffect(true);
        }
        line->EnableEffect(true);
    }else
    if( update == UPDATELIST_SCROLL_UP ) {
        if( selected >= 0 ) {
            visible_items[selected]->EnableEffect(false);
            //visible_items[selected]->SetFocus(false);
            SetActiveElement(NULL);
            visible_items[selected]->EnableEffect(true);
        }
        // move list down
        index--;
        if( visible_items[num_item_rows-1] ) {
            RemoveAndDelete(visible_items[num_item_rows-1]);
        }
        float yy = ypitch * (num_item_rows - 1);
        for(int i = num_item_rows-1; i > 0; i--) {
            visible_items[i] = visible_items[i-1];
            visible_items[i]->SetPosition(0.0f, yy);
            visible_items[i]->SetVisible(true);
            yy -= ypitch;
        }
        GuiElmListLine* line = listline->Create(this);
        line->SetWidth(GetWidth());
        line->SetHeight(rowheight);
        line->Initialize(item_list[0+index]);
        line->SetPosition(0, 0);
        line->EnableEffect(true);
        AddOnTopOf(visible_items[0], line, effectDefault);
        visible_items[0] = line;
        // preserve current selection
        if( selected >= 0 ) {
            visible_items[selected]->EnableEffect(false);
            visible_items[selected]->SetFocus(true);
            visible_items[selected]->EnableEffect(true);
        }
        line->EnableEffect(true);
    }else
    if( update == UPDATELIST_REBUILD ) {
        // rebuild
        float yy = 0.0f;
        for(int i = 0; i < num_item_rows; i++) {
            if( i < num_items ) {
                GuiElmListLine* line = listline->Create(this);
                line->SetWidth(GetWidth());
                line->SetHeight(rowheight);
                line->Initialize(item_list[i+index]);
                line->SetPosition(0.0f, yy);
                line->SetVisible(true);
                line->EnableEffect(true);
                if( visible_items[i] != NULL ) {
                    AddOnTopOf(visible_items[i], line, effectDefault);
                    RemoveAndDelete(visible_items[i], effectDefault);
                }else{
                    AddTop(line, effectDefault);
                }
                visible_items[i] = line;
            }else{
                visible_items[i] = NULL;
            }
            yy += ypitch;
        }
    }
    // Up arrow
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
    // Down arrow
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

    Unlock();
}


void GuiElmSelectionList::InitSelection(GuiElmListLine *listln, void **items, int num, int select,
                                        float pitchy, float posx, float posy, float width)
{
    CleanUp();

    assert( listln != NULL );
    listline = listln;
    SetWidth(width);
    ypitch = pitchy;
    rowheight = pitchy * 1.1f;

    SetHeight((num_item_rows-1) * pitchy + rowheight);
    SetPosition(posx, posy);

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

    // Arrows
    sprArrowUp = new GuiSprite(this, "arrow_up");
    GuiImages::AssignSpriteToImage(sprArrowUp, "image_arrow");
    sprArrowUp->SetRefPixelPosition(sprArrowUp->GetWidth() / 2,
                                    sprArrowUp->GetHeight() / 2);
    sprArrowUp->SetPosition(GetWidth()/2, ypitch/2);
    sprArrowUp->SetScaledWidth(width / 2);
    sprArrowUp->SetScaledHeight(ypitch / 2);
    sprArrowUp->SetRotation(180.0f);
    sprArrowUp->SetVisible(false);

    sprArrowDown = new GuiSprite(this, "arrow_down");
    GuiImages::AssignSpriteToImage(sprArrowDown, "image_arrow");
    sprArrowDown->SetRefPixelPosition(sprArrowUp->GetWidth() / 2,
                                      sprArrowUp->GetHeight() / 2);
    sprArrowDown->SetPosition(GetWidth()/2, (num_item_rows-1)*ypitch+ypitch/2);
    sprArrowDown->SetScaledWidth(width / 2);
    sprArrowDown->SetScaledHeight(ypitch / 2);
    sprArrowDown->SetVisible(false);

    // Initialize list
    UpdateList(UPDATELIST_REBUILD);
    visible_items[new_selected]->SetFocus(true);

    // Arrows
    AddTop(sprArrowUp, effectDefault);
    AddTop(sprArrowDown, effectDefault);

    is_showing = true;
}

