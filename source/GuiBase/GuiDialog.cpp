
#include "GuiDialog.h"

#include "../arch/archThread.h"

#include "GuiElement.h"
#include "GuiContainer.h"
#include "GuiSprite.h"

typedef struct _gritem {
  GuiElement *element;
  struct _gritem *next;
} GRITEM;

int GuiDialog::running_count = 0;
GuiSprite* GuiDialog::cursor = NULL;

GuiDialog::GuiDialog(GuiContainer *cntr)
         : GuiContainer(cntr)
{
    first_item = last_item = NULL;
    selected_element = NULL;
    last_selected = NULL;
    return_value = NULL;
    use_keyboard = true;
    quit = false;
}

GuiDialog::~GuiDialog()
{
}

/*-----------------*/

void GuiDialog::OnUpdateScreen(void)
{
}

void GuiDialog::OnKey(BTN key, bool pressed)
{
    void *rval;
    if( !pressed ) {
        return;
    }
    switch( key ) {
        case BTN_JOY1_WIIMOTE_A:
        case BTN_JOY1_CLASSIC_A:
        case BTN_JOY2_WIIMOTE_A:
        case BTN_JOY2_CLASSIC_A:
        case BTN_RETURN:
        case BTN_SPACE:
            rval = GetSelected(true);
            if( rval != NULL ) {
                Leave(rval);
            }
            break;
        case BTN_JOY1_WIIMOTE_B:
        case BTN_JOY1_CLASSIC_B:
        case BTN_JOY2_WIIMOTE_B:
        case BTN_JOY2_CLASSIC_B:
        case BTN_ESCAPE:
            Leave(NULL);
            break;
        default:
            break;
    }
}

/*-----------------*/

void GuiDialog::AddElement(GuiElement *element)
{
    if( element != NULL ) {
        GRITEM *item = new GRITEM;
        item->element = element;
        item->next = NULL;
        if( last_item == NULL ) {
            first_item = last_item = item;
        }else{
            last_item->next = item;
            last_item = item;
        }
        if( last_selected == NULL ) {
            last_selected = element;
        }
    }
}

void GuiDialog::RemoveElement(GuiElement *element)
{
    GRITEM *item = first_item;
    while( item != NULL && item->element == element ) {
        if( item->next == NULL ) {
            first_item = last_item = NULL;
        }else{
            first_item = item->next;
        }
        delete item;
        item = first_item;
    }
    while( item != NULL ) {
        GRITEM *prev = item;
        item = item->next;
        while( item != NULL ) {
            while( item != NULL && item->element == element ) {
                if( item->next == NULL ) {
                    last_item = prev;
                    prev->next = NULL;
                }else{
                    prev->next = item->next;
                }
                delete item;
                item = prev->next;
            }
            prev = item;
            if( item != NULL ) {
                item = item->next;
            }
        }
    }
    if( last_selected == element ) {
        last_selected = (first_item != NULL)? first_item->element : NULL;
    }
}

/*-----------------*/

void GuiDialog::AddIndex(int index, GuiLayer *layer, bool fix, GuiEffect *effect)
{
    GuiContainer::AddIndex(index, layer, fix, effect);
}
void GuiDialog::AddIndex(int index, GuiElement *element, bool fix, GuiEffect *effect)
{
    GuiContainer::AddIndex(index, element, fix, effect);
    AddElement(element);
}

void GuiDialog::AddTop(GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::AddTop(layer, effect);
}
void GuiDialog::AddTop(GuiElement *element, GuiEffect *effect)
{
    GuiContainer::AddTop(element, effect);
    AddElement(element);
}

void GuiDialog::AddTopFixed(GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::AddTopFixed(layer, effect);
}
void GuiDialog::AddTopFixed(GuiElement *element, GuiEffect *effect)
{
    GuiContainer::AddTopFixed(element, effect);
    AddElement(element);
}

bool GuiDialog::AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect)
{
    return GuiContainer::AddOnTopOf(ontopof, layer, effect);
}
bool GuiDialog::AddOnTopOf(GuiLayer *ontopof, GuiElement *element, GuiEffect *effect)
{
    if( GuiContainer::AddOnTopOf(ontopof, element, effect) ) {
        AddElement(element);
        return true;
    }
    return false;
}

