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

#include "GuiContainer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "GuiEffectFade.h"
#include "GuiRootContainer.h"

GuiContainer::GuiContainer(GuiContainer *parent, const char *name,
                           float posx, float posy, float width, float height, float alpha)
            : GuiLayer(parent, name)
{
    if( parent != NULL ) {
        _parent = parent;
    }else{
        _parent = GetRootContainer();
    }

    if( _parent != NULL ) {
        SetWidth(width > 0? width : _parent->GetWidth());
        SetHeight(height > 0? height : _parent->GetHeight());
        SetPointerImage(_parent->GetPointerImage());
    }else{
        SetWidth(0);
        SetHeight(0);
        SetPointerImage(NULL);
    }
    SetPosition(posx, posy);

    first_nonfixed = layers.end();
    stop_requested = false;
}

void GuiContainer::Destructor()
{
    Lock();

    // Delete effects in progress
    DeleteAllEffects();

    // Remove layers not deleted yet
    LayerRemoveAll();

    Unlock();
}

GuiContainer::~GuiContainer()
{
    Destructor();
}


//--------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------

void GuiContainer::Lock(void)
{
    GuiRootContainer *root = GetRootContainer();
    if( root != NULL ) {
        root->Lock();
    }
}

void GuiContainer::Unlock(void)
{
    GuiRootContainer *root = GetRootContainer();
    if( root != NULL ) {
        root->Unlock();
    }
}

GuiContainer* GuiContainer::GetParentContainer(void)
{
    return _parent;
}

void GuiContainer::SetPointerImage(GuiImage *image)
{
    pointer_image = image;
}

GuiImage* GuiContainer::GetPointerImage(void)
{
    return pointer_image;
}


//--------------------------------------------------------------------------
// Render/Frame callbacks
//--------------------------------------------------------------------------

void GuiContainer::AddRenderCallback(bool (*callback)(void*), void *context)
{
    GuiContainerCallback newrender;
    newrender.callback = callback;
    newrender.context = context;

    Lock();
    render_callback.push_back(newrender);
    Unlock();
}

void GuiContainer::RemoveRenderCallback(bool (*callback)(void*), void *context)
{
    Lock();

    std::list<GuiContainerCallback>::iterator it;
    for(it = render_callback.begin(); it != render_callback.end(); ++it) {
        if( (*it).context == context ) {
            render_callback.erase(it);
            Unlock();
            return;
        }
    }

    Unlock();
}

void GuiContainer::AddFrameCallback(bool (*callback)(void*), void *context)
{
    GuiContainerCallback newrender;
    newrender.callback = callback;
    newrender.context = context;

    Lock();

    frame_callback.push_back(newrender);

    Unlock();
}

void GuiContainer::RemoveFrameCallback(bool (*callback)(void*), void *context)
{
    Lock();

    std::list<GuiContainerCallback>::iterator it;
    for(it = frame_callback.begin(); it != frame_callback.end(); ++it) {
        if( (*it).callback == callback && (*it).context == context ) {
            frame_callback.erase(it);
            Unlock();
            return;
        }
    }

    Unlock();
}


//--------------------------------------------------------------------------
// Override Delete in order to implement callback on it
//--------------------------------------------------------------------------

void GuiContainer::Delete(GuiAtom *atom)
{
    if( atom != NULL ) {
        Lock();
        OnDelete(atom);
        GetRootContainer()->ReleaseAtom(atom);
        Unlock();
    }
}


//--------------------------------------------------------------------------
// Layer effect operations
//--------------------------------------------------------------------------

void GuiContainer::DeleteAllEffects()
{
    Lock();

    std::list<LayerEffect>::iterator it = effect_list.begin();
    while( it != effect_list.end() ) {
        LayerEffect ef = *it;
        // Free item from list
        effect_list.erase(it++);
        // When it is a remove effect, do the remove actions
        if( ef.postaction == EP_REMOVE ) {
            LayerRemove(ef.layer);
        }
        // Clear the old effect
        if( ef.effect != NULL ) {
            Delete(ef.effect);
        }
    }

    Unlock();
}

void GuiContainer::LayerRemoveAll()
{
    Lock();

    LayerIndex it;
    while( (it = layers.begin()) != layers.end() ) {
        GuiLayer *layer = *it;
        Remove(layer);
    }

    Unlock();
}

