#ifndef _GUIBASE_CONTAINER_H
#define _GUIBASE_CONTAINER_H

#define MAX_LAYERS 256

#include "GuiLayer.h"

#ifdef DEBUG
#define Remove(layer, ...) _Remove(__FILE__, __LINE__, layer, __VA_ARGS__)
#define RemoveAndDelete(layer, ...) _RemoveAndDelete(__FILE__, __LINE__, layer, __VA_ARGS__)
#define Delete(layer) _Delete(__FILE__, __LINE__, layer)
#endif

class GuiRootContainer;
class GuiEffect;
class GuiImage;

typedef struct _GuiContainerCallback {
    bool (*callback)(void*);
    void *context;
    _GuiContainerCallback *next;
} GuiContainerCallback;

typedef struct _LayerEffect {
    const char *file;
    int line;
    GuiEffect *effect;
    GuiLayer *active_layer[2];
    GuiLayer *remove_layer;
    GuiLayer *delete_layer;
} LayerEffect;

class GuiContainer : public GuiLayer
{
public:
    GuiContainer(GuiContainer *cntr);
    virtual ~GuiContainer();

    // Utility
    void Lock(void);
    void Unlock(void);
    static GuiRootContainer* GetRootContainer(void);
    GuiContainer* GetParentContainer(void);
    void SetPointerImage(GuiImage *image);
    GuiImage* GetPointerImage(void);

    // Basic layer administration
    void LayerAppend(GuiLayer* layer);
    void LayerInsert(GuiLayer* layer, u32 index);
    void LayerRemove(GuiLayer* layer);
    void LayerRemoveAll();
    int LayerGetIndex(GuiLayer* layer);
    GuiLayer* LayerGetLayerAt(u32 index) const;
    u32 LayerGetSize() const;

    // Render/Frame callbacks
    void AddRenderCallback(bool (*callback)(void*), void *context);
    void RemoveRenderCallback(bool (*callback)(void*), void *context);
    void AddFrameCallback(bool (*callback)(void*), void *context);
    void RemoveFrameCallback(bool (*callback)(void*), void *context);

    // GuiLayer
    bool IsBusy(void);
    void Draw(void);
    void SetWidth(u32 width);
    void SetHeight(u32 height);

    // Layer order management
    int GetIndex(GuiLayer *layer);
    int GetFixedLayers(void);
    void RegisterForDelete(GuiLayer *layer);
    void DeleteAll(void);

    virtual void AddIndex(int index, GuiLayer *layer, bool fix, GuiEffect *effect = NULL);
    virtual void AddTop(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddTopFixed(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddBottom(GuiLayer *layer, GuiEffect *effect = NULL);
#ifdef DEBUG
    virtual void _Remove(const char *file, int line, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void _RemoveAndDelete(const char *file, int line, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void _Delete(const char *file, int line, GuiLayer *layer);
#else
    virtual void Remove(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void Delete(GuiLayer *layer);
#endif
protected:
    static GuiRootContainer *_root;

private:
    void DeleteDirect(GuiLayer *layer);
    void PrivateRemove(const char *file, int line, GuiLayer *layer, bool needdelete, GuiEffect *effect);

    // Basic layer administration
    GuiLayer** _layers;
    u32 _size, _boundary;

    GuiContainer *_parent;
    GuiImage* pointer_image;
    bool stop_requested;
    u32 fixed_layers;
    GuiContainerCallback *render_callback;
    GuiContainerCallback *frame_callback;
    GuiLayer *layers_to_delete[MAX_LAYERS];
    LayerEffect *effect_list[MAX_LAYERS];
};

#endif
