
#include "GuiContainer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "GuiEffect.h"
#include "GuiRootContainer.h"

GuiRootContainer* GuiContainer::_root = NULL;

GuiContainer::GuiContainer(GuiContainer *parent, const char *name)
            : GuiLayer(parent, name)
{
    _parent = parent? parent : _root;

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

    // Initialize our layers for usage
    _layers = NULL;
    _size = 0;
    _boundary = MAX_LAYERS;
    _layers = new GuiLayer*[_boundary];
    for(u32 i = 0; i < _boundary; i++)_layers[i] = NULL;

    render_callback = NULL;
    frame_callback = NULL;

    stop_requested = false;
    fixed_layers = 0;
    memset(layers_to_delete, 0, sizeof(layers_to_delete));
    memset(effect_list, 0, sizeof(effect_list));
}

GuiContainer::~GuiContainer()
{
    Lock();

    // Delete effects in progress
    for(int i = 0; i < MAX_LAYERS; i++) {
        LayerEffect *pe = effect_list[i];
        if( pe ) {
            if( pe->effect ) {
                delete pe->effect;
                pe->effect = NULL;
            }
            delete pe;
            effect_list[i] = NULL;
        }
    }

    // Delete render callbacks
    GuiContainerCallback *p = render_callback;
    render_callback = NULL;
    while( p ) {
        GuiContainerCallback *cb = p;
        p = p->next;
        delete cb;
    }

    // Delete frame callbacks
    p = frame_callback;
    frame_callback = NULL;
    while( p ) {
        GuiContainerCallback *cb = p;
        p = p->next;
        delete cb;
    }

    // Remove layers not deleted yet
    LayerRemoveAll();
    DeleteAll();

    // Clean-up layer administration
    if(_layers) {
      delete[] _layers;
      _layers = NULL;
    }
    Unlock();
}

//--------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------

void GuiContainer::Lock(void)
{
    if( _root != NULL ) {
        _root->Lock();
    }
}

void GuiContainer::Unlock(void)
{
    if( _root != NULL ) {
        _root->Unlock();
    }
}

