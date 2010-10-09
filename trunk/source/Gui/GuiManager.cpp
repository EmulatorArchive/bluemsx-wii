
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GuiManager.h"


GuiManager *GuiManager::pThis;

void GuiManager::AddRenderCallback(bool (*callback)(void*), void *context)
{
    GuiManagerCallback *p, *newrender = new GuiManagerCallback;
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

void GuiManager::RemoveRenderCallback(bool (*callback)(void*), void *context)
{
    GuiManagerCallback *cur = render_callback;
    GuiManagerCallback *prev = NULL;
    while( cur ) {
        if( cur->callback == callback && cur->context == context ) {
            Lock();
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
}

void GuiManager::AddFrameCallback(bool (*callback)(void*), void *context)
{
    GuiManagerCallback *newrender = new GuiManagerCallback;
    newrender->callback = callback;
    newrender->context = context;
    Lock();
    newrender->next = frame_callback;
    frame_callback = newrender;
    Unlock();
}

void GuiManager::RemoveFrameCallback(bool (*callback)(void*), void *context)
{
    GuiManagerCallback *cur = frame_callback;
    GuiManagerCallback *prev = NULL;
    while( cur ) {
        if( cur->callback == callback && cur->context == context ) {
            Lock();
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
}

void GuiManager::Lock(void)
{
    mutex.Lock();
}

void GuiManager::Unlock(void)
{
    mutex.Unlock();
}

int GuiManager::GetIndex(Layer *layer)
{
    return manager->GetIndex(layer);
}

int GuiManager::GetFixedLayers(void)
{
    return fixed_layers;
}

void GuiManager::AddIndex(int index, Layer *layer, bool fix, int fade, int delay)
{
    FrameAdd *add = NULL;
    int i;
    if( fade ) {
        // find free spot in add-list
        for(i = 0; i < GUI_MAX_LAYERS; i++) {
            if( add_list[i].layer == NULL ) break;
        }
        if( i == GUI_MAX_LAYERS ) return;
        add = &add_list[i];
        // set parameters for adding it
        add->count = add->fade = fade;
        add->delay = delay;
        add->alpha = layer->GetTransparency();
        add->curalpha = 0;
        // start at 'fully transparent'
        layer->SetTransparency(add->curalpha);
    }
    if( index < fixed_layers ) {
        manager->Insert(layer, index);
        fixed_layers++;
    }else
    if( fix ) {
        manager->Insert(layer, 0);
        fixed_layers++;
    }else{
        if( index == GUI_MAX_LAYERS ) {
            manager->Append(layer);
        }else{
            manager->Insert(layer, index);
        }
    }
    if( fade ) {
        // start fading-in
        add->layer = layer;
    }
}

void GuiManager::AddTop(Layer *layer, int fade, int delay)
{
    AddIndex(fixed_layers, layer, false, fade, delay);
}

void GuiManager::AddTopFixed(Layer *layer, int fade, int delay)
{
    AddIndex(fixed_layers, layer, true, fade, delay);
}

void GuiManager::AddOnTopOf(Layer *ontopof, Layer *layer, int fade, int delay)
{
    int index = manager->GetIndex(ontopof);
    if( index >= 0 ) {
        AddIndex(index, layer, false, fade, delay);
    }
}

void GuiManager::AddBehind(Layer *behind, Layer *layer, int fade, int delay)
{
    int index = manager->GetIndex(behind);
    if( index >= 0 ) {
        AddIndex(index+1, layer, false, fade, delay);
    }
}

void GuiManager::AddBottom(Layer *layer, int fade, int delay)
{
    AddIndex(GUI_MAX_LAYERS, layer, false, fade, delay);
}

void GuiManager::RegisterRemove(Layer *layer, bool needdelete, int fade, int delay, Image *image)
{
    int i;
    // remove from add-list (still busy fading-in)
    // remove from remove-list to prevent deleting twice
    for(i = 0; i < GUI_MAX_LAYERS; i++) {
        if( add_list[i].layer == layer ) {
            add_list[i].layer = NULL;
        }
        if( remove_list[i].layer == layer ) {
            remove_list[i].layer = NULL;
        }
    }
    // find free spot in remove-list
    for(i = 0; i < GUI_MAX_LAYERS; i++) {
        if( remove_list[i].layer == NULL ) break;
    }
    if( i == GUI_MAX_LAYERS ) return;
    FrameRemove *remove = &remove_list[i];
    // Add entry to list
    remove->needdelete = needdelete;
    remove->count = remove->fade = fade;
    remove->delay = delay;
    remove->alpha = layer->GetTransparency();
    remove->image = image;
    remove->layer = layer;
}

void GuiManager::Remove(Layer *layer, int fade, int delay)
{
    if( fade == 0 ) {
        int idx = manager->GetIndex(layer);
        if( idx < 0 ) return;

        manager->Remove(layer);

        if( idx < fixed_layers ) {
            fixed_layers--;
        }
    }else{
        RegisterRemove(layer, false, fade, delay, NULL);
    }
}

void GuiManager::RemoveAndDelete(Layer *layer, Image *image, int fade, int delay)
{
    if( layer != NULL ) {
        if( fade == 0 ) {
            Remove(layer);
            if( image ) {
                delete image;
            }
            delete layer;
        }else{
            RegisterRemove(layer, true, fade, delay, image);
        }
    }
}

GW_VIDEO_MODE GuiManager::GetMode(void)
{
    return gwd.GetMode();
}

u32 GuiManager::GetWidth(void)
{
    return gwd.GetWidth();
}

u32 GuiManager::GetHeight(void)
{
    return gwd.GetHeight();
}

void GuiManager::SetMode(GW_VIDEO_MODE mode)
{
    gwd.SetMode(mode);
}

bool GuiManager::DrawFuncWrapper(void *context)
{
    return ((GuiManager*)context)->DrawFunc();
}

bool GuiManager::DrawFunc()
{
    // Call ALL registered render callbacks
    GuiManagerCallback *p = render_callback;
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

    // Handle fade-out
    for(int i = 0; i < GUI_MAX_LAYERS; i++) {
        FrameRemove *remove = &remove_list[i];
        if( remove->layer ) {
            if( remove->delay ) {
                remove->delay--;
            }else
            if( remove->count ) {
                remove->count--;
                remove->layer->SetTransparency((remove->count * remove->alpha) / remove->fade);
            }else{
                if( remove->needdelete ) {
                    RemoveAndDelete(remove->layer, remove->image);
                }else{
                    Remove(remove->layer);
                    remove->layer->SetTransparency(remove->alpha);
                }
                remove->layer = NULL;
            }
    
        }
    }
    // Handle fade-in
    for(int i = 0; i < GUI_MAX_LAYERS; i++) {
        FrameAdd *add = &add_list[i];
        if( add->layer ) {
            if( add->delay ) {
                add->delay--;
            }else
            if( add->count ) {
                add->count--;
                if( add->layer->GetTransparency() != add->curalpha ) {
                    // someone has changed the transparency, change as new setpoint for fade
                    add->alpha = add->layer->GetTransparency();
                }
                add->curalpha = ((add->fade - add->count) * add->alpha) / add->fade;
                add->layer->SetTransparency(add->curalpha);
            }else{
                add->layer = NULL;
            }
    
        }
    }

    // Draw layers
    manager->Draw();

    Unlock();
    return stop_requested;
}

void GuiManager::RunMainFunc(void *context)
{
    ((GuiManager*)context)->gui_main_func((GuiManager*)context);
}

void GuiManager::Run(GUIFUNC_MAIN func_main)
{
    gui_main_func = func_main;

    // Initialize GameWindow
    gwd.InitVideo();
    gwd.SetBackground(0, 0, 0, 255);

    // Initialize manager
    manager = new LayerManager(GUI_MAX_LAYERS);
    render_callback = NULL;
    frame_callback = NULL;

    gwd.Run(RunMainFunc, DrawFuncWrapper, this);
}

GuiManager::GuiManager()
{
    assert( pThis == NULL );
    pThis = this;
    stop_requested = false;
    fixed_layers = 0;
    memset(remove_list, 0, sizeof(remove_list));
    memset(add_list, 0, sizeof(add_list));
}

GuiManager::~GuiManager()
{
    gwd.StopVideo();

    // Handle pending delete requests
    Lock();
    for(int i = 0; i < GUI_MAX_LAYERS; i++) {
        if( remove_list[i].layer != NULL ) {
            if( remove_list[i].needdelete ) {
                RemoveAndDelete(remove_list[i].layer, remove_list[i].image);
            }else{
                Remove(remove_list[i].layer);
            }
        }
    }
    Unlock();

    // Clean-up
    GuiManagerCallback *p = render_callback;
    render_callback = NULL;
    while( p ) {
        GuiManagerCallback *cb = p;
        p = p->next;
        delete cb;
    }
    p = frame_callback;
    frame_callback = NULL;
    while( p ) {
        GuiManagerCallback *cb = p;
        p = p->next;
        delete cb;
    }

    delete manager;
    pThis = NULL;
}

