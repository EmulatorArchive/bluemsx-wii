#ifndef _GUIBASE_CONTAINER_H
#define _GUIBASE_CONTAINER_H

#define MAX_LAYERS 256

#include <list>
#include "GuiLayer.h"

class GuiRootContainer;
class GuiEffect;
class GuiImage;

typedef struct _GuiContainerCallback {
    bool (*callback)(void*);
    void *context;
} GuiContainerCallback;

typedef struct _LayerEffect {
    GuiEffect *effect;
    GuiLayer  *add_layer;
    GuiLayer  *remove_layer;
} LayerEffect;

typedef std::list<GuiLayer*>::iterator LayerIndex;

class GuiContainer : public GuiLayer
{
    friend class GuiLayer; // GuiLayer is allowed to call 'RegisterForDelete'

public:
    GuiContainer(GuiContainer *parent, const char *name);
    virtual ~GuiContainer();

    // Callbacks
    virtual void OnDelete(GuiAtom *atom) {};

    // Utility
    void Lock(void);
    void Unlock(void);
    GuiContainer* GetParentContainer(void);
    void SetPointerImage(GuiImage *image);
    GuiImage* GetPointerImage(void);

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

    // Layer order management
    virtual void AddTop(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddTopFixed(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddBottom(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void Remove(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void Delete(GuiAtom *atom);

private:
    // Internal layer administration
    void LayerAdd(LayerIndex index, bool movenonfixed, GuiLayer *layer, GuiEffect *effect = NULL);
    void LayerRemove(GuiLayer* layer);
    void LayerRemoveAll();
    LayerIndex LayerGetIndex(GuiLayer* layer);

    void DeleteDirect(GuiLayer *layer);
    void PrivateRemove(GuiLayer *layer, GuiEffect *effect);

    GuiContainer *_parent;
    GuiImage* pointer_image;
    bool stop_requested;

    LayerIndex first_nonfixed;
    std::list<GuiLayer*> layers;
    std::list<GuiLayer*> layers_to_delete;
    std::list<LayerEffect> effect_list;
    std::list<GuiContainerCallback> render_callback;
    std::list<GuiContainerCallback> frame_callback;
};

#endif
