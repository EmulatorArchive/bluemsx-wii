
#include "GuiContainer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "GuiEffect.h"
#include "GuiRootContainer.h"

GuiContainer::GuiContainer(GuiContainer *parent, const char *name)
            : GuiLayer(parent, name)
{
    if( parent != NULL ) {
        _parent = parent;
    }else{
        _parent = GetRootContainer();
    }

    // Inherit properties
    if( _parent != NULL ) {
        SetWidth(_parent->GetWidth());
        SetHeight(_parent->GetHeight());
        SetPointerImage(_parent->GetPointerImage());
    }else{
        SetWidth(0);
        SetHeight(0);
        SetPointerImage(NULL);
    }

    first_nonfixed = layers.end();
    stop_requested = false;
}

GuiContainer::~GuiContainer()
{
    Lock();

    // Delete effects in progress
    while( !effect_list.empty() ) {
        LayerEffect ef = effect_list.front();
        if( ef.effect ) {
            Delete(ef.effect);
            ef.effect = NULL;
        }
        effect_list.pop_front();
    } 

    // Remove layers not deleted yet
    LayerRemoveAll();

    Unlock();
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
        OnDelete(atom);
        GetRootContainer()->ReleaseAtom(atom);
    }
}


//--------------------------------------------------------------------------
// Internal layer administration
//--------------------------------------------------------------------------

void GuiContainer::LayerAdd(LayerIndex index, bool movenonfixed, GuiLayer *layer, GuiEffect *effect)
{
    if( effect != NULL ) {
        LayerTransform old_transform;
        // Check for pending actions with this layer in effect-list
        std::list<LayerEffect>::iterator it = effect_list.begin();
        while( it != effect_list.end() ) {
            LayerEffect &ef = *it;
            // check if currently active
            if( ((ef.add_layer && ef.add_layer == layer) ||
                 (ef.remove_layer && ef.remove_layer == layer)) )
            {
                // Cancel the old effect
                if( ef.effect->CancelLayer(layer, &old_transform) ) {
                    // When it is a remove effect, do the remove actions
                    if( ef.remove_layer ) {
                        LayerRemove(ef.remove_layer);
                        ef.remove_layer = NULL;
                    }
                    // Clear the old effect
                    if( ef.effect != NULL ) {
                        Delete(ef.effect);
                        ef.effect = NULL;
                    }
                    // Free item from list
                    effect_list.erase(it++);
                    continue;
                }
            }
            it++;
        }

        // initialize the effect
        effect->Initialize(NULL, layer, LayerTransform(), old_transform);
    }

    // add layer
    GuiRootContainer::UseAtom(layer);
    layers.insert(index, layer);
    if( movenonfixed && first_nonfixed == index ) {
        --first_nonfixed;
    }

    if( effect ) {
        // start the effect
        LayerEffect ef;
        ef.add_layer = layer;
        ef.remove_layer = NULL;
        ef.effect = effect;
        effect_list.push_back(ef);
    }
}

LayerIndex GuiContainer::LayerGetIndex(GuiLayer* layer)
{
    if(layer == NULL) return layers.end();

    Lock();
    LayerIndex it;
    for(it = layers.begin(); it != layers.end(); ++it) {
        if( *it == layer ) {
            Unlock();
            return it;
        }
    }
    Unlock();
    return layers.end();
}

void GuiContainer::LayerRemove(GuiLayer* layer)
{
    if(layer == NULL) return;

    Lock();

    LayerIndex it;
    for(it = layers.begin(); it != layers.end(); ++it) {
        if( *it == layer ) {
            if( it == first_nonfixed ) {
                ++first_nonfixed;
            }
            layers.erase(it);
            GuiRootContainer::ReleaseAtom(layer);
            Unlock();
            return;
        }
    }

    Unlock();
}

void GuiContainer::LayerRemoveAll()
{
    Lock();

    LayerIndex it;
    while( (it = layers.begin()) != layers.end() ) {
        Remove(*it);
    }

    Unlock();
}

//--------------------------------------------------------------------------
// Layer order management (public interface)
//--------------------------------------------------------------------------

void GuiContainer::AddTop(GuiLayer *layer, GuiEffect *effect)
{
    Lock();
    LayerAdd(first_nonfixed, true, layer, effect);
    Unlock();
}

void GuiContainer::AddTopFixed(GuiLayer *layer, GuiEffect *effect)
{
    Lock();
    LayerAdd(first_nonfixed, false, layer, effect);
    Unlock();
}

