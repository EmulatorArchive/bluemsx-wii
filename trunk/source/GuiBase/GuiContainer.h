#ifndef _GUIBASE_CONTAINER_H
#define _GUIBASE_CONTAINER_H

#define MAX_LAYERS 256

#include "GuiLayer.h"

class GuiRootContainer;
class GuiEffect;
class GuiImage;

typedef struct _GuiContainerCallback {
    bool (*callback)(void*);
    void *context;
    _GuiContainerCallback *next;
} GuiContainerCallback;

typedef struct _LayerEffect {
    GuiEffect *effect;
    GuiLayer  *add_layer;
    GuiLayer  *remove_layer;
    bool      delete_layer;
} LayerEffect;

class GuiContainer : public GuiLayer
{
    friend class GuiLayer; // GuiLayer is allowed to call 'RegisterForDelete'

public:
    GuiContainer(GuiContainer *parent, const char *name);
    virtual ~GuiContainer();

    // Callbacks
    virtual void OnDelete(GuiLayer *layer) {};

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
    void DeleteAll(void);

    virtual void AddIndex(int index, GuiLayer *layer, bool fix, GuiEffect *effect = NULL);
    virtual void AddTop(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddTopFixed(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddBottom(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void Remove(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void Delete(GuiLayer *layer);
protected:
    void RegisterForDelete(GuiLayer *layer);
    bool IsRegisteredForDelete(GuiLayer *layer);
    static GuiRootContainer *_root;

private:
    void DeleteDirect(GuiLayer *layer);
    void PrivateRemove(GuiLayer *layer, bool needdelete, GuiEffect *effect);

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
