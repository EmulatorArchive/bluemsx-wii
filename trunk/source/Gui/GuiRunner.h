#ifndef _GUI_RUNNER_H
#define _GUI_RUNNER_H

#include "GuiManager.h"
#include "GuiElement.h"

typedef struct _gritem GRITEM;
typedef int GRDIR;

class GuiDialog;

class GuiRunner {
public:
    GuiRunner(GuiManager *man, GuiDialog *dia);
    virtual ~GuiRunner();

    void AddElement(GuiElement *element);
    void RemoveElement(GuiElement *element);

    void AddIndex(int index, GuiElement *element, bool fix, int fade, int delay);
    void AddTop(GuiElement *element, int fade = 0, int delay = 0);
    void AddTopFixed(GuiElement *element, int fade = 0, int delay = 0);
    void AddOnTopOf(GuiElement *ontopof, GuiElement *element, int fade = 0, int delay = 0);
    void AddOnTopOf(Layer *ontopof, GuiElement *element, int fade = 0, int delay = 0);
    void AddBehind(GuiElement *behind, GuiElement *element, int fade = 0, int delay = 0);
    void AddBehind(Layer *behind, GuiElement *element, int fade = 0, int delay = 0);
    void AddBottom(GuiElement *elementr, int fade = 0, int delay = 0);
    void Remove(GuiElement *element, int fade = 0, int delay = 0);
    void RemoveAndDelete(GuiElement *element, int fade = 0, int delay = 0);

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

    GuiManager *manager;
    static int running_count;
    static Sprite *cursor;
    GuiDialog *dialog;
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
