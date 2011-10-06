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

#include "GuiDialog.h"

#include "../arch/archThread.h"

#include "GuiContainer.h"
#include "GuiElement.h"
#include "GuiRect.h"
#include "GuiSprite.h"

int GuiDialog::s_running_count = 0;
GuiSprite* GuiDialog::s_cursor = NULL;

GuiDialog::GuiDialog(GuiContainer *parent, const char *name,
                     float posx, float posy, float width, float height, float alpha) :
           GuiElement(parent, name, posx, posy, width, height, alpha)
{
    m_last_mouse_x = m_last_mouse_y = 0;
    m_is_active = false;
    m_return_value = NULL;
    m_quit = false;
}

GuiDialog::~GuiDialog()
{
    Deactivate();
}

// GuiDialog callbacks
//---------------------------

void GuiDialog::OnUpdateScreen(void)
{
}

// GuiElement callbacks
//---------------------------

bool GuiDialog::OnKey(GuiDialog *dlg, BTN key, bool pressed)
{
    bool handled = false;
    void *rval;
    if( pressed ) {
        switch( key ) {
            case BTN_JOY1_WIIMOTE_A:
            case BTN_JOY1_CLASSIC_A:
            case BTN_JOY2_WIIMOTE_A:
            case BTN_JOY2_CLASSIC_A:
            case BTN_RETURN:
            case BTN_SPACE:
                rval = GetFocusElement();
                if( rval != NULL ) {
                    Leave(rval);
                    handled = true;
                }
                break;
            case BTN_JOY1_WIIMOTE_B:
            case BTN_JOY1_CLASSIC_B:
            case BTN_JOY2_WIIMOTE_B:
            case BTN_JOY2_CLASSIC_B:
            case BTN_ESCAPE:
                Leave(NULL);
                handled = true;
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


// Public interface
//------------------

void* GuiDialog::Run(bool modal)
{
    m_quit = false;
    m_return_value = NULL;

    Activate(modal);

    for(;;) {
        // Check keys
        g_poGwd->input.GetButtonEvents(GetKeysCallbackWrapper, this);

        // Leave when requested
        if( m_quit ) {
            break;
        }
        archThreadSleep(20);
    }

    Deactivate();

    return m_return_value;
}

void GuiDialog::Leave(void *retval)
{
    m_return_value = retval;
    m_quit = true;
}


// Private functions
//-------------------

void GuiDialog::GetKeysCallback(BTN code, int pressed)
{
    (void)HandleKey(this, code, !!pressed);
}

void GuiDialog::GetKeysCallbackWrapper(void *context, BTN code, int pressed)
{
    GuiDialog *me = (GuiDialog *)context;
    me->GetKeysCallback(code, pressed);
}

bool GuiDialog::FrameCallbackWrapper(void *context)
{
    return ((GuiDialog *)context)->FrameCallback();
}

bool GuiDialog::FrameCallback(void)
{
    Lock();

    if( s_cursor == NULL ) {
        Unlock();
        return m_is_modal;
    }

    // Infrared
    int x, y, angle;
    if( g_poGwd->input.GetWiiMoteIR(&x, &y, &angle) ) {
        s_cursor->SetPosition(x, y);
        s_cursor->SetRotation((float)angle);
        if( !s_cursor->IsVisible() ) {
            s_cursor->SetVisible(true);
        }
        if( x != m_last_mouse_x || y != m_last_mouse_y ) {
            // Position changed, call handler
            HandleMouseMove((float)x, (float)y);
            m_last_mouse_x = x;
            m_last_mouse_y = y;
        }
    }else{
        s_cursor->SetVisible(false);
        s_cursor->SetPosition(0, 0);
        m_last_mouse_x = m_last_mouse_y = -1;
    }
    
    OnUpdateScreen();

    Unlock();
    return m_is_modal;
}

void GuiDialog::Activate(bool modal)
{
    m_is_modal = modal;

    Lock();

    if( m_is_active ) {
        Unlock();
        return;
    }
    m_is_active = true;
    SetFocus(true);

    // Cursor
    if( s_running_count == 0 ) {
        GuiImage *image;
        GuiRect rect;
        GetPointerImage(&image, &rect);
        s_cursor = new GuiSprite(GetRootContainer(), "cursor", NULL, 0, 0);
        s_cursor->SetImage(image, rect);
        s_cursor->SetVisible(false);
        GetRootContainer()->AddTopFixed(s_cursor);
    }
    s_running_count++;

    GetRootContainer()->AddFrameCallback(FrameCallbackWrapper, this);

    Unlock();
}

void GuiDialog::Deactivate(void)
{
    Lock();

    if( !m_is_active ) {
        Unlock();
        return;
    }
    m_is_active = false;
    SetFocus(false);

    GetRootContainer()->RemoveFrameCallback(FrameCallbackWrapper, this);

    s_running_count--;

    // Cleanup
    if( s_running_count == 0 ) {
        GetRootContainer()->RemoveAndDelete(s_cursor);
        s_cursor = NULL;
    }

    Unlock();
}

