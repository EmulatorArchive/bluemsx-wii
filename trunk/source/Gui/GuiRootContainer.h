#ifndef _GUI_MANAGER_H
#define _GUI_MANAGER_H

#include "../wiisprite/wiisprite.h"
#include "GuiContainer.h"

#define GUI_MAX_LAYERS 1000

extern GameWindow *g_poGwd;

class GuiRootContainer;
typedef void (*GUIFUNC_MAIN)(GuiRootContainer *);

class GuiRootContainer : public GuiContainer {
public:
    GuiRootContainer();
    virtual ~GuiRootContainer();

    GameWindow gwd;

    void Run(GUIFUNC_MAIN func_main);
    GameWindow *GetGameWindow(void) { return &gwd; };
    void SetMode(GW_VIDEO_MODE mode);
    GW_VIDEO_MODE GetMode(void);
    u32 GetWidth(void);
    u32 GetHeight(void);
protected:
    static void RunMainFunc(void *context);
    static bool DrawFuncWrapper(void *context);
    bool DrawFunc();

private:
    bool stop_requested;
    GUIFUNC_MAIN gui_main_func;
};

#endif