GuiRootContainer* GuiContainer::GetRootContainer(void)
{
    return _root;
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
// Basic layer administration
//--------------------------------------------------------------------------

void GuiContainer::LayerAppend(GuiLayer* layer)
{
    if(layer == NULL) return;

    Lock();

#ifdef DEBUG // Only for debug because this check eats performance
    assert( LayerGetIndex(layer) == -1 ); // Layer should not added already
#endif

    // Check if it can be appended
    if(_size >= _boundary) {
        Unlock();
        return;
    }

    // Set layer on farthest position
    _layers[_size] = layer;
    _size++;

    Unlock();
}

void GuiContainer::LayerInsert(GuiLayer* layer, u32 index)
{
    if(layer == NULL) return;

    Lock();

#ifdef DEBUG // Only for debug because this check eats performance
    assert( LayerGetIndex(layer) == -1 ); // Layer should not added already
#endif

    // Check if it can be inserted
    if(_size >= _boundary || index > _size) {
        Unlock();
        return;
    }

    // Make some space for our new layer
    for(u32 i = _size; i > index; i--) {
        _layers[i] = _layers[i-1];
    }
    _layers[index] = layer;
    _size++;

    Unlock();
}

int GuiContainer::LayerGetIndex(GuiLayer* layer)
{
    if(layer == NULL) return -1;

    Lock();
    for(u32 i = 0; i < _size; i++){
        if( _layers[i] != NULL &&
            layer->GetID() == _layers[i]->GetID() )
        {
            Unlock();
            return i;
        }
    }
    Unlock();
    return -1;
}

void GuiContainer::LayerRemove(GuiLayer* layer)
{
    if(layer == NULL) return;

    bool shift = false;

    Lock();

    for(u32 i = 0; i < _size; i++){
        // Or check if the layers are equal and turn shifting on
        if(_layers[i] != NULL && layer->GetID() == _layers[i]->GetID()) {
            _layers[i] = NULL;
            shift = true;
            if( i < fixed_layers ) {
                fixed_layers--;
            }
        }
        // Layer found and shifting everything forward
        if(shift) {
            if( i+1 < _size){
                _layers[i] = _layers[i+1];
            }else{
                _layers[i] = NULL;
            }
        }
    }

    if(shift)_size--; // Success!

    Unlock();
}

void GuiContainer::LayerRemoveAll()
{
    Lock();

    for(u32 i = 0; i < _size; i++){
        _layers[i] = NULL;
    }
    fixed_layers = 0;
    _size = 0;

    Unlock();
}

u32 GuiContainer::LayerGetSize() const
{
    return _size;
}

//--------------------------------------------------------------------------
// Render/Frame callbacks
//--------------------------------------------------------------------------

void GuiContainer::AddRenderCallback(bool (*callback)(void*), void *context)
{
    GuiContainerCallback *p, *newrender = new GuiContainerCallback;
    newrender->callback = callback;
    newrender->context = context;
    newrender->next = NULL;

    Lock();

    if( render_callback ) {
        p = render_callback;
        while( p->next ) {
            p = p->next;
        }
        p->next = newrender;
    }else{
        render_callback = newrender;
    }

    Unlock();
}

void GuiContainer::RemoveRenderCallback(bool (*callback)(void*), void *context)
{
    Lock();

    GuiContainerCallback *cur = render_callback;
    GuiContainerCallback *prev = NULL;
    while( cur ) {
        if( cur->callback == callback && cur->context == context ) {
            if( prev ) {
                prev->next = cur->next;
            }else{
                render_callback = cur->next;
            }
            delete cur;
            Unlock();
            return;
        }
        prev = cur;
        cur = cur->next;
    }

    Unlock();
}

void GuiContainer::AddFrameCallback(bool (*callback)(void*), void *context)
{
    GuiContainerCallback *newrender = new GuiContainerCallback;
    newrender->callback = callback;
    newrender->context = context;

    Lock();

    newrender->next = frame_callback;
    frame_callback = newrender;

    Unlock();
}

void GuiContainer::RemoveFrameCallback(bool (*callback)(void*), void *context)
{
    Lock();

    GuiContainerCallback *cur = frame_callback;
    GuiContainerCallback *prev = NULL;
    while( cur ) {
        if( cur->callback == callback && cur->context == context ) {
            if( prev ) {
                prev->next = cur->next;
            }else{
                frame_callback = cur->next;
            }
            delete cur;
            Unlock();
            return;
        }
        prev = cur;
        cur = cur->next;
    }

    Unlock();
}

//--------------------------------------------------------------------------
// Layer order management
//--------------------------------------------------------------------------

int GuiContainer::GetIndex(GuiLayer *layer)
{
    return LayerGetIndex(layer);
}

int GuiContainer::GetFixedLayers(void)
{
    return fixed_layers;
}

void GuiContainer::RegisterForDelete(GuiLayer *layer)
{
    Lock();

    for(int i = 0; i < MAX_LAYERS; i++) {
        if( layers_to_delete[i] == NULL ) {
            layers_to_delete[i] = layer;
            Unlock();
            return;
        }
    }

    Unlock();
    assert(0);
}

bool GuiContainer::IsRegisteredForDelete(GuiLayer *layer)
{
    Lock();

    for(int i = 0; i < MAX_LAYERS; i++) {
        if( layers_to_delete[i] == layer ) {
            Unlock();
            return true;
        }
    }

    Unlock();
    return false;
}

void GuiContainer::DeleteDirect(GuiLayer *layer)
{
    Lock();

    assert( layer );
#ifdef DEBUG // Only for debug because this checks eats performance
    assert( IsRegisteredForDelete(layer) ); // Layer must be registered
    assert( LayerGetIndex(layer) == -1 ); // Layer should not be displaying
#endif
    for(int i = 0; i < MAX_LAYERS; i++) {
        if( layers_to_delete[i] != NULL &&
            layers_to_delete[i]->GetID() == layer->GetID() ) {
            layers_to_delete[i] = NULL;
            Unlock();
            OnDelete(layer);
            delete layer;
            return;
        }
    }

    Unlock();
    assert(0);
}

void GuiContainer::DeleteAll(void)
{
    Lock();

    // Make sure no layer is shown anymore
    LayerRemoveAll();

    bool bAgain;
    do {
        bAgain = false;
        for(int i = 0; i < MAX_LAYERS; i++) {
            GuiLayer *layer = layers_to_delete[i];
            if( layer != NULL ) {
                layers_to_delete[i] = NULL;
                Unlock();
                OnDelete(layer);
                delete layer;
                Lock();
                bAgain = true;
            }
        }
    }while( bAgain );

    Unlock();
}

void GuiContainer::AddIndex(int index, GuiLayer *layer, bool fix, GuiEffect *effect)
{
    Lock();

    if( effect != NULL ) {
        LayerTransform old_transform;
        // Check for pending actions with this layer in effect-list
        for(int i = 0; i < MAX_LAYERS; i++) {
            LayerEffect *pe = effect_list[i];
            // check if currently active
            if( pe != NULL &&
                ((pe->add_layer && pe->add_layer->GetID() == layer->GetID()) ||
                 (pe->remove_layer && pe->remove_layer->GetID() == layer->GetID())) )
            {
                // Cancel the old effect
                if( pe->effect->CancelLayer(layer, &old_transform) ) {
                    // When it is a remove effect, do the remove actions
                    if( pe->remove_layer ) {
                        LayerRemove(pe->remove_layer);
                        if( pe->delete_layer ) {
                            DeleteDirect(pe->remove_layer);
                            pe->delete_layer = false;
                        }
                        pe->remove_layer = NULL;
                    }
                    // Clear the old effect
                    if( pe->effect != NULL ) {
                        delete pe->effect;
                        pe->effect = NULL;
                    }
                    // Free item from list
                    delete effect_list[i];
                    effect_list[i] = NULL;
                }
            }
        }

        // initialize the effect
        effect->Initialize(NULL, layer, LayerTransform(), old_transform);
    }

    // add layer
    if( (u32)index < fixed_layers ) {
        LayerInsert(layer, index);
        fixed_layers++;
    }else
    if( fix ) {
        LayerInsert(layer, 0);
        fixed_layers++;
    }else{
        if( index == MAX_LAYERS ) {
            LayerAppend(layer);
        }else{
            LayerInsert(layer, index);
        }
    }

    if( effect ) {
        // find free spot in effect-list
        int i;
        for(i = 0; i < MAX_LAYERS; i++) {
            if( effect_list[i] == NULL ) break;
        }
        assert( i != MAX_LAYERS );

        // start the effect
        LayerEffect *pe = new LayerEffect;
        pe->add_layer = layer;
        pe->remove_layer = NULL;
        pe->delete_layer = false;
        pe->effect = effect;
        effect_list[i] = pe;
    }

    Unlock();
}

void GuiContainer::AddTop(GuiLayer *layer, GuiEffect *effect)
{
    AddIndex(fixed_layers, layer, false, effect);
}

void GuiContainer::AddTopFixed(GuiLayer *layer, GuiEffect *effect)
{
    AddIndex(fixed_layers, layer, true, effect);
}

bool GuiContainer::AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect)
{
    Lock();

    int index = LayerGetIndex(ontopof);
    if( index >= 0 ) {
        AddIndex(index, layer, false, effect);
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

bool GuiContainer::AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect)
{
    Lock();

    int index = LayerGetIndex(behind);
    if( index >= 0 ) {
        AddIndex(index+1, layer, false, effect);
        Unlock();
        return true;
    }

    Unlock();
    return false;
}

void GuiContainer::AddBottom(GuiLayer *layer, GuiEffect *effect)
{
    AddIndex(MAX_LAYERS, layer, false, effect);
}

void GuiContainer::PrivateRemove(GuiLayer *layer, bool needdelete, GuiEffect *effect)
{
    LayerTransform old_transform;

    assert(layer);

    Lock();

    // Check for pending actions with this layer in effect-list
    for(int i = 0; i < MAX_LAYERS; i++) {
        LayerEffect *pe = effect_list[i];
        // Check if beeing removed already
        if( pe != NULL && pe->remove_layer && pe->remove_layer->GetID() == layer->GetID() )
        {
            // If already pending for delete, don't loose the request
            if( pe->delete_layer ) {
                needdelete = true;
            }
            // Cancel the effect
            //   (but don't handle the remove because we are about to create a new remove request)
            pe->remove_layer = NULL;
            pe->delete_layer = false;
            if( pe->effect->CancelLayer(layer, &old_transform) ) {
                // Effect done, delete the effect
                delete pe->effect;
                pe->effect = NULL;
                // Remove from list
                delete pe;
                pe = NULL;
                effect_list[i] = NULL;
            }
        }
        // Check if currently beeing added
        if( pe != NULL && pe->add_layer && pe->add_layer->GetID() == layer->GetID() )
        {
            // Cancel the effect
            pe->add_layer = NULL;
            if( pe->effect->CancelLayer(layer, &old_transform) ) {
                // Effect done, delete the effect
                delete pe->effect;
                pe->effect = NULL;
                // Handle remove (must be some other layer)
                if( pe->remove_layer && !pe->remove_layer->IsBusy() ) {
                    assert( pe->remove_layer->GetID() != layer->GetID() ); // must be someone else
                    LayerRemove(pe->remove_layer);
                    if( pe->delete_layer ) {
                        DeleteDirect(pe->remove_layer);
                        pe->delete_layer = false;
                    }
                    pe->remove_layer = NULL;
                }
                // Remove from list if all done
                if( pe->effect == NULL && pe->remove_layer == NULL ) {
                    delete pe;
                    effect_list[i] = NULL;
                }
            }
        }
    }

    if( effect != NULL || layer->IsBusy() ) {
        // initialize the new effect
        if( effect != NULL ) {
            effect->Initialize(layer, NULL, old_transform);
        }

        // find free spot in effect-list
        int i;
        for(i = 0; i < MAX_LAYERS; i++) {
            if( effect_list[i] == NULL ) break;
        }
        assert( i != MAX_LAYERS );

        // Add entry to list
        LayerEffect *pe = new LayerEffect;
        pe->add_layer = NULL;
        pe->remove_layer = layer;
        pe->delete_layer = needdelete;
        pe->effect = effect;
        effect_list[i] = pe;
    }
    else
    {
        // No effect and not busy, remove/delete directly
        LayerRemove(layer);
        if( needdelete ) {
            DeleteDirect(layer);
        }
    }

    Unlock();
}

void GuiContainer::Remove(GuiLayer *layer, GuiEffect *effect)
{
    PrivateRemove(layer, false, effect);
}

void GuiContainer::RemoveAndDelete(GuiLayer *layer, GuiEffect *effect)
{
    PrivateRemove(layer, true, effect);
}

void GuiContainer::Delete(GuiLayer *layer)
{
    PrivateRemove(layer, true, NULL);
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

    for(int i = 0; i < MAX_LAYERS; i++) {
        if( effect_list[i] != NULL ) {
            Unlock();
            return true;
        }
    }

    Unlock();
    return false;
}

void GuiContainer::Draw(void)
{
    LayerTransform transform = GetTransform();

    // Call ALL registered render callbacks
    GuiContainerCallback *p = render_callback;
    while( p ) {
        if( p->callback(p->context) ) {
          stop_requested = true;
        }
        p = p->next;
    }
    // Call registered frame callback top-to-bottom
    // stop on first modal frame (return value = true)
    p = frame_callback;
    while( p ) {
        if( p->callback(p->context) ) {
            break;
        }
        p = p->next;
    }

    Lock();

    // Reset transformation of layers
    for(u32 i = _size; i > 0; i--){
        GuiLayer *lay = _layers[i-1];
        if( lay != NULL ) {
            LayerTransform t = transform;
            f32 posx = lay->GetX();
            f32 posy = lay->GetY();
            // Rotation transformation
            if( transform.rotation != 0.0f ) {
                f32 dx = lay->GetX() + lay->GetRefPixelX() - _refPixelX;
                f32 dy = lay->GetY() + lay->GetRefPixelY() - _refPixelY;
                f32 r = sqrt(dx*dx+dy*dy);
                f32 a = atan2(dy,dx);
                a = fmod(a + (transform.rotation * GUI_2PI / 360.0f), GUI_2PI);
                posx += cos(a) * r - dx;
                posy += sin(a) * r - dy;
            }
            // Zoom transformation
            posx = (posx - _refPixelX) * transform.stretchWidth + _refPixelX;
            posy = (posy - _refPixelY) * transform.stretchHeight + _refPixelY;
            // Apply
            t.offsetX += posx - lay->GetX();
            t.offsetY += posy - lay->GetY();
            lay->ResetTransform(t);
        }
    }

    // Handle effects
    for(int i = 0; i < MAX_LAYERS; i++) {
        LayerEffect *eff = effect_list[i];
        if( eff != NULL ) {
            if( eff->effect != NULL ) {
                if( eff->effect->Run() ) {
                    delete eff->effect;
                    eff->effect = NULL;
                }
            }
            if( eff->effect == NULL ) {
                // Do remove
                if( eff->remove_layer && !eff->remove_layer->IsBusy() ) {
                    LayerRemove(eff->remove_layer);
                    if( eff->delete_layer ) {
                        DeleteDirect(eff->remove_layer);
                        eff->delete_layer = false;
                    }
                    eff->remove_layer = NULL;
                }
            }
            if( eff->effect == NULL && eff->remove_layer == NULL ) {
                // all done, remove from effect list
                delete eff;
                effect_list[i] = NULL;
            }
        }
    }

    // Draw layers
    for(u32 i = _size; i > 0; i--){
        GuiLayer *lay = _layers[i-1];
        if( lay != NULL ) {
            lay->Draw();
        }
    }

    Unlock();
}

