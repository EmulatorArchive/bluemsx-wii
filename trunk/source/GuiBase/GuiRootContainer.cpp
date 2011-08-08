
#include "GuiRootContainer.h"
#include "GuiImage.h"
#include "GuiTextImage.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

GameWindow *g_poGwd = NULL;

GuiRootContainer::GuiRootContainer()
                 :GuiContainer(NULL, "root")
{
    assert(_root == NULL); // only one root allowed
    _root = this;
    g_poGwd = &gwd;
    stop_requested = false;
}

GuiRootContainer::~GuiRootContainer()
{
	DeleteAll();
    gwd.StopVideo();
    _root = NULL;
}

/*---------------------*/

bool GuiRootContainer::DrawFuncWrapper(void *context)
{
    return ((GuiRootContainer*)context)->DrawFunc();
}

bool GuiRootContainer::DrawFunc()
{
    LayerTransform transform;
    transform.offsetX = transform.offsetY = 0.0f;
    transform.stretchWidth = transform.stretchHeight = 1.0f;
    transform.rotation = 0.0f;
    transform.alpha = 255;
    ResetTransform(transform);
    // Draw layers
    Draw();

    return stop_requested;
}

void GuiRootContainer::RunMainFunc(void *context)
{
    ((GuiRootContainer*)context)->Main();
}

void GuiRootContainer::Run(void)
{
    // Initialize GameWindow
    gwd.InitVideo();
    gwd.SetBackground(0, 0, 0, 255);

    SetWidth(gwd.GetWidth());
    SetHeight(gwd.GetHeight());

    gwd.Run(RunMainFunc, DrawFuncWrapper, this);
}

/*---------------------*/

void GuiRootContainer::Lock(void)
{
    mutex.Lock();
}

void GuiRootContainer::Unlock(void)
{
    mutex.Unlock();
}

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

// Image management

GuiImage* GuiRootContainer::CreateImage()
{
    ImageRef ref;
    GuiImage *image = new GuiImage();
    ref.image = image;
    ref.count = 1;
	image_lock.Lock();
    image_ref.push_back(ref);
	image_lock.Unlock();
    return image;
}

GuiTextImage* GuiRootContainer::CreateTextImage()
{
    ImageRef ref;
    GuiTextImage *image = new GuiTextImage();
    ref.image = image;
    ref.count = 1;
	image_lock.Lock();
    image_ref.push_back(ref);
	image_lock.Unlock();
    return image;
}

void GuiRootContainer::UseImage(GuiImage *image)
{
	image_lock.Lock();
    std::list<ImageRef>::iterator it;
    for( it = image_ref.begin(); it != image_ref.end(); ++it ) {
        ImageRef &ref = *it;
        if( ref.image == image ) {
            ref.count++;
			image_lock.Unlock();
            return;
        }
    }
	image_lock.Unlock();
    assert(0);
    return;
}

void GuiRootContainer::ReleaseImage(GuiImage *image)
{
	image_lock.Lock();
    std::list<ImageRef>::iterator it;
    for( it = image_ref.begin(); it != image_ref.end(); ++it ) {
        ImageRef &ref = *it;
        if( ref.image == image ) {
            if( --ref.count == 0 ) {
                delete image;
                image_ref.erase(it);
            }
			image_lock.Unlock();
            return;
        }
    }
	image_lock.Unlock();
    assert(0);
    return;
}

