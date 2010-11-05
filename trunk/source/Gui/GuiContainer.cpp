
#include "GuiContainer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GuiEffect.h"
#include "GuiRootContainer.h"

GuiRootContainer* GuiContainer::_root = NULL;

GuiContainer::GuiContainer(GuiContainer *cntr)
            : Layer()
{
    _parent = cntr? cntr : _root;

    if( _parent != NULL ) {
        SetWidth(_parent->GetWidth());
        SetHeight(_parent->GetHeight());
    }

    // Initialize our layers for usage
    _layers = NULL;
    _size = 0;
    _boundary = MAX_LAYERS;
    _layers = new Layer*[_boundary];
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
// Basic layer administration
//--------------------------------------------------------------------------

void GuiContainer::LayerAppend(Layer* layer)
{
    if(layer == NULL) return;

    // Check if it can be appended
    if(_size >= _boundary) return;

    Lock();

    // Set layer on farthest position
    _layers[_size] = layer;
    _size++;

    Unlock();
}

void GuiContainer::LayerInsert(Layer* layer, u32 index)
{
    if(layer == NULL || index > _size)return;

    // Check if it can be inserted
    if(_size >= _boundary || index > _size)return;

    Lock();

    // Make some space for our new layer
    for(u32 i = _size; i > index; i--) {
        _layers[i] = _layers[i-1];
    }
    _layers[index] = layer;
    _size++;

    Unlock();
}

int GuiContainer::LayerGetIndex(Layer* layer)
{
    if(layer == NULL)return -1;

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

void GuiContainer::LayerRemove(Layer* layer)
{
    if(layer == NULL)return;

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

Layer* GuiContainer::LayerGetLayerAt(u32 index) const
{
    if(index >= _size)return NULL;
    return _layers[index];
}

u32 GuiContainer::LayerGetSize() const
{
    return _size;
}

void GuiContainer::LayerDraw(f32 offsetX, f32 offsetY, f32 rot, u8 alpha)
{
    // Do the layers!
    Lock();
    for(u32 i = _size; i > 0; i--){
        if(i == 0) break;
        if(_layers[i-1]) {
            _layers[i-1]->SetTransform(offsetX, offsetY, rot, alpha);
            _layers[i-1]->Draw();
        }
    }
    Unlock();
}

//--------------------------------------------------------------------------
// Layer order management
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

void GuiContainer::Lock(void)
{
    mutex.Lock();
}

void GuiContainer::Unlock(void)
{
    mutex.Unlock();
}


int GuiContainer::GetIndex(Layer *layer)
{
    return LayerGetIndex(layer);
}

int GuiContainer::GetFixedLayers(void)
{
    return fixed_layers;
}

void GuiContainer::RegisterForDelete(Layer *layer)
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

void GuiContainer::DeleteDirect(Layer *layer)
{
    Lock();

    assert( layer );
    for(int i = 0; i < MAX_LAYERS; i++) {
        if( layers_to_delete[i] != NULL &&
            layers_to_delete[i]->GetID() == layer->GetID() ) {
            layers_to_delete[i] = NULL;
            Unlock();
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
    bool bAgain;
    do {
        bAgain = false;
        for(int i = 0; i < MAX_LAYERS; i++) {
            Layer *layer = layers_to_delete[i];
            if( layer != NULL ) {
                layers_to_delete[i] = NULL;
                Unlock();
                delete layer;
                Lock();
                bAgain = true;
            }
        }
    }while( bAgain );
    Unlock();
}

void GuiContainer::AddIndex(int index, Layer *layer, bool fix, GuiEffect *effect)
{
    if( effect ) {
        // initialize the effect
        effect->Initialize(NULL, layer);
    }

    Lock();

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
        pe->active_layer[0] = layer;
        pe->active_layer[1] = NULL;
        pe->remove_layer = NULL;
        pe->delete_layer = NULL;
        pe->effect = effect;
        effect_list[i] = pe;
    }

    Unlock();
}

void GuiContainer::AddTop(Layer *layer, GuiEffect *effect)
{
    AddIndex(fixed_layers, layer, false, effect);
}

void GuiContainer::AddTopFixed(Layer *layer, GuiEffect *effect)
{
    AddIndex(fixed_layers, layer, true, effect);
}

bool GuiContainer::AddOnTopOf(Layer *ontopof, Layer *layer, GuiEffect *effect)
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

bool GuiContainer::AddBehind(Layer *behind, Layer *layer, GuiEffect *effect)
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

void GuiContainer::AddBottom(Layer *layer, GuiEffect *effect)
{
    AddIndex(MAX_LAYERS, layer, false, effect);
}

void GuiContainer::PrivateRemove(const char *file, int line, Layer *layer, bool needdelete, GuiEffect *effect)
{
    assert(layer);

    Lock();

    if( effect != NULL ) {
        // Check for pending actions with this layer in effect-list
        for(int i = 0; i < MAX_LAYERS; i++) {
            LayerEffect *pe = effect_list[i];
            // check if currently active
            if( pe != NULL &&
                ((pe->active_layer[0] && pe->active_layer[0]->GetID() == layer->GetID()) ||
                 (pe->active_layer[1] && pe->active_layer[1]->GetID() == layer->GetID())) )
            {
                // Finish the old effect
                if( pe->effect != NULL ) {
                    while( !pe->effect->Run() );
                    delete pe->effect;
                    pe->effect = NULL;
                }
                // This should not be a remove effect
                assert( pe->remove_layer == NULL );
                assert( pe->delete_layer == NULL );
            }
        }

        // initialize the new effect
        Unlock();
        if( effect != NULL ) {
            effect->Initialize(layer, NULL);
        }
        Lock();

        // find free spot in effect-list
        int i;
        for(i = 0; i < MAX_LAYERS; i++) {
            if( effect_list[i] == NULL ) break;
        }
        assert( i != MAX_LAYERS );

        // Add entry to list
        LayerEffect *pe = new LayerEffect;
        pe->file = file;
        pe->line = line;
        pe->active_layer[0] = layer;
        pe->active_layer[1] = NULL;
        pe->remove_layer = layer;
        pe->delete_layer = needdelete? layer : NULL;
        pe->effect = effect;
        effect_list[i] = pe;
    }
    else
    {
        // Check for pending actions with this layer in effect-list
        for(int i = 0; i < MAX_LAYERS; i++) {
            LayerEffect *pe = effect_list[i];
            // check if currently active
            if( pe != NULL &&
                ((pe->active_layer[0] && pe->active_layer[0]->GetID() == layer->GetID()) ||
                 (pe->active_layer[1] && pe->active_layer[1]->GetID() == layer->GetID())) )
            {
                // found an active effect with the layer to remove and/or delete
                if( pe->remove_layer != NULL ) {
                    // already pending for remove, check it
                    assert( pe->remove_layer->GetID() == layer->GetID() );
                }else{
                    // make this layer dending for remove
                    pe->remove_layer = layer;
                }
                if( needdelete ) {
                    if( pe->delete_layer != NULL ) {
                        // already pending for delete, check it
                        assert( pe->delete_layer->GetID() == layer->GetID() );
                    }else{
                        // make this layer dending for delete
                        pe->delete_layer = layer;
                    }
                }
                Unlock();
                return;
            }
        }
        
        // No effect with this layer currently in progress, remove/delete directly
        if( layer->IsBusy() ) {
            // layer is busy, postpone till ready

            // find free spot in effect-list
            int i;
            for(i = 0; i < MAX_LAYERS; i++) {
                if( effect_list[i] == NULL ) break;
            }
            assert( i != MAX_LAYERS );
            
            // Add entry to list
            LayerEffect *pe = new LayerEffect;
            pe->file = file;
            pe->line = line;
            pe->active_layer[0] = layer;
            pe->active_layer[1] = NULL;
            pe->remove_layer = layer;
            pe->delete_layer = needdelete? layer : NULL;
            pe->effect = NULL;
            effect_list[i] = pe;
        }
        else
        {
            // layer can be removed/deleted directly
            LayerRemove(layer);
            if( needdelete ) {
                DeleteDirect(layer);
            }
        }
    }

    Unlock();
}

#ifdef DEBUG

void GuiContainer::_Remove(const char *file, int line, Layer *layer, GuiEffect *effect)
{
    PrivateRemove(file, line, layer, false, effect);
}

void GuiContainer::_RemoveAndDelete(const char *file, int line, Layer *layer, GuiEffect *effect)
{
    PrivateRemove(file, line, layer, true, effect);
}

void GuiContainer::_Delete(const char *file, int line, Layer *layer)
{
    PrivateRemove(file, line, layer, true, NULL);
}

#else

void GuiContainer::Remove(Layer *layer, GuiEffect *effect)
{
    PrivateRemove("", 0, layer, false, effect);
}

void GuiContainer::RemoveAndDelete(Layer *layer, GuiEffect *effect)
{
    PrivateRemove("", 0, layer, true, effect);
}

void GuiContainer::Delete(Layer *layer)
{
    PrivateRemove("", 0, layer, true, NULL);
}

#endif


GuiRootContainer* GuiContainer::GetRootContainer(void)
{
    return _root;
}

GuiContainer* GuiContainer::GetParentContainer(void)
{
    return _parent;
}

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
    for(int i = 0; i < MAX_LAYERS; i++) {
        if( effect_list[i] != NULL ) {
            return true;
        }
    }
    return false;
}

void GuiContainer::Draw(void)
{
    f32 absX = GetXabs();
    f32 absY = GetYabs();
    f32 rotation = GetRotationAbs();
    u8 alpha = GetTransparencyAbs();

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
                if( eff->remove_layer && !eff->remove_layer->IsBusy() ) {
                    LayerRemove(eff->remove_layer);
                    assert( eff->delete_layer == NULL ||
                            eff->delete_layer == eff->remove_layer );
                    eff->remove_layer = NULL;
                    if( eff->delete_layer ) {
                        DeleteDirect(eff->delete_layer);
                        eff->delete_layer = NULL;
                    }
                }
            }
            if( eff->effect == NULL && eff->remove_layer == NULL && eff->delete_layer == NULL ) {
                // all done, remove from effect list
                delete eff;
                effect_list[i] = NULL;
            }
        }
    }

    // Clip drawing
    int cx = (int)GetXabs(), cy = (int)GetYabs();
    int cw = (int)GetWidth(), ch = (int)GetHeight();
    GameWindow::SetGetClipping(&cx, &cy, &cw, &ch);

    // Draw layers
    LayerDraw(absX, absY, rotation, alpha);

    // Restore clipping
    GameWindow::SetGetClipping(&cx, &cy, &cw, &ch);

    Unlock();
}

