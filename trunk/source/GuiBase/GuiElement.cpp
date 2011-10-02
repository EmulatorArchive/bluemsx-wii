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

#include <assert.h>

#include "GuiElement.h"
#include "GuiRect.h"

GuiElement::GuiElement(GuiContainer *parent, const char *name,
                       float posx, float posy, float width, float height, float alpha)
          : GuiContainer(parent, name, posx, posy, width, height, alpha)
{
    m_active_element = NULL;
    m_parent_element = NULL;
    m_focus_element = NULL;
    m_is_above = NULL;
    m_has_focus = false;
}

GuiElement::GuiElement(GuiElement *parent, const char *name,
                       float posx, float posy, float width, float height, float alpha)
          : GuiContainer(parent, name, posx, posy, width, height, alpha)
{
    m_active_element = NULL;
    m_parent_element = parent;
    m_focus_element = NULL;
    m_is_above = NULL;
    m_has_focus = false;
}

GuiElement::~GuiElement()
{
}

// GuiContainer overloads
//---------------------------

void GuiElement::AddTop(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    GuiContainer::AddTop(layer, effect, queue);
}
void GuiElement::AddTop(GuiElement *element, GuiEffect *effect, bool queue)
{
    GuiContainer::AddTop(element, effect, queue);
    AddElement(element);
}

void GuiElement::AddTopFixed(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    GuiContainer::AddTopFixed(layer, effect, queue);
}
void GuiElement::AddTopFixed(GuiElement *element, GuiEffect *effect, bool queue)
{
    GuiContainer::AddTopFixed(element, effect, queue);
    AddElement(element);
}

bool GuiElement::AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect, bool queue)
{
    return GuiContainer::AddOnTopOf(ontopof, layer, effect, queue);
}
bool GuiElement::AddOnTopOf(GuiLayer *ontopof, GuiElement *element, GuiEffect *effect, bool queue)
{
    if( GuiContainer::AddOnTopOf(ontopof, element, effect, queue) ) {
        AddElement(element);
        return true;
    }
    return false;
}

bool GuiElement::AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect, bool queue)
{
    return GuiContainer::AddBehind(behind, layer, effect, queue);
}
bool GuiElement::AddBehind(GuiLayer *behind, GuiElement *element, GuiEffect *effect, bool queue)
{
    if( GuiContainer::AddBehind(behind, element, effect, queue) ) {
        AddElement(element);
        return true;
    }
    return false;
}

void GuiElement::AddBottom(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    GuiContainer::AddBottom(layer, effect, queue);
}
void GuiElement::AddBottom(GuiElement *element, GuiEffect *effect, bool queue)
{
    GuiContainer::AddBottom(element, effect, queue);
    AddElement(element);
}

void GuiElement::Remove(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    GuiContainer::Remove(layer, effect, queue);
}
void GuiElement::Remove(GuiElement *element, GuiEffect *effect, bool queue)
{
    RemoveElement(element);
    GuiContainer::Remove(element, effect, queue);
}

void GuiElement::RemoveAndDelete(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    GuiContainer::RemoveAndDelete(layer, effect, queue);
}
void GuiElement::RemoveAndDelete(GuiElement *element, GuiEffect *effect, bool queue)
{
    RemoveElement(element);
    GuiContainer::RemoveAndDelete(element, effect, queue);
}

// GuiContainer callbacks
//---------------------------

void GuiElement::OnDelete(GuiAtom *atom)
{
    if( atom == m_focus_element ) {
        m_focus_element = NULL;
    }
    if( atom == m_active_element ) {
        m_active_element = NULL;
    }
    if( atom == m_is_above ) {
        m_is_above = NULL;
    }
}

// GuiElement callbacks
//---------------------------

void GuiElement::OnFocus(bool focus)
{
}

void GuiElement::OnActive(bool active)
{
}

void GuiElement::OnSelect(GuiElement *element)
{
}

