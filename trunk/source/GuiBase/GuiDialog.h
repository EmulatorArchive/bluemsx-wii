#ifndef _GUI_DIALOG_H
#define _GUI_DIALOG_H

#include "GuiRootContainer.h"
#include "GuiContainer.h"

typedef struct _gritem GRITEM;
typedef int GRDIR;

class GuiDialog;
class GuiElement;
class GuiSprite;

class GuiDialog : public GuiContainer {
public:
    GuiDialog(GuiContainer *parent, const char *name);
    virtual ~GuiDialog();

    // Callbacks
    virtual void OnUpdateScreen(void);
    virtual void OnKey(BTN key, bool pressed);

    // Callbacks from GuiContainer
    virtual void OnDelete(GuiLayer *layer);

    void AddElement(GuiElement *element);
    void RemoveElement(GuiElement *element);

    virtual void AddIndex(int index, GuiLayer *layer, bool fix, GuiEffect *effect = NULL);
    virtual void AddIndex(int index, GuiElement *element, bool fix, GuiEffect *effect = NULL);
    virtual void AddTop(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddTop(GuiElement *element, GuiEffect *effect = NULL);
    virtual void AddTopFixed(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddTopFixed(GuiElement *element, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddOnTopOf(GuiLayer *ontopof, GuiElement *element, GuiEffect *effect = NULL);
    virtual bool AddBehind(GuiLayer *behind, GuiLayer *layer, GuiEffect *effect = NULL);
    virtual bool AddBehind(GuiLayer *behind, GuiElement *element, GuiEffect *effect = NULL);
    virtual void AddBottom(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void AddBottom(GuiElement *elementr, GuiEffect *effect = NULL);
    virtual void Remove(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void Remove(GuiElement *element, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(GuiLayer *layer, GuiEffect *effect = NULL);
    virtual void RemoveAndDelete(GuiElement *element, GuiEffect *effect = NULL);

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
    GuiElement* CheckCollision(GuiSprite *sprite);
    GuiElement* FindNearestElement(GuiElement *elm, GRDIR dir);
    bool SelectNearestElement(GuiElement *elm, GRDIR dir);

    static int running_count;
    static GuiSprite *cursor;
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
