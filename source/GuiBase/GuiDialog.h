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
#ifndef _GUI_DIALOG_H
#define _GUI_DIALOG_H

#include "GuiRootContainer.h"
#include "GuiElement.h"

class GuiDialog;
class GuiSprite;
class GuiRect;

class GuiDialog : public GuiElement {
public:
    GuiDialog(GuiContainer *parent, const char *name, float posx=0, float posy=0, float width=0, float height=0, float alpha=1.0f);
    virtual ~GuiDialog();

    // GuiDialog callbacks
    virtual void OnUpdateScreen(void);

    // GuiElement callbacks
    virtual bool OnKey(GuiDialog *dlg, BTN key, bool pressed);

    // GuiDialog
    void* Run(bool modal = true);
    void Leave(void *retval);

private:
    void AddDialog(GuiDialog *dialog);
    void RemoveDialog(GuiDialog *dialog);
    void Activate(bool modal = false);
    void Deactivate(void);

    static void GetKeysCallbackWrapper(void *context, BTN code, int pressed);
    static bool FrameCallbackWrapper(void *context);
    void GetKeysCallback(BTN code, int pressed);
    bool FrameCallback(void);

    static int s_running_count;
    static GuiSprite *s_cursor;
    int m_last_mouse_x, m_last_mouse_y;
    bool m_is_modal;
    bool m_is_active;
    bool m_quit;
    void *m_return_value;
};

#endif
