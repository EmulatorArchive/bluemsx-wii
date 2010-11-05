#ifndef _GUI_CONTAINER_H
#define _GUI_CONTAINER_H

#include "../wiisprite/wiisprite.h"

#define MAX_LAYERS 1000

#ifdef DEBUG
#define Remove(layer, ...) _Remove(__FILE__, __LINE__, layer, __VA_ARGS__)
#define RemoveAndDelete(layer, ...) _RemoveAndDelete(__FILE__, __LINE__, layer, __VA_ARGS__)
#define Delete(layer) _Delete(__FILE__, __LINE__, layer)
#endif

class GuiRootContainer;
class GuiEffect;

typedef struct _GuiContainerCallback {
    bool (*callback)(void*);
    void *context;
    _GuiContainerCallback *next;
} GuiContainerCallback;

typedef struct _LayerEffect {
    const char *file;
    int line;
    GuiEffect *effect;
    Layer *active_layer[2];
    Layer *remove_layer;
    Layer *delete_layer;
} LayerEffect;

class GuiContainer : public Layer
{
public:
    GuiContainer(GuiContainer *cntr);
    virtual ~GuiContainer();

    // Layer
    bool IsBusy(void);
    void Draw(void);

    // Basic layer administration
    void LayerAppend(Layer* layer);
    void LayerInsert(Layer* layer, u32 index);
    void LayerRemove(Layer* layer);
    void LayerRemoveAll();
    int LayerGetIndex(Layer* layer);
    Layer* LayerGetLayerAt(u32 index) const;
    u32 LayerGetSize() const;
    void LayerDraw(f32 offsetX, f32 offsetY, f32 rot, u8 alpha);

    // Container
    static GuiRootContainer* GetRootContainer(void);
    GuiContainer* GetParentContainer(void);
    void SetWidth(u32 width);
    void SetHeight(u32 height);
    void AddRenderCallback(bool (*callback)(void*), void *context);
    void RemoveRenderCallback(bool (*callback)(void*), void *context);
    void AddFrameCallback(bool (*callback)(void*), void *context);
    void RemoveFrameCallback(bool (*callback)(void*), void *context);
    void Lock(void);
    void Unlock(void);
    int GetIndex(Layer *layer);
    int GetFixedLayers(void);

    void RegisterForDelete(Layer *layer);
    void DeleteAll(void);

    virtual void AddIndex(int index, Layer *layer, bool fix, GuiEffect *effect = NULL);
    virtual void AddTop(Layer *layer, GuiEffect *effect = NULL);
    virtual void AddTopFixed(Layer *layer, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(Layer *ontopof, Layer *layer, GuiEffect *effect = NULL);
    virtual bool AddBehind(Layer *behind, Layer *layer, GuiEffect *effect = NULL);
    virtual void AddBottom(Layer *layer, GuiEffect *effect = NULL);
#ifdef DEBUG
    virtual void _Remove(const char *file, int line, Layer *layer, GuiEffect *effect = NULL);
    virtual void _RemoveAndDelete(const char *file, int line, Layer *layer, GuiEffect *effect = NULL);
    virtual void _Delete(const char *file, int line, Layer *layer);
#else
    virtual void Remove(Layer *layer, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(Layer *layer, GuiEffect *effect = NULL);
    virtual void Delete(Layer *layer);
#endif
protected:
    static void RunMainFunc(void *context);
    static bool DrawFuncWrapper(void *context);
    bool DrawFunc();

    static GuiRootContainer *_root;

private:
    void DeleteDirect(Layer *layer);
    void PrivateRemove(const char *file, int line, Layer *layer, bool needdelete, GuiEffect *effect);

    GuiContainer *_parent;

    // Basic layer administration
    Layer** _layers;
    u32 _size, _boundary;

    CMutex mutex;
    bool stop_requested;
    u32 fixed_layers;
    GuiContainerCallback *render_callback;
    GuiContainerCallback *frame_callback;
    Layer *layers_to_delete[MAX_LAYERS];
    LayerEffect *effect_list[MAX_LAYERS];
};

#endif