bool GuiContainer::AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect)
{
    Lock();

    LayerIndex index = LayerGetIndex(ontopof);
    if( index != layers.end() ) {
        LayerAdd(index, true, layer, effect);
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

bool GuiContainer::AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect)
{
    Lock();

    LayerIndex index = LayerGetIndex(behind);
    if( index != layers.end() ) {
        LayerAdd(++index, true, layer, effect);
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

void GuiContainer::AddBottom(GuiLayer *layer, GuiEffect *effect)
{
    Lock();
    LayerAdd(layers.end(), true, layer, effect);
    Unlock();
}

void GuiContainer::PrivateRemove(GuiLayer *layer, GuiEffect *effect)
{
    LayerTransform old_transform;

    assert(layer);

    Lock();

    // Check for pending actions with this layer in effect-list
    std::list<LayerEffect>::iterator it = effect_list.begin();
    while( it != effect_list.end() ) {
        LayerEffect &ef = *it;
        bool erase = false;
        // Check if beeing removed already
        if( ef.remove_layer && ef.remove_layer == layer )
        {
            // Cancel the effect
            //   (but don't handle the remove because we are about to create a new remove request)
            ef.remove_layer = NULL;
            if( ef.effect->CancelLayer(layer, &old_transform) ) {
                // Effect done, delete the effect
                Delete(ef.effect);
                ef.effect = NULL;
                // Remove from list
                erase = true;
            }
        }
        // Check if currently beeing added
        if( ef.add_layer && ef.add_layer == layer )
        {
            // Cancel the effect
            ef.add_layer = NULL;
            if( ef.effect->CancelLayer(layer, &old_transform) ) {
                // Effect done, delete the effect
                Delete(ef.effect);
                ef.effect = NULL;
                // Handle remove (must be some other layer)
                if( ef.remove_layer && !ef.remove_layer->IsBusy() ) {
                    assert( ef.remove_layer != layer ); // must be someone else
                    LayerRemove(ef.remove_layer);
                    ef.remove_layer = NULL;
                }
                // Remove from list if all done
                if( ef.effect == NULL && ef.remove_layer == NULL ) {
                    erase = true;
                }
            }
        }
        if( erase ) {
            effect_list.erase(it++);
        }else{
            it++;
        }
    }

    if( effect != NULL ) {
        // initialize the new effect
        if( effect != NULL ) {
            Unlock();
            effect->Initialize(layer, NULL, old_transform);
            Lock();
        }

        // Add entry to list
        LayerEffect ef;
        ef.add_layer = NULL;
        ef.remove_layer = layer;
        ef.effect = effect;
        effect_list.push_back(ef);
    }
    else
    {
        // No effect, remove/delete directly
        LayerRemove(layer);
    }

    Unlock();
}

void GuiContainer::Remove(GuiLayer *layer, GuiEffect *effect)
{
    PrivateRemove(layer, effect);
}

void GuiContainer::RemoveAndDelete(GuiLayer *layer, GuiEffect *effect)
{
    PrivateRemove(layer, effect);
    Delete(layer);
}


//--------------------------------------------------------------------------
// Overloaded GuiLayer functions
//--------------------------------------------------------------------------

void GuiContainer::SetWidth(u32 width)
{
    _width = width;
}

void GuiContainer::SetHeight(u32 height)
{
    _height = height;
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
    if( !IsVisible() ) {
        return;
    }

    LayerTransform transform = GetTransform();

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

    Lock();

    // Reset transformation of layers
    for(std::list<GuiLayer*>::reverse_iterator rit = layers.rbegin(); rit != layers.rend(); ++rit) {
        GuiLayer *lay = *rit;
        LayerTransform t = transform;
        f32 posx = lay->GetX();
        f32 posy = lay->GetY();
        // Rotation transformation
        if( transform.rotation != 0.0f ) {
            f32 dx = lay->GetX() + lay->GetRefPixelX() - _refPixelX;
            f32 dy = lay->GetY() + lay->GetRefPixelY() - _refPixelY;
            f32 r = (f32)sqrt(dx*dx+dy*dy);
            f32 a = (f32)atan2(dy,dx);
            a = (f32)fmod(a + (transform.rotation * GUI_2PI / 360.0f), GUI_2PI);
            posx += (f32)cos(a) * r - dx;
            posy += (f32)sin(a) * r - dy;
        }
        // Zoom transformation
        posx = (posx - _refPixelX) * transform.stretchWidth + _refPixelX;
        posy = (posy - _refPixelY) * transform.stretchHeight + _refPixelY;
        // Apply
        t.offsetX += posx - lay->GetX();
        t.offsetY += posy - lay->GetY();
        lay->ResetTransform(t);
    }

    // Handle effects
    std::list<LayerEffect>::iterator it = effect_list.begin();
    while( it != effect_list.end() ) {
        LayerEffect &ef = *it;
        if( ef.effect != NULL ) {
            if( ef.effect->Run() ) {
                Delete(ef.effect);
                ef.effect = NULL;
            }
        }
        if( ef.effect == NULL ) {
            // Do remove
            if( ef.remove_layer && !ef.remove_layer->IsBusy() ) {
                LayerRemove(ef.remove_layer);
                ef.remove_layer = NULL;
            }
        }
        if( ef.effect == NULL && ef.remove_layer == NULL ) {
            // all done, remove from effect list
            effect_list.erase(it++);
            continue;
        }
        it++;
    }

    // Draw layers
    for(std::list<GuiLayer*>::reverse_iterator rit = layers.rbegin(); rit != layers.rend(); ++rit) {
        (*rit)->Draw();
    }

    Unlock();
}