bool GuiElement::OnKey(GuiDialog *dlg, BTN key, bool pressed)
{
    bool handled = false;
    // Handle selection movement (arrow keys)
    if( m_has_focus &&
        pressed &&
        m_focus_element != NULL ) {
        switch( key ) {
            case BTN_UP:
            case BTN_JOY1_UP:
            case BTN_JOY2_UP:
                if( SelectNearestElement(GRDIR_UP) ) {
                    handled = true;
                }
                break;
            case BTN_DOWN:
            case BTN_JOY1_DOWN:
            case BTN_JOY2_DOWN:
                if( SelectNearestElement(GRDIR_DOWN) ) {
                    handled = true;
                }
                break;
            case BTN_LEFT:
            case BTN_JOY1_LEFT:
            case BTN_JOY2_LEFT:
                if( SelectNearestElement(GRDIR_LEFT) ) {
                    handled = true;
                }
                break;
            case BTN_RIGHT:
            case BTN_JOY1_RIGHT:
            case BTN_JOY2_RIGHT:
                if( SelectNearestElement(GRDIR_RIGHT) ) {
                    handled = true;
                }
                break;
            default:
                break;
        }
    }
    return handled;
}

void GuiElement::OnMouseMove(float x, float y)
{
}

void GuiElement::OnMouseLost(void)
{
}

bool GuiElement::OnTestActiveArea(float x, float y)
{
    return false; // no active area by default
}

// GuiElement overloadable
//---------------------------

void GuiElement::GetActiveAreaRect(GuiRect *rect)
{
    rect->x = GetX();
    rect->y = GetY();
    rect->width = GetWidth();
    rect->height = GetHeight();
}

// GuiElement
//---------------------------

void GuiElement::SetFocus(bool focus)
{
    if( focus != m_has_focus ) {
        if( focus ) {
            // claim focus
            if( m_parent_element != NULL ) {
                m_parent_element->ClaimFocus(this);
            }
            OnFocus(true);
        }else{
            // remove focus from children
            if( m_focus_element != NULL ) {
                m_focus_element->SetFocus(false);
                m_focus_element = NULL;
            }
            OnFocus(false);
        }
        m_has_focus = focus;
    }
}

bool GuiElement::GetTopFocusRect(GuiRect *rect)
{
    if( m_has_focus ) {
        if( m_focus_element != NULL && m_focus_element->GetTopFocusRect(rect) ) {
            rect->x += GetX();
            rect->y += GetY();
            return true;
        }
        GetActiveAreaRect(rect);
        return true;
    }
    return false;
}

void GuiElement::SetActiveElement(GuiElement *elm)
{
    if( m_active_element != elm ) {
        if( m_active_element != NULL ) {
            m_active_element->OnActive(false);
        }
        if( elm != NULL ) {
            elm->OnActive(true);
        }
        m_active_element = elm;
        OnSelect(elm);
    }
}

bool GuiElement::HandleKey(GuiDialog *dlg, BTN key, bool pressed)
{
    // First check if some any of the children handles it
    ElementIndex elm_it;
    for(elm_it = m_element_list.begin(); elm_it != m_element_list.end(); ++elm_it) {
        if( (*elm_it)->OnKey(dlg, key, pressed) ) {
            return true;
        }
    }
    return OnKey(dlg, key, pressed);
}

void GuiElement::HandleMouseMove(float x, float y)
{
    // Check if mouse cursor is above something
    GuiElement *element = NULL;
    (void)IsInActiveArea(x, y, &element);

    // Switch between elements
    if( element != m_is_above ) {
        if( m_is_above != NULL ) {
            m_is_above->OnMouseLost();
        }
        m_is_above = element;
    }

    // Select element if mouse is above it
    if( m_is_above != NULL ) {
        // above child, delegate
        m_is_above->HandleMouseMove(x - GetX(), y - GetY());
    }else{
        // we are the topmost element at the cursor location, get focus
        SetFocus(true);
    }
    OnMouseMove(x, y);
}

