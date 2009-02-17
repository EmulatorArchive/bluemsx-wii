#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include <wiiuse/wpad.h>

#include "kbdlib.h"
#include "GuiMessageBox.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

extern "C" {
#include "ArchThread.h"
}

void GuiMessageBox::Show(const char *txt, int txtwidth, Image *image, int alpha)
{
    manager->Lock();

    if( is_showing ) {
        Remove();
    }

    // container
    int sizex = image? 560 : 400;
    int sizey = 160;
    int imgoffset = 0;
    container = new GuiContainer(320-(sizex>>1), 240-(sizey>>1), sizex, sizey, alpha);
    manager->AddTop(container->GetLayer());
    // image (optional)
    if( image ) {
        imgoffset = image->GetWidth() + 24;
        img_sprite = new Sprite;
        img_sprite->SetImage(image);
        img_sprite->SetRefPixelPosition(0, 0);
        img_sprite->SetPosition(320-(sizex>>1)+24, 240-(sizey>>1)+32);
        manager->AddTop(img_sprite);
    }
    // text
    txt_image = new DrawableImage;
    txt_image->CreateImage(txtwidth, 60);
    txt_image->SetFont(g_fontArial);
    txt_image->SetSize(32);
    txt_image->SetColor((GXColor){255,255,255,255});
    txt_image->RenderText(txt);
    txt_sprite = new Sprite;
    txt_sprite->SetImage(txt_image->GetImage());
    txt_sprite->SetPosition(320-(txtwidth>>1)+imgoffset, 240-30);
    manager->AddTop(txt_sprite);
    is_showing = true;
    manager->Unlock();
}

void GuiMessageBox::Remove(void)
{
    if( is_showing ) {
        manager->Lock();
        if( img_sprite ) {
            manager->Remove(img_sprite);
            delete img_sprite;
            img_sprite = NULL;
        }
        if( txt_sprite ) {
            manager->Remove(txt_sprite);
            delete txt_sprite;
            txt_sprite = NULL;
        }
        if( container ) {
            manager->Remove(container->GetLayer());
            delete container;
            container = NULL;
        }
        if( txt_image ) {
            delete txt_image;
            txt_image = NULL;
        }
        is_showing = false;
        manager->Unlock();
    }
}

static GuiMessageBox *myself;
static void MessageBoxPopupThreadWrapper(void)
{
    myself->MessageBoxPopupThread();
}

void GuiMessageBox::MessageBoxPopupThread(void)
{
    // Wait about 2 seconds
    for(int i = 0; !quit_thread && i < 20; i++) {
        archThreadSleep(100);
    }
    // Remove the popup
    Remove();
}

void GuiMessageBox::ShowPopup(const char *txt, int txtwidth, Image *image, int alpha)
{
    // Terminate old thread
    if( thread_popup ) {
        quit_thread = true;
        archThreadJoin(thread_popup, -1);
        archThreadDestroy(thread_popup);
        thread_popup = NULL;
    }
    manager->Lock();
    // Show popup
    Show(txt, txtwidth, image, alpha);
    // Start thread
    myself = this;
    quit_thread = false;
    thread_popup = archThreadCreateEx(MessageBoxPopupThreadWrapper, THREAD_PRIO_NORMAL, 16*1024);
    // done
    manager->Unlock();
}

GuiMessageBox::GuiMessageBox(GuiManager *man)
{
    manager = man;
    is_showing = false;
    container = NULL;
    txt_sprite = NULL;
    img_sprite = NULL;
    txt_image = NULL;
    thread_popup = NULL;
}

GuiMessageBox::~GuiMessageBox()
{
    if( is_showing ) {
        Remove();
    }
    if( thread_popup ) {
        quit_thread = true;
        archThreadJoin(thread_popup, -1);
        archThreadDestroy(thread_popup);
    }
}

