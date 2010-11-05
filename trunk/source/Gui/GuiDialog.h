#ifndef _GUI_DIALOG_H
#define _GUI_DIALOG_H

#include "GuiRootContainer.h"
#include "GuiContainer.h"

typedef struct _gritem GRITEM;
typedef int GRDIR;

class GuiDialog;
class GuiElement;

class GuiDialog : public GuiContainer {
public:
    GuiDialog(GuiContainer *cntr);
    virtual ~GuiDialog();

    virtual void OnUpdateScreen(void);
    virtual void OnKey(BTN key, bool pressed);

    void AddElement(GuiElement *element);
    void RemoveElement(GuiElement *element);

    virtual void AddIndex(int index, Layer *layer, bool fix, GuiEffect *effect = NULL);
    virtual void AddIndex(int index, GuiElement *element, bool fix, GuiEffect *effect = NULL);
    virtual void AddTop(Layer *layer, GuiEffect *effect = NULL);
    virtual void AddTop(GuiElement *element, GuiEffect *effect = NULL);
    virtual void AddTopFixed(Layer *layer, GuiEffect *effect = NULL);
    virtual void AddTopFixed(GuiElement *element, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(Layer *ontopof, Layer *layer, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(Layer *ontopof, GuiElement *element, GuiEffect *effect = NULL);
    virtual bool AddBehind(Layer *behind, Layer *layer, GuiEffect *effect = NULL);
    virtual bool AddBehind(Layer *behind, GuiElement *element, GuiEffect *effect = NULL);
    virtual void AddBottom(Layer *layer, GuiEffect *effect = NULL);
    virtual void AddBottom(GuiElement *elementr, GuiEffect *effect = NULL);
#ifdef DEBUG
    virtual void _Remove(const char *file, int line, Layer *layer, GuiEffect *effect = NULL);
    virtual void _Remove(const char *file, int line, GuiElement *element, GuiEffect *effect = NULL);
    virtual void _RemoveAndDelete(const char *file, int line, Layer *layer, GuiEffect *effect = NULL);
    virtual void _RemoveAndDelete(const char *file, int line, GuiElement *element, GuiEffect *effect = NULL);
#else
    virtual void Remove(Layer *layer, GuiEffect *effect = NULL);
    virtual void Remove(GuiElement *element, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(Layer *layer, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(GuiElement *element, GuiEffect *effect = NULL);
#endif

    void SetSelected(GuiElement *elm, int x=0, int y=0);
    GuiElement* GetSelected(bool active_only = false);
    void* Run(bool modal = true);
    void Leave(void *retval);

    void GetKeysCallback(BTN code, int pressed);
    static void GetKeysCallbackWrapper(void *context, BTN code, int pressed);

protected:
    static bool FrameCallbackWrapper(void *context);
    bool FrameCallback(void);

private:
    GuiElement* CheckCollision(Sprite *sprite);
    GuiElement* FindNearestElement(GuiElement *elm, GRDIR dir);
    bool SelectNearestElement(GuiElement *elm, GRDIR dir);

    static int running_count;
    static Sprite *cursor;
    GRITEM *first_item;
    GRITEM *last_item;
    GuiElement *last_selected;
    GuiElement *selected_element;
    GuiElement *is_above;
    bool is_modal;
    bool use_keyboard;
    bool quit;
    void *return_value;
};

#endif