bool GuiElement::IsInActiveArea(float x, float y, GuiElement **elm)
{
    // First check if the coordinate is within our client rect
    GuiRect rect;
    GetActiveAreaRect(&rect);
    if( x < rect.x || x > rect.x + rect.width ||
        y < rect.y || y > rect.y + rect.height ) {
        return false;
    }

    // Now test if it hits one of our childs
    ElementIndex it;
    for(it = m_element_list.begin(); it != m_element_list.end(); ++it) {
        if( (*it)->IsInActiveArea(x - GetX(), y - GetY(), elm) ) {
            if( elm != NULL ) {
                *elm = *it;
            }
            return true;
        }
    }
    return OnTestActiveArea(x, y);
}


// Private functions
//-------------------

void GuiElement::AddElement(GuiElement *element)
{
    if( element != NULL ) {
        m_element_list.push_back(element);
    }
}

void GuiElement::RemoveElement(GuiElement *element)
{
    ElementIndex it;
    for(it = m_element_list.begin(); it != m_element_list.end(); ++it) {
        if( *it == element ) {
            m_element_list.erase(it);
            break;
        }
    }
}

void GuiElement::ClaimFocus(GuiElement *elm)
{
    assert( elm != NULL );
    if( elm != m_focus_element ) {
        if( m_focus_element != NULL ) {
            m_focus_element->SetFocus(false);
        }
        m_has_focus = true;
        m_focus_element = elm;
        if( m_parent_element != NULL ) {
            m_parent_element->ClaimFocus(this);
        }
        SetActiveElement(elm);
    }
}

bool GuiElement::SelectNearestElement(GuiRect rect, GuiElement *notthis, GRDIR dir, unsigned *distance)
{
    GuiElement *element = NULL;
    unsigned dist2 = (unsigned)-1;

    rect.x -= GetX();
    rect.y -= GetY();

    // Find nearest element
    ElementIndex elm_it;
    for(elm_it = m_element_list.begin(); elm_it != m_element_list.end(); ++elm_it) {
        GuiElement *elm = *elm_it;
        if( elm != notthis && elm->IsVisible() ) {
            GuiRect elmrect;
            elm->GetActiveAreaRect(&elmrect);
            if( (dir == GRDIR_UP    && elmrect.y + elmrect.height / 2 < rect.y + rect.height/2) ||
                (dir == GRDIR_DOWN  && elmrect.y + elmrect.height / 2 > rect.y + rect.height/2) ||
                (dir == GRDIR_LEFT  && elmrect.x + elmrect.width / 2 < rect.x + rect.width/2) ||
                (dir == GRDIR_RIGHT && elmrect.x + elmrect.width / 2 > rect.x + rect.width/2) ||
                (dir == GRDIR_ANY) )
            {
                float dx, dy;
                unsigned d;
                if( elmrect.x < rect.x ) {
                    dx = rect.x - elmrect.x;
                }else if( elmrect.x > rect.x+rect.width ) {
                    dx = elmrect.x - (rect.x+rect.width);
                }else{
                    dx = 0;
                }
                if( elmrect.y < rect.y ) {
                    dy = rect.y - elmrect.y;
                }else if( elmrect.y > rect.y+rect.height ) {
                    dy = elmrect.y - (rect.y+rect.height);
                }else{
                    dy = 0;
                }
                d = (unsigned)(dx * dx + dy * dy);
                if( d < dist2 ) {
                    element = *elm_it;
                    dist2 = d;
                }
            }
        }
    }

    if( element != NULL && distance != NULL ) {
        *distance = dist2;
    }

    if( element != NULL ) {
        (void)element->SelectNearestElement(rect, NULL, GRDIR_ANY);
        return true;
    }else{
        SetFocus(true);
        return false;
    }
}

bool GuiElement::SelectNearestElement(GRDIR dir)
{
    GuiRect rect;
    if( GetTopFocusRect(&rect) ) {
        return SelectNearestElement(rect, m_focus_element, dir);
    }
    return false;
}

