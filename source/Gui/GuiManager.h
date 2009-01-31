#ifndef _GUI_MANAGER_H
#define _GUI_MANAGER_H

#include <gccore.h>
#include <wiisprite.h>

using namespace wsp;

typedef struct _GuiManagerCallback {
    void (*callback)(void*);
    void *context;
    _GuiManagerCallback *next;
} GuiManagerCallback;
    
class GuiManager {
public:
    GuiManager();
    virtual ~GuiManager();
    void AddRenderCallback(void (*callback)(void*), void *context);
    void RemoveRenderCallback(void (*callback)(void*), void *context);
    static void *DisplayThreadWrapper(void *arg);
    void DisplayThread(void);
    void SetYOffset(int yoff);
    void Lock(void);
    void Unlock(void);
    void AddTop(Layer *layer);
    void AddBottom(Layer *layer);
    void Remove(Layer *layer);
    void FixLayers(int fix);
    void UnfixLayers(int fix);

private:
    GameWindow gwd;
    LayerManager *manager;
    mutex_t mutex;
    lwp_t thread;
    void *thread_stack;
    bool quit_thread;
    int yoffset;
    int fixed_layers;
    GuiManagerCallback *render;
};

#endif