bool GuiDialog::AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect)
{
    return GuiContainer::AddBehind(behind, layer, effect);
}
bool GuiDialog::AddBehind(GuiLayer *behind, GuiElement *element, GuiEffect *effect)
{
    if( GuiContainer::AddBehind(behind, element, effect) ) {
        AddElement(element);
        return true;
    }
    return false;
}

void GuiDialog::AddBottom(GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::AddBottom(layer, effect);
}
void GuiDialog::AddBottom(GuiElement *element, GuiEffect *effect)
{
    GuiContainer::AddBottom(element, effect);
    AddElement(element);
}

#ifdef DEBUG
void GuiDialog::_Remove(const char *file, int line, GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::_Remove(file, line, layer, effect);
}
void GuiDialog::_Remove(const char *file, int line, GuiElement *element, GuiEffect *effect)
{
    RemoveElement(element);
    GuiContainer::_Remove(file, line, element, effect);
}

void GuiDialog::_RemoveAndDelete(const char *file, int line, GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::_RemoveAndDelete(file, line, layer, effect);
}
void GuiDialog::_RemoveAndDelete(const char *file, int line, GuiElement *element, GuiEffect *effect)
{
    RemoveElement(element);
    GuiContainer::_RemoveAndDelete(file, line, element, effect);
}
#else
void GuiDialog::Remove(GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::Remove(layer, effect);
}
void GuiDialog::Remove(GuiElement *element, GuiEffect *effect)
{
    RemoveElement(element);
    GuiContainer::Remove(element, effect);
}

void GuiDialog::RemoveAndDelete(GuiLayer *layer, GuiEffect *effect)
{
    GuiContainer::RemoveAndDelete(layer, effect);
}
void GuiDialog::RemoveAndDelete(GuiElement *element, GuiEffect *effect)
{
    RemoveElement(element);
    GuiContainer::RemoveAndDelete(element, effect);
}
#endif

/*-----------------*/

void GuiDialog::SetSelected(GuiElement *elm, int x, int y)
{
    if( selected_element != elm ) {
        if( selected_element != NULL ) {
            selected_element->ElmSetSelected(false, x, y);
        }
        if( elm != NULL ) {
            elm->ElmSetSelected(true, x, y);
            last_selected = elm;
        }
        selected_element = elm;
    }
}

GuiElement* GuiDialog::GetSelected(bool active_only)
{
    if( !active_only || use_keyboard || (is_above == selected_element) ) {
        return selected_element;
    }
    return NULL;
}

/*-----------------*/

GuiElement* GuiDialog::CheckCollision(GuiSprite *sprite)
{
    GRITEM *item = first_item;
    while( item != NULL ) {
        if( item->element->ElmSetSelectedOnCollision(sprite) ) {
            return item->element;
        }
        item = item->next;
    }
    return NULL;
}

enum {
    GRDIR_UP,
    GRDIR_DOWN,
    GRDIR_LEFT,
    GRDIR_RIGHT
};

GuiElement* GuiDialog::FindNearestElement(GuiElement *elm, GRDIR dir)
{
    GuiElement *rvalue = NULL;
    GRITEM *item = first_item;
    unsigned dist2 = (unsigned)-1;
    int x, y, w, h;
    elm->ElmGetRegion(&x, &y, &w, &h);
    while( item != NULL ) {
        int xx, yy, ww, hh;
        if( item->element != elm &&
            item->element->ElmGetRegion(&xx, &yy, &ww, &hh) )
        {
            if( (dir == GRDIR_UP    && yy+hh/2 < y+h/2) ||
                (dir == GRDIR_DOWN  && yy+hh/2 > y+h/2) ||
                (dir == GRDIR_LEFT  && xx+ww/2 < x+w/2) ||
                (dir == GRDIR_RIGHT && xx+ww/2 > x+w/2) )
            {
                int dx, dy;
                unsigned d;
                if( xx < x ) {
                    dx = x - xx;
                }else if( xx > x+w ) {
                    dx = xx - (x+w);
                }else{
                    dx = 0;
                }
                if( yy < y ) {
                    dy = y - yy;
                }else if( yy > y+h ) {
                    dy = yy - (y+h);
                }else{
                    dy = 0;
                }
                d = dx * dx + dy * dy;
                if( d < dist2 ) {
                    rvalue = item->element;
                    dist2 = d;
                }
            }
        }
        item = item->next;
    }
    return rvalue;
}

