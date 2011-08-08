#ifndef _GUI_ROOTCONTAINER_H
#define _GUI_ROOTCONTAINER_H

#include <list>

#include "GameWindow.h"
#include "GuiContainer.h"
#include "Mutex.h"

#define GUI_MAX_LAYERS 1000

extern GameWindow *g_poGwd;

class GuiRootContainer;
class GuiTextImage;
class GuiImage;

typedef void (*GUIFUNC_MAIN)(GuiRootContainer *);

typedef struct {
    GuiImage* image;
    int count;
} ImageRef;

class GuiRootContainer : public GuiContainer {
public:
    GuiRootContainer();
    virtual ~GuiRootContainer();

    GameWindow gwd;

    void Run(void);
    GameWindow *GetGameWindow(void) { return &gwd; };
    void Lock(void);
    void Unlock(void);
    void SetMode(GW_VIDEO_MODE mode);
    GW_VIDEO_MODE GetMode(void);
    u32 GetWidth(void);
    u32 GetHeight(void);

    // Image management
    GuiImage *CreateImage();
    GuiTextImage *CreateTextImage();
    void UseImage(GuiImage *image);
    void ReleaseImage(GuiImage *image);

protected:
    static void RunMainFunc(void *context);
    static bool DrawFuncWrapper(void *context);
    virtual void Main(void) = 0;
    bool DrawFunc();

private:
    CMutex mutex;
    bool stop_requested;

    // Image management
    CMutex image_lock;
    std::list<ImageRef> image_ref;
};

#endif