void GuiContainer::ActivateEffect(GuiLayer *layer)
{
    // Activate the first inactive effect for this layer, used for queuing
    std::list<LayerEffect>::iterator it = effect_list.begin();
    while( it != effect_list.end() ) {
        if( (*it).layer && (*it).layer == layer ) {
            LayerEffect &ef = *it;
            if( !ef.active ) {
                // found an inactive effect, activate it now
                ef.active = true;
                break;
            }
        }
        it++;
    }
}

void GuiContainer::CancelEffectsInProgress(GuiLayer *layer, bool allow_remove, bool *queue, LayerTransform *transform)
{
    // Check for pending actions with this layer in effect-list
    LayerEffect ef;
    bool found = false;
    std::list<LayerEffect>::iterator it = effect_list.begin();
    while( it != effect_list.end() ) {
        if( (*it).layer && (*it).layer == layer )
        {
            if( *queue ) {
                // found, indeed needs to be queued
                return;
            }else{
                // found, remove and cancel effect
                if( (*it).active ) {
                    // active one, this is the one we must cancel
                    ef = *it;
                    found = true;
                }else{
                    // inactive, delete directly
                    Delete((*it).effect);
                }
                effect_list.erase(it++);
                continue;
            }
        }
        it++;
    }

    if( *queue ) {
        // requested for queuing but not found, no need to queue
        *queue = false;
        return;
    }

    if( found ) {
        // Cancel the old effect
        ef.effect->Cancel(transform);
        // When it is a remove effect, do the remove action
        if( allow_remove ) {
            if( ef.postaction == EP_REMOVE ) {
                LayerRemove(ef.layer);
            }
        }else{
            assert( ef.postaction != EP_REMOVE );
        }
        // Clear the old effect
        Delete(ef.effect);
    }
}

void GuiContainer::LayerAdd(LayerIndex index, bool movenonfixed, GuiLayer *layer, GuiEffect *effect, bool queue, LayerTransform *transform)
{
    // add layer
    GuiRootContainer::UseAtom(layer);
    layers.insert(index, layer);
    if( movenonfixed && first_nonfixed == index ) {
        --first_nonfixed;
    }

    if( effect != NULL ) {
        // Register the effect
        LayerEffect ef;
        ef.active = !queue;
        ef.type = ET_SHOW;
        ef.postaction = EP_NONE;
        ef.layer = layer;
        ef.effect = effect;
        effect_list.push_back(ef);

        // Initialize the effect
        effect->Initialize(layer, ef.type, *transform);
    }
}

void GuiContainer::LayerRemove(GuiLayer *layer, GuiEffect *effect, bool queue, LayerTransform *transform)
{

    assert(layer);

    if( effect != NULL ) {
        // Add entry to list
        LayerEffect ef;
        ef.active = !queue;
        ef.type = ET_HIDE;
        ef.postaction = EP_REMOVE;
        ef.layer = layer;
        ef.effect = effect;
        effect_list.push_back(ef);

        // Initialize the new effect
        effect->Initialize(layer, ef.type, *transform);
    }
    else
    {
        // No effect, remove directly
        LayerIndex it;
        for(it = layers.begin(); it != layers.end(); ++it) {
            if( *it == layer ) {
                if( it == first_nonfixed ) {
                    ++first_nonfixed;
                }
                layers.erase(it);
                GetRootContainer()->ReleaseAtom(layer);
                break;
            }
        }
    }
}

void GuiContainer::Show(GuiLayer *layer, GuiEffect *effect, bool queue, LayerTransform *transform)
{
    bool effect_busy = false;
 
    assert( layer != NULL );

    Lock();

    // Check for pending actions with this layer in effect-list
    LayerTransform old_transform;
    CancelEffectsInProgress(layer, false, &queue, &old_transform);
    (void)old_transform;

    if( layer->IsVisible() && !effect_busy ) {
        // No need to do anything, bail out
        Delete(effect); // We don't use it so we need to delete it
        Unlock();
        return;
    }

    // Immediately visible, effect will handle visibility using alpha channel
    layer->SetVisible(true);

    if( effect != NULL ) {
        // Register the effect
        LayerEffect ef;
        ef.active = !queue;
        ef.type = ET_SHOW;
        ef.postaction = EP_NONE;
        ef.layer = layer;
        ef.effect = effect;
        effect_list.push_back(ef);

        // Initialize the effect
        effect->Initialize(layer, ef.type, transform? *transform : LayerTransform());
    }

    Unlock();
}

