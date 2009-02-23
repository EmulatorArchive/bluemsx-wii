
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include "GuiManager.h"

#define GUI_DISP_STACK_SIZE 64*1024

void *GuiManager::DisplayThreadWrapper(void *arg)
{
    GuiManager *my = (GuiManager *)arg;
    my->DisplayThread();
    return NULL;
}

void GuiManager::DisplayThread(void)
{
    do {
        LWP_MutexLock(mutex);

        GuiManagerCallback *p = render;
        while( p ) {
            p->callback(p->context);
            p = p->next;
        }

        manager->Draw(0, yoffset);
        LWP_MutexUnlock(mutex);
        gwd.Flush();
    }while( !quit_thread );
}

void GuiManager::AddRenderCallback(void (*callback)(void*), void *context)
{
    GuiManagerCallback *p, *newrender = new GuiManagerCallback;
    newrender->callback = callback;
    newrender->context = context;
    newrender->next = NULL;
    if( render ) {
        p = render;
        while( p->next ) {
            p = p->next;
        }
        p->next = newrender;
    }else{
        render = newrender;
    }
}

void GuiManager::RemoveRenderCallback(void (*callback)(void*), void *context)
{
    GuiManagerCallback *cur = render;
    GuiManagerCallback *prev = NULL;
    while( cur ) {
        if( cur->callback == callback && cur->context == context ) {
            LWP_MutexLock(mutex);
            if( prev ) {
                prev->next = cur->next;
            }else{
                render = cur->next;
            }
            delete cur;
            LWP_MutexUnlock(mutex);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void GuiManager::Lock(void)
{
    LWP_MutexLock(mutex);
}

void GuiManager::Unlock(void)
{
    LWP_MutexUnlock(mutex);
}

void GuiManager::SetYOffset(int yoff)
{
    yoffset = yoff;
}

int GuiManager::GetYOffset(void)
{
    return yoffset;
}

void GuiManager::AddTop(Layer *layer)
{
    manager->Insert(layer, fixed_layers);
}

void GuiManager::AddBottom(Layer *layer)
{
    manager->Append(layer);
}

void GuiManager::Remove(Layer *layer)
{
    manager->Remove(layer);
}

void GuiManager::FixLayers(int fix)
{
    fixed_layers += fix;
}

void GuiManager::UnfixLayers(int fix)
{
    fixed_layers -= fix;
}

bool GuiManager::GetWiiMoteIR(int *x, int *y, int *angle)
{
    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);
    if( !ir.state || !ir.smooth_valid ) {
        WPAD_IR(WPAD_CHAN_1, &ir);
    }
    if( !ir.state || !ir.smooth_valid ) {
        return false;
    }
    int sx = (int)(((ir.sx - (500-200)) * 640) / 400);
    int sy = (int)(((ir.sy - (500-150)) * 480) / 300);
    if( x ) *x = sx;
    if( y ) *y = sy;
    if( angle ) *angle = ir.angle;

    return true;
}

GuiManager::GuiManager()
{
    yoffset = 0;
    fixed_layers = 0;

    // Initialize GameWindow
	gwd.InitVideo();
	gwd.SetBackground((GXColor){ 0, 0, 0, 255 });

    // Initialize manager
    manager = new LayerManager(64);

    // Start displaying
    quit_thread = false;
    LWP_MutexInit(&mutex, 1);
    thread_stack = malloc(GUI_DISP_STACK_SIZE);
    LWP_CreateThread(&thread, DisplayThreadWrapper, this,
                     thread_stack, GUI_DISP_STACK_SIZE, 90);

}

GuiManager::~GuiManager()
{
    // Stop display thread
    quit_thread = true;
    LWP_JoinThread(thread, NULL);
    LWP_MutexDestroy(mutex);

    // Clean-up
    GuiManagerCallback *p = render;
    while( p ) {
        GuiManagerCallback *cb = p;
        p = p->next;
        delete cb;
    }

    delete manager;
}

