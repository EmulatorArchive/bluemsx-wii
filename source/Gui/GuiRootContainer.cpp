
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GuiRootContainer.h"

GameWindow *g_poGwd = NULL;

GuiRootContainer::GuiRootContainer()
                 :GuiContainer(NULL)
{
    assert(_root == NULL); // only one root allowed
    _root = this;
    g_poGwd = &gwd;
    stop_requested = false;
}

GuiRootContainer::~GuiRootContainer()
{
    gwd.StopVideo();
}

/*---------------------*/

bool GuiRootContainer::DrawFuncWrapper(void *context)
{
    return ((GuiRootContainer*)context)->DrawFunc();
}

bool GuiRootContainer::DrawFunc()
{
    // Draw layers
    Draw();

    return stop_requested;
}

void GuiRootContainer::RunMainFunc(void *context)
{
    ((GuiRootContainer*)context)->gui_main_func((GuiRootContainer*)context);
}

void GuiRootContainer::Run(GUIFUNC_MAIN func_main)
{
    gui_main_func = func_main;

    // Initialize GameWindow
    gwd.InitVideo();
    gwd.SetBackground(0, 0, 0, 255);

    SetWidth(gwd.GetWidth());
    SetHeight(gwd.GetHeight());

    gwd.Run(RunMainFunc, DrawFuncWrapper, this);
}

/*---------------------*/

GW_VIDEO_MODE GuiRootContainer::GetMode(void)
{
    return gwd.GetMode();
}

u32 GuiRootContainer::GetWidth(void)
{
    return gwd.GetWidth();
}

u32 GuiRootContainer::GetHeight(void)
{
    return gwd.GetHeight();
}

void GuiRootContainer::SetMode(GW_VIDEO_MODE mode)
{
    gwd.SetMode(mode);
}