void GuiContainer::Hide(GuiLayer *layer, GuiEffect *effect, bool queue, LayerTransform *transform)
{
    LayerEffectPostAction postaction = EP_HIDE;

    Lock();

    // Check for pending actions with this layer in effect-list
    LayerTransform old_transform;
    CancelEffectsInProgress(layer, false, &queue, &old_transform);

    if( effect != NULL ) {
        // Register the effect
        LayerEffect ef;
        ef.active = !queue;
        ef.type = ET_HIDE;
        ef.postaction = postaction;
        ef.layer = layer;
        ef.effect = effect;
        effect_list.push_back(ef);

        // Initialize the effect
        effect->Initialize(layer, ef.type, old_transform, transform? *transform : LayerTransform());
    }else{
        // Hide immediately
        if( postaction == EP_REMOVE ) {
            // Must be removed instead
            LayerRemove(layer);
        }else{
            layer->SetVisible(false);
        }
    }

    Unlock();
}

void GuiContainer::Show(GuiLayer *layer, GuiEffect *effect, bool queue, float offset_x, float offset_y,
                        float zoom_x, float zoom_y, float rot, float alpha)
{
    LayerTransform tr;
    tr.valid = true;
    tr.offsetX = offset_x;
    tr.offsetY = offset_y;
    tr.stretchWidth = zoom_x;
    tr.stretchHeight = zoom_y;
    tr.rotation = rot;
    tr.alpha = alpha;
    Show(layer, effect, queue, &tr);
}

void GuiContainer::Hide(GuiLayer *layer, GuiEffect *effect, bool queue, float offset_x, float offset_y,
                        float zoom_x, float zoom_y, float rot, float alpha)
{
    LayerTransform tr;
    tr.valid = true;
    tr.offsetX = offset_x;
    tr.offsetY = offset_y;
    tr.stretchWidth = zoom_x;
    tr.stretchHeight = zoom_y;
    tr.rotation = rot;
    tr.alpha = alpha;
    Hide(layer, effect, queue, &tr);
}

void GuiContainer::Morph(GuiLayer *from, GuiLayer *to, GuiEffect *effectFrom, GuiEffect *effectTo, bool queue)
{
    if( from != NULL ) {
        LayerTransform tr;
        if( to != NULL ) {
            tr.valid = true;
            tr.alpha = 0.0f;
            tr.offsetX = to->GetX() - from->GetX();
            tr.offsetY = to->GetY() - from->GetY();
            tr.stretchWidth = (float)to->GetWidth() / (float)from->GetWidth();
            tr.stretchHeight = (float)to->GetHeight() / (float)from->GetHeight();
        }
        Hide(from, effectFrom, queue, &tr);
    }else{
        Delete(effectFrom); // unused effect must be deleted
    }
    if( to != NULL ) {
        LayerTransform tr;
        if( from != NULL ) {
            tr.valid = true;
            tr.alpha = 0.0f;
            tr.offsetX = from->GetX() - to->GetX();
            tr.offsetY = from->GetY() - to->GetY();
            tr.stretchWidth = (float)from->GetWidth() / (float)to->GetWidth();
            tr.stretchHeight = (float)from->GetHeight() / (float)to->GetHeight();
        }
        Show(to, effectTo, queue, &tr);
    }else{
        Delete(effectTo); // unused effect must be deleted
    }
}

//--------------------------------------------------------------------------
// Layer order management
//--------------------------------------------------------------------------

void GuiContainer::AddTop(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Lock();
    LayerTransform transform;
    CancelEffectsInProgress(layer, true, &queue, &transform);
    LayerAdd(first_nonfixed, true, layer, effect, queue, &transform);
    Unlock();
}

void GuiContainer::AddTopFixed(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Lock();
    LayerTransform transform;
    CancelEffectsInProgress(layer, true, &queue, &transform);
    LayerAdd(first_nonfixed, false, layer, effect, queue, &transform);
    Unlock();
}

LayerIndex GuiContainer::LayerGetIndex(GuiLayer* layer)
{
    if(layer == NULL) return layers.end();

    LayerIndex it;
    for(it = layers.begin(); it != layers.end(); ++it) {
        if( *it == layer ) {
            return it;
        }
    }

    return layers.end();
}

