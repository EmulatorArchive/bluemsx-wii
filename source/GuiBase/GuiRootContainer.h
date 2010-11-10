#ifndef _GUI_ROOTCONTAINER_H
#define _GUI_ROOTCONTAINER_H

#include "GameWindow.h"
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

    void Run(void);
    GameWindow *GetGameWindow(void) { return &gwd; };
    void SetMode(GW_VIDEO_MODE mode);
    GW_VIDEO_MODE GetMode(void);
    u32 GetWidth(void);
    u32 GetHeight(void);
protected:
    static void RunMainFunc(void *context);
    static bool DrawFuncWrapper(void *context);
    virtual void Main(void) = 0;
    bool DrawFunc();

private:
    bool stop_requested;
};

#endif

