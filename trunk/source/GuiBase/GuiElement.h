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
#ifndef _GUI_ELEMENT_H
#define _GUI_ELEMENT_H

#include "InputDevices.h"
#include "GuiContainer.h"

typedef enum {
  GRDIR_UP,
  GRDIR_DOWN,
  GRDIR_LEFT,
  GRDIR_RIGHT,
  GRDIR_ANY
} GRDIR;

class GuiElement;
class GuiDialog;
class GuiEffect;
class GuiLayer;
class GuiSprite;
class GuiRect;

typedef std::list<GuiElement*>::iterator ElementIndex;

class GuiElement : public GuiContainer {
public:
    GuiElement(GuiContainer *parent, const char *name,
               float posx=0, float posy=0, float width=0, float height=0, float alpha=1.0f);
    GuiElement(GuiElement *parent, const char *name,
               float posx=0, float posy=0, float width=0, float height=0, float alpha=1.0f);
    virtual ~GuiElement();

    // GuiContainer (overloaded)
    virtual void AddTop(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddTop(GuiElement *element, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddTopFixed(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddTopFixed(GuiElement *element, GuiEffect *effect = NULL, bool queue = false);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiElement *element, GuiEffect *effect = NULL, bool queue = false);
    virtual bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual bool AddBehind(GuiLayer *behind, GuiElement *element, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddBottom(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddBottom(GuiElement *elementr, GuiEffect *effect = NULL, bool queue = false);
    virtual void Remove(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void Remove(GuiElement *element, GuiEffect *effect = NULL, bool queue = false);
    virtual void RemoveAndDelete(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void RemoveAndDelete(GuiElement *element, GuiEffect *effect = NULL, bool queue = false);
    inline void AddTop(GuiLayer *layer, GuiEffect &effect, bool queue = false) { AddTop(layer, effect.Clone(), queue); }
    inline void AddTop(GuiElement *element, GuiEffect &effect, bool queue = false) { AddTop(element, effect.Clone(), queue); }
    inline void AddTopFixed(GuiLayer *layer, GuiEffect &effect, bool queue = false) { AddTopFixed(layer, effect.Clone(), queue); }
    inline void AddTopFixed(GuiElement *element, GuiEffect &effect, bool queue = false) { AddTopFixed(element, effect.Clone(), queue); }
    inline bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect &effect, bool queue = false) { return AddOnTopOf(ontopof, layer, effect.Clone(), queue); }
    inline bool AddOnTopOf(GuiLayer *ontopof, GuiElement *element, GuiEffect &effect, bool queue = false) { return AddOnTopOf(ontopof, element, effect.Clone(), queue); }
    inline bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect &effect, bool queue = false) { return AddBehind(behind, layer, effect.Clone(), queue); }
    inline bool AddBehind(GuiLayer *behind, GuiElement *element, GuiEffect &effect, bool queue = false) { return AddBehind(behind, element, effect.Clone(), queue); }
    inline void AddBottom(GuiLayer *layer, GuiEffect &effect, bool queue = false) { AddBottom(layer, effect.Clone(), queue); }
    inline void AddBottom(GuiElement *element, GuiEffect &effect, bool queue = false) { AddBottom(element, effect.Clone(), queue); }
    inline void Remove(GuiLayer *layer, GuiEffect &effect, bool queue = false) { Remove(layer, effect.Clone(), queue); }
    inline void Remove(GuiElement *element, GuiEffect &effect, bool queue = false) { Remove(element, effect.Clone(), queue); }
    inline void RemoveAndDelete(GuiLayer *layer, GuiEffect &effect, bool queue = false) { RemoveAndDelete(layer, effect.Clone(), queue); }
    inline void RemoveAndDelete(GuiElement *element, GuiEffect &effect, bool queue = false) { RemoveAndDelete(element, effect.Clone(), queue); }

    // GuiContainer callbacks
    virtual void OnDelete(GuiAtom *atom);

    // GuiElement callbacks
    virtual void OnFocus(bool focus);
    virtual void OnActive(bool active);
    virtual void OnSelect(GuiElement *element);
    virtual bool OnKey(GuiDialog *dlg, BTN key, bool pressed);
    virtual void OnMouseMove(float x, float y);
    virtual void OnMouseLost(void);
    virtual bool OnTestActiveArea(float x, float y);

    // GuiElement overloadable
    virtual void GetActiveAreaRect(GuiRect *rect);

    // GuiElement
    void SetFocus(bool focus);
    GuiElement* GetFocusElement(void) { return m_focus_element; }
    bool GetTopFocusRect(GuiRect *rect);
    void SetActiveElement(GuiElement *elm);
    bool HandleKey(GuiDialog *dlg, BTN key, bool pressed);
    void HandleMouseMove(float x, float y);
    bool HasFocus(void) { return m_has_focus; }
    bool IsInActiveArea(float x, float y, GuiElement **elm = NULL);

private:
    bool m_has_focus;
    std::list<GuiElement*> m_element_list;
    GuiElement *m_parent_element;
    GuiElement *m_active_element;
    GuiElement *m_focus_element;
    GuiElement *m_is_above;

    // Private functions
    void AddElement(GuiElement *element);
    void RemoveElement(GuiElement *element);
    void ClaimFocus(GuiElement *elm);
    bool SelectNearestElement(GuiRect rect, GuiElement *notthis, GRDIR dir, unsigned *distance = NULL);
    bool SelectNearestElement(GRDIR dir);
};

#endif
