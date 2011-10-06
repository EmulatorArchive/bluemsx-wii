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
#ifndef _GUIBASE_CONTAINER_H
#define _GUIBASE_CONTAINER_H

#define MAX_LAYERS 256

#include <list>
#include "GuiEffect.h"
#include "GuiLayer.h"
#include "GuiRect.h"

class GuiRootContainer;
class GuiImage;

typedef struct _GuiContainerCallback {
    bool (*callback)(void*);
    void *context;
} GuiContainerCallback;

typedef enum {
    EP_NONE,
    EP_HIDE,
    EP_REMOVE
} LayerEffectPostAction;

typedef struct _LayerEffect {
    bool active;
    GuiEffectType type;
    LayerEffectPostAction postaction;
    GuiEffect *effect;
    GuiLayer  *layer;
} LayerEffect;

typedef std::list<GuiLayer*>::iterator LayerIndex;

class GuiContainer : public GuiLayer
{
public:
    GuiContainer(GuiContainer *parent, const char *name,
                 float posx=0, float posy=0, float width=0, float height=0, float alpha=1.0f);
    virtual ~GuiContainer();
    void Destructor();

    // Callbacks
    virtual void OnDelete(GuiAtom *atom) {};

    // Utility
    void Lock(void);
    void Unlock(void);
    GuiContainer* GetParentContainer(void);
    void SetPointerImage(GuiImage *image, GuiRect rect = GuiRect());
    void GetPointerImage(GuiImage **image, GuiRect *rect);

    // Render/Frame callbacks
    void AddRenderCallback(bool (*callback)(void*), void *context);
    void RemoveRenderCallback(bool (*callback)(void*), void *context);
    void AddFrameCallback(bool (*callback)(void*), void *context);
    void RemoveFrameCallback(bool (*callback)(void*), void *context);

    // GuiLayer
    virtual bool IsInVisibleArea(float x, float y);
    virtual bool IsInVisibleArea(GuiLayer *layer);
    virtual bool IsBusy(void);
    virtual void Draw(void);

    // Layer order management
    virtual void AddTop(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddTopFixed(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void AddBottom(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void Remove(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void RemoveAndDelete(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false);
    virtual void Show(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false, LayerTransform *transform = NULL);
    virtual void Hide(GuiLayer *layer, GuiEffect *effect = NULL, bool queue = false, LayerTransform *transform = NULL);
    virtual void Show(GuiLayer *layer, GuiEffect *effect, bool queue, float offset_x, float offset_y,
                      float zoom_x = 1.0f, float zoom_y = 1.0f, float rot = 0.0f, float alpha = 0.0f);
    virtual void Hide(GuiLayer *layer, GuiEffect *effect, bool queue, float offset_x, float offset_y,
                      float zoom_x = 1.0f, float zoom_y = 1.0f, float rot = 0.0f, float alpha = 0.0f);
    virtual void Morph(GuiLayer *from, GuiLayer *to, GuiEffect *effectFrom = NULL, GuiEffect *effectTo = NULL, bool queue = false);

    inline void AddTop(GuiLayer *layer, GuiEffect &effect, bool queue = false) { AddTop(layer, effect.Clone(), queue); }
    inline void AddTopFixed(GuiLayer *layer, GuiEffect &effect, bool queue = false) { AddTopFixed(layer, effect.Clone(), queue); }
    inline bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect &effect, bool queue = false)
                           { return AddOnTopOf(ontopof, layer, effect.Clone(), queue); }
    inline bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect &effect, bool queue = false)
                          { return AddBehind(behind, layer, effect.Clone(), queue); }
    inline void AddBottom(GuiLayer *layer, GuiEffect &effect, bool queue = false) { AddBottom(layer, effect.Clone(), queue); }
    inline void Remove(GuiLayer *layer, GuiEffect &effect, bool queue = false) { Remove(layer, effect.Clone(), queue); }
    inline void RemoveAndDelete(GuiLayer *layer, GuiEffect &effect, bool queue = false) { RemoveAndDelete(layer, effect.Clone(), queue); }
    inline void Show(GuiLayer *layer, GuiEffect &effect, bool queue = false) { Show(layer, effect.Clone(), queue); }
    inline void Hide(GuiLayer *layer, GuiEffect &effect, bool queue = false) { Hide(layer, effect.Clone(), queue); }
    inline void Show(GuiLayer *layer, GuiEffect &effect, bool queue, float offset_x, float offset_y,
                     float zoom_x = 1.0f, float zoom_y = 1.0f, float rot = 0.0f, float alpha = 0.0f)
                     { Show(layer, effect.Clone(), queue, offset_x, offset_y, zoom_x, zoom_y, rot, alpha); }
    inline void Hide(GuiLayer *layer, GuiEffect &effect, bool queue, float offset_x, float offset_y,
                     float zoom_x = 1.0f, float zoom_y = 1.0f, float rot = 0.0f, float alpha = 0.0f)
                     { Hide(layer, effect.Clone(), queue, offset_x, offset_y, zoom_x, zoom_y, rot, alpha); }
    inline void Morph(GuiLayer *from, GuiLayer *to, GuiEffect &effectFrom, GuiEffect &effectTo, bool queue = false)
                     { Morph(from, to, effectFrom.Clone(), effectTo.Clone(), queue); }

    virtual void Delete(GuiAtom *atom);

private:
    // Internal layer administration
    void ActivateEffect(GuiLayer *layer);
    bool CancelEffectsInProgress(GuiLayer *layer, bool allow_remove, bool *queue, LayerTransform *transform);
    void LayerAdd(LayerIndex index, bool movenonfixed, GuiLayer *layer, GuiEffect *effect, bool queue, LayerTransform *transform);
    void LayerRemove(GuiLayer* layer, GuiEffect *effect = NULL, bool queue = false, LayerTransform *transform = NULL);
    void LayerRemoveAll();
    void DeleteAllEffects();
    LayerIndex LayerGetIndex(GuiLayer* layer);

    GuiContainer *_parent;
    GuiImage* pointer_image;
    GuiRect pointer_image_clip;
    bool stop_requested;

    LayerIndex first_nonfixed;
    std::list<GuiLayer*> layers;
    std::list<LayerEffect> effect_list;
    std::list<GuiContainerCallback> render_callback;
    std::list<GuiContainerCallback> frame_callback;
};

#endif
