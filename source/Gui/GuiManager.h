#ifndef _GUI_MANAGER_H
#define _GUI_MANAGER_H

#include "../wiisprite/wiisprite.h"

#define GUI_MAX_LAYERS 1000

class GuiManager;
typedef void (*GUIFUNC_MAIN)(GuiManager *);

typedef struct _GuiManagerCallback {
    bool (*callback)(void*);
    void *context;
    _GuiManagerCallback *next;
} GuiManagerCallback;

typedef struct _FrameRemove {
    Layer *layer;
    Image *image;
    bool needdelete;
    int fade;
    int delay;
    int count;
    int alpha;
} FrameRemove;

typedef struct _FrameAdd {
    Layer *layer;
    int fade;
    int delay;
    int count;
    int alpha;
    int curalpha;
} FrameAdd;

class GuiManager {
public:
    GuiManager();
    virtual ~GuiManager();

    GameWindow gwd;

    void Run(GUIFUNC_MAIN func_main);
    GameWindow *GetGameWindow(void) { return &gwd; };
    void SetMode(GW_VIDEO_MODE mode);
    GW_VIDEO_MODE GetMode(void);
    u32 GetWidth(void);
    u32 GetHeight(void);
    void AddRenderCallback(bool (*callback)(void*), void *context);
    void RemoveRenderCallback(bool (*callback)(void*), void *context);
    void AddFrameCallback(bool (*callback)(void*), void *context);
    void RemoveFrameCallback(bool (*callback)(void*), void *context);
    void Lock(void);
    void Unlock(void);
    int GetIndex(Layer *layer);
    int GetFixedLayers(void);
    void AddIndex(int index, Layer *layer, bool fix, int fade = 0, int delay = 0);
    void AddTop(Layer *layer, int fade = 0, int delay = 0);
    void AddTopFixed(Layer *layer, int fade = 0, int delay = 0);
    void AddOnTopOf(Layer *ontopof, Layer *layer, int fade = 0, int delay = 0);
    void AddBehind(Layer *behind, Layer *layer, int fade = 0, int delay = 0);
    void AddBottom(Layer *layer, int fade = 0, int delay = 0);
    void Remove(Layer *layer, int fade = 0, int delay = 0);
    void RemoveAndDelete(Layer *layer, Image *image = NULL, int fade = 0, int delay = 0);
    int WriteScreenshot(const char *fname) { return gwd.WriteScreenshot(fname); };
protected:
    static void RunMainFunc(void *context);
    static bool DrawFuncWrapper(void *context);
    bool DrawFunc();

private:
    void RegisterRemove(Layer *layer, bool needdelete, int fade, int delay, Image *image);

    static GuiManager *pThis;
    LayerManager *manager;
    CMutex mutex;
    bool stop_requested;
    int fixed_layers;
    GuiManagerCallback *render_callback;
    GuiManagerCallback *frame_callback;
    GUIFUNC_MAIN gui_main_func;
    FrameRemove remove_list[GUI_MAX_LAYERS];
    FrameAdd add_list[GUI_MAX_LAYERS];
};

#endif