bool GuiContainer::AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Lock();

    LayerTransform transform;
    CancelEffectsInProgress(layer, true, &queue, &transform);
    LayerIndex index = LayerGetIndex(ontopof);
    if( index != layers.end() ) {
        LayerAdd(index, true, layer, effect, queue, &transform);
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

bool GuiContainer::AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Lock();

    LayerTransform transform;
    CancelEffectsInProgress(layer, true, &queue, &transform);
    LayerIndex index = LayerGetIndex(behind);
    if( index != layers.end() ) {
        LayerAdd(++index, true, layer, effect, queue, &transform);
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

void GuiContainer::AddBottom(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Lock();
    LayerTransform transform;
    CancelEffectsInProgress(layer, true, &queue, &transform);
    LayerAdd(layers.end(), true, layer, effect, queue, &transform);
    Unlock();
}

void GuiContainer::Remove(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Lock();
    LayerTransform transform;
    CancelEffectsInProgress(layer, false, &queue, &transform);
    LayerRemove(layer, effect, queue, &transform);
    Unlock();
}

void GuiContainer::RemoveAndDelete(GuiLayer *layer, GuiEffect *effect, bool queue)
{
    Remove(layer, effect, queue);
    Delete(layer);
}


//--------------------------------------------------------------------------
// Overloaded GuiLayer functions
//--------------------------------------------------------------------------

bool GuiContainer::IsInVisibleArea(float x, float y)
{
    Lock();

    x -= GetX();
    y -= GetY();

    LayerIndex it;
    for(it = layers.begin(); it != layers.end(); ++it) {
        if( (*it)->IsInVisibleArea(x,y) ) {
            Unlock();
            return true;
        }
    }

    Unlock();
    return false; // container itself is not considered visible
}

bool GuiContainer::IsInVisibleArea(GuiLayer *layer)
{
    return IsInVisibleArea(layer->GetX() + layer->GetRefPixelX(),
                           layer->GetY() + layer->GetRefPixelY());
}

bool GuiContainer::IsBusy(void)
{
    Lock();

    if( !effect_list.empty() ) {
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

void GuiContainer::Draw(void)
{
    Lock();

    LayerTransform transform = GetTransform();

    if( IsVisible() ) {
        // Call ALL registered render callbacks
        for(std::list<GuiContainerCallback>::reverse_iterator rit = render_callback.rbegin(); rit != render_callback.rend(); ++rit) {
            if( (*rit).callback((*rit).context) ) {
              stop_requested = true;
            }
        }

        // Call registered frame callback top-to-bottom
        // stop on first modal frame (return value = true)
        for(std::list<GuiContainerCallback>::reverse_iterator rit = frame_callback.rbegin(); rit != frame_callback.rend(); ++rit) {
            if( (*rit).callback((*rit).context) ) {
                break;
            }
        }
    }

    // Reset transformation of layers
    for(std::list<GuiLayer*>::reverse_iterator rit = layers.rbegin(); rit != layers.rend(); ++rit) {
        GuiLayer *lay = *rit;
        LayerTransform t = transform;
        float posx = lay->GetX();
        float posy = lay->GetY();
        // Rotation transformation
        if( transform.rotation != 0.0f ) {
            float dx = lay->GetX() + lay->GetRefPixelX() - GetRefPixelX();
            float dy = lay->GetY() + lay->GetRefPixelY() - GetRefPixelY();
            float r = (float)sqrt(dx*dx+dy*dy);
            float a = (float)atan2(dy, dx);
            a = (float)fmod(a + (transform.rotation * GUI_2PI / 360.0f), GUI_2PI);
            posx += (float)cos(a) * r - dx;
            posy += (float)sin(a) * r - dy;
        }
        // Zoom transformation
        posx = (posx - GetRefPixelX()) * transform.stretchWidth + GetRefPixelX();
        posy = (posy - GetRefPixelY()) * transform.stretchHeight + GetRefPixelY();
        // Apply
        t.offsetX += posx - lay->GetX();
        t.offsetY += posy - lay->GetY();
        lay->ResetTransform(t);
    }

    // Handle effects
    std::list<LayerEffect>::iterator it = effect_list.begin();
    while( it != effect_list.end() ) {
        LayerEffect ef = *it;
        if( ef.active ) {
            if( ef.effect != NULL ) {
                if( ef.effect->Run() ) {
                    Delete(ef.effect);
                    ef.effect = NULL;
                }
            }
            if( ef.effect == NULL ) {
                // Effect is done, remove from effect list
                effect_list.erase(it++);
                // Activate next effect for the layer (if any)
                ActivateEffect(ef.layer);
                // Do post-effect actions
                if( ef.postaction == EP_REMOVE ) {
                    LayerRemove(ef.layer);
                }
                if( ef.postaction == EP_HIDE ) {
                    ef.layer->SetVisible(false);
                }
                continue;
            }
        }
        it++;
    }

    // Draw layers
    if( IsVisible() ) {
        for(std::list<GuiLayer*>::reverse_iterator rit = layers.rbegin(); rit != layers.rend(); ++rit) {
            (*rit)->Draw();
        }
    }

    Unlock();
}