bool GuiDialog::SelectNearestElement(GuiElement *elm, GRDIR dir)
{
    int x, y, w, h;
    GuiElement *select = NULL;
    if( elm != NULL && elm->ElmGetRegion(&x, &y, &w, &h) ) {
        x = x + w/2;
        y = y + y/2;
        select = FindNearestElement(elm, dir);
        if( select == NULL && selected_element == NULL ) {
            select = elm;
        }
        if( select != NULL ) {
            SetSelected(select, x, y);
            return true;
        }
    }
    return false;
}

void GuiDialog::GetKeysCallback(BTN code, int pressed)
{
    GRITEM *item = first_item;
    while( item != NULL ) {
        if( item->element->ElmHandleKey(this, code, !!pressed) ) {
            use_keyboard = true;
            return;
        }
        item = item->next;
    }
    if( pressed &&
        last_selected != NULL ) {
        switch( code ) {
            case BTN_UP:
            case BTN_JOY1_UP:
            case BTN_JOY2_UP:
                if( SelectNearestElement(last_selected, GRDIR_UP) ) {
                    use_keyboard = true;
                }
                break;
            case BTN_DOWN:
            case BTN_JOY1_DOWN:
            case BTN_JOY2_DOWN:
                if( SelectNearestElement(last_selected, GRDIR_DOWN) ) {
                    use_keyboard = true;
                }
                break;
            case BTN_LEFT:
            case BTN_JOY1_LEFT:
            case BTN_JOY2_LEFT:
                if( SelectNearestElement(last_selected, GRDIR_LEFT) ) {
                    use_keyboard = true;
                }
                break;
            case BTN_RIGHT:
            case BTN_JOY1_RIGHT:
            case BTN_JOY2_RIGHT:
                if( SelectNearestElement(last_selected, GRDIR_RIGHT) ) {
                    use_keyboard = true;
                }
                break;
            default:
                break;
        }
    }
    OnKey(code, !!pressed);
}

void GuiDialog::GetKeysCallbackWrapper(void *context, BTN code, int pressed)
{
    GuiDialog *me = (GuiDialog *)context;
    me->GetKeysCallback(code, pressed);
}

void GuiDialog::Leave(void *retval)
{
    return_value = retval;
    quit = true;
}

bool GuiDialog::FrameCallbackWrapper(void *context)
{
    return ((GuiDialog *)context)->FrameCallback();
}

bool GuiDialog::FrameCallback(void)
{
    cursor->SetVisible(false);
    
    // Infrared
    int x, y, angle;
    if( g_poGwd->input.GetWiiMoteIR(&x, &y, &angle) ) {
        cursor->SetPosition((f32)x, (f32)y);
        cursor->SetRotation((f32)angle);
        cursor->SetVisible(true);
    }else{
        cursor->SetVisible(false);
        cursor->SetPosition(0, 0);
    }
    
    // Check mouse cursor colisions
    is_above = CheckCollision(cursor);
    
    // Check arrow keys
    if( is_above != NULL || !use_keyboard ) {
        SetSelected(is_above, x, y);
        if( is_above != NULL ) {
            use_keyboard = false;
        }
    }
    
    OnUpdateScreen();

    return is_modal;
}

void* GuiDialog::Run(bool modal)
{
    quit = false;
    is_modal = modal;
    return_value = NULL;

    // Cursor
    if( running_count == 0 ) {
        cursor = new GuiSprite;
        cursor->SetImage(GetPointerImage());
        cursor->SetPosition(0, 0);
        cursor->SetVisible(false);
        GetRootContainer()->RegisterForDelete(cursor);
        GetRootContainer()->AddTopFixed(cursor);
    }

    running_count++;
    GetRootContainer()->AddFrameCallback(FrameCallbackWrapper, this);

    for(;;) {
        // Check keys
        g_poGwd->input.GetButtonEvents(GetKeysCallbackWrapper, this);

        // Leave when requested
        if( quit ) {
            break;
        }
        archThreadSleep(20);
    }

    GetRootContainer()->RemoveFrameCallback(FrameCallbackWrapper, this);
    running_count--;

    // Cleanup
    if( running_count == 0 ) {
        GetRootContainer()->RemoveAndDelete(cursor);
        cursor = NULL;
    }

    return return_value;
}

