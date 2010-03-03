
#include "GuiRunner.h"
#include "GuiDialog.h"
#include "GuiImages.h"
#include <wiiuse/wpad.h>

typedef struct _gritem {
  GuiElement *element;
  struct _gritem *next;
} GRITEM;

GuiRunner::GuiRunner(GuiManager *man, GuiDialog *dia)
{
  manager = man;
  dialog = dia;
  first_item = last_item = NULL;
  selected_element = NULL;
  last_selected = NULL;
  return_value = NULL;
  use_keyboard = true;
  quit = false;
}

GuiRunner::~GuiRunner()
{
}

void GuiRunner::AddElement(GuiElement *element)
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

void GuiRunner::RemoveElement(GuiElement *element)
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

void GuiRunner::AddIndex(int index, GuiElement *element, bool fix, int fade, int delay)
{
    AddElement(element);
    element->ElmAddLayers(manager, index, fix, fade, delay);
}

void GuiRunner::AddTop(GuiElement *element, int fade, int delay)
{
    AddElement(element);
    element->ElmAddLayers(manager, manager->GetFixedLayers(), false, fade, delay);
}

void GuiRunner::AddTopFixed(GuiElement *element, int fade, int delay)
{
    AddElement(element);
    element->ElmAddLayers(manager, 0, true, fade, delay);
}

void GuiRunner::AddOnTopOf(GuiElement *ontopof, GuiElement *element, int fade, int delay)
{
    if( ontopof != NULL ) {
        int index = manager->GetIndex(ontopof->ElmGetTopLayer());
        if( index >= 0 ) {
            AddElement(element);
            element->ElmAddLayers(manager, index, false, fade, delay);
        }
    }
}

void GuiRunner::AddOnTopOf(Layer *ontopof, GuiElement *element, int fade, int delay)
{
    if( ontopof != NULL ) {
        int index = manager->GetIndex(ontopof);
        if( index >= 0 ) {
            AddElement(element);
            element->ElmAddLayers(manager, index, false, fade, delay);
        }
    }
}

void GuiRunner::AddBehind(GuiElement *behind, GuiElement *element, int fade, int delay)
{
    if( behind != NULL ) {
        int index = manager->GetIndex(behind->ElmGetBottomLayer());
        if( index >= 0 ) {
            AddElement(element);
            element->ElmAddLayers(manager, index, false, fade, delay);
        }
    }
}

void GuiRunner::AddBehind(Layer *behind, GuiElement *element, int fade, int delay)
{
    if( behind != NULL ) {
        int index = manager->GetIndex(behind);
        if( index >= 0 ) {
            AddElement(element);
            element->ElmAddLayers(manager, index, false, fade, delay);
        }
    }
}

void GuiRunner::AddBottom(GuiElement *element, int fade, int delay)
{
    AddElement(element);
    element->ElmAddLayers(manager, GUI_MAX_LAYERS, false, fade, delay);
}

void GuiRunner::Remove(GuiElement *element, int fade, int delay)
{
    RemoveElement(element);
    element->ElmRemoveLayers(manager, false, fade, delay);
}

void GuiRunner::RemoveAndDelete(GuiElement *element, int fade, int delay)
{
    RemoveElement(element);
    element->ElmRemoveLayers(manager, true, fade, delay);
}

/*-----------------*/

void GuiRunner::SetSelected(GuiElement *elm, int x, int y)
{
    if( selected_element != elm ) {
        if( selected_element != NULL ) {
            selected_element->ElmSetSelected(this, false, x, y);
        }
        if( elm != NULL ) {
            elm->ElmSetSelected(this, true, x, y);
            last_selected = elm;
        }
        selected_element = elm;
    }
}

GuiElement* GuiRunner::GetSelected(bool active_only)
{
    if( !active_only || use_keyboard || (is_above == selected_element) ) {
        return selected_element;
    }
    return NULL;
}

/*-----------------*/

GuiElement* GuiRunner::CheckCollision(Sprite *sprite)
{
    GRITEM *item = first_item;
    while( item != NULL ) {
        if( item->element->ElmSetSelectedOnCollision(this, sprite) ) {
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

GuiElement* GuiRunner::FindNearestElement(GuiElement *elm, GRDIR dir)
{
    GuiElement *rvalue = NULL;
    GRITEM *item = first_item;
    unsigned dist2 = (unsigned)-1;
    int x, y, w, h;
    elm->ElmGetRegion(this, &x, &y, &w, &h);
    while( item != NULL ) {
        int xx, yy, ww, hh;
        if( item->element != elm &&
            item->element->ElmGetRegion(this, &xx, &yy, &ww, &hh) )
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

bool GuiRunner::SelectNearestElement(GuiElement *elm, GRDIR dir)
{
    int x, y, w, h;
    GuiElement *select = NULL;
    if( elm != NULL && elm->ElmGetRegion(this, &x, &y, &w, &h) ) {
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

void GuiRunner::GetKeysCallback(KEY code, int pressed)
{
    GRITEM *item = first_item;
    while( item != NULL ) {
        if( item->element->ElmHandleKey(this, code, pressed) ) {
            use_keyboard = true;
            return;
        }
        item = item->next;
    }
    if( pressed &&
        last_selected != NULL ) {
        switch( code ) {
            case KEY_UP:
            case KEY_JOY1_UP:
            case KEY_JOY2_UP:
                if( SelectNearestElement(last_selected, GRDIR_UP) ) {
                    use_keyboard = true;
                }
                break;
            case KEY_DOWN:
            case KEY_JOY1_DOWN:
            case KEY_JOY2_DOWN:
                if( SelectNearestElement(last_selected, GRDIR_DOWN) ) {
                    use_keyboard = true;
                }
                break;
            case KEY_LEFT:
            case KEY_JOY1_LEFT:
            case KEY_JOY2_LEFT:
                if( SelectNearestElement(last_selected, GRDIR_LEFT) ) {
                    use_keyboard = true;
                }
                break;
            case KEY_RIGHT:
            case KEY_JOY1_RIGHT:
            case KEY_JOY2_RIGHT:
                if( SelectNearestElement(last_selected, GRDIR_RIGHT) ) {
                    use_keyboard = true;
                }
                break;
            default:
                break;
        }
    }
    dialog->OnKey(this, code, pressed);
}

void GuiRunner::GetKeysCallbackWrapper(void *context, KEY code, int pressed)
{
    GuiRunner *me = (GuiRunner *)context;
    me->GetKeysCallback(code, pressed);
}

void GuiRunner::Leave(void *retval)
{
    return_value = retval;
    quit = true;
}

void* GuiRunner::Run(void)
{
    quit = false;
    return_value = NULL;

    manager->Lock();

    // Cursor
    Sprite *cursor = new Sprite;
    cursor->SetImage(g_imgMousecursor);
    cursor->SetPosition(0, 0);
    cursor->SetVisible(false);
    manager->AddTopFixed(cursor);

    manager->Unlock();

    for(;;) {
        manager->Lock();
        cursor->SetVisible(false);
        manager->Unlock();

        // Check keys
        KBD_GetKeys(GetKeysCallbackWrapper, this);

        manager->Lock();

        // Infrared
        int x, y, angle;
        if( manager->GetWiiMoteIR(&x, &y, &angle) ) {
            cursor->SetPosition(x, y);
            cursor->SetRotation(angle/2);
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

        manager->Unlock();

        // Leave when requested
        if( quit ) {
            break;
        }

        dialog->OnUpdateScreen(this);
        VIDEO_WaitVSync();
    }

    // Cleanup
    manager->Remove(cursor);
    delete cursor;

    return return_value;
}

