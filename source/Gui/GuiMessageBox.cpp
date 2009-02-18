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

bool GuiMessageBox::DoSelection(Sprite *yes, Sprite *no)
{
    bool use_keyboard = true;

    manager->Lock();

    // Cursor
    Sprite *cursor = new Sprite;
    cursor->SetImage(g_imgMousecursor);
    cursor->SetPosition(0, 0);
    cursor->SetVisible(false);
    manager->AddTop(cursor);

    // Set default button
    bool selected = true;
    yes->SetTransparency(255);
    no->SetTransparency(192);

    manager->Unlock();

    (void)KBD_GetPadButtons(WPAD_CHAN_0); // flush first
    (void)KBD_GetPadButtons(WPAD_CHAN_1);

    for(;;) {
        // WPAD + Infrared
        WPAD_ScanPads();
        u32 buttons = KBD_GetPadButtons(WPAD_CHAN_0) | KBD_GetPadButtons(WPAD_CHAN_1);
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);
        if( !ir.state || !ir.smooth_valid ) {
            WPAD_IR(WPAD_CHAN_1, &ir);
        }

        manager->Lock();

        if( ir.state && ir.smooth_valid ) {
            cursor->SetPosition(ir.sx, ir.sy);
            cursor->SetRotation(ir.angle/2);
            cursor->SetVisible(true);
        }else{
            cursor->SetVisible(false);
            cursor->SetPosition(0, 0);
        }

        // Check mouse cursor colisions
        if( cursor->CollidesWith(yes, true) ) {
            yes->SetTransparency(255);
            no->SetTransparency(192);
            selected = true;
            use_keyboard = false;
            if( buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) {
                break;
            }
        }else
        if( cursor->CollidesWith(no, true) ) {
            yes->SetTransparency(192);
            no->SetTransparency(255);
            selected = false;
            use_keyboard = false;
            if( buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) {
                break;
            }
        }else
        if( !use_keyboard ) {
            yes->SetTransparency(192);
            no->SetTransparency(192);
        }

        // Check keys
        if( buttons & (WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT) ) {
            yes->SetTransparency(255);
            no->SetTransparency(192);
            selected = true;
            use_keyboard = true;
        }
        if( buttons & (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT) ) {
            yes->SetTransparency(192);
            no->SetTransparency(255);
            selected = false;
            use_keyboard = true;
        }
        if( use_keyboard && buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) {
            break;
        }

        manager->Unlock();
        VIDEO_WaitVSync();
    }
    manager->Unlock();

    // Cleanup
    manager->Remove(cursor);
    delete cursor;

    return selected;
}

bool GuiMessageBox::Show(const char *txt, int txtwidth, Image *image, bool yesno, int alpha)
{
    manager->Lock();

    if( is_showing ) {
        Remove();
    }

    // container
    int sizex = image? 560 : 400;
    int sizey = 160;
    int imgoffset = 0;
    int yoffset = 0;
    if( yesno ) {
        sizex += 120;
        sizey += 40;
        yoffset -= 40;
    }
    container = new GuiContainer(320-(sizex>>1), 240-(sizey>>1), sizex, sizey, alpha);
    manager->AddTop(container->GetLayer());
    // image (optional)
    if( image ) {
        imgoffset = image->GetWidth() + 24;
        img_sprite = new Sprite;
        img_sprite->SetImage(image);
        img_sprite->SetRefPixelPosition(0, 0);
        img_sprite->SetPosition(320-(sizex>>1)+24, 240-(sizey>>1)+32+yoffset);
        manager->AddTop(img_sprite);
    }
    // yes/no buttons (optional)
    if( yesno ) {
        int x = 320-128-12;
        int y = 240+(sizey>>1)-54-24;
        spr_yes = new Sprite;
        spr_yes->SetImage(g_imgButtonYes);
        spr_yes->SetPosition(x, y);
        manager->AddTop(spr_yes);
        x = 320+12;
        y = 240+(sizey>>1)-54-24;
        spr_no = new Sprite;
        spr_no->SetImage(g_imgButtonNo);
        spr_no->SetPosition(x, y);
        manager->AddTop(spr_no);
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
    txt_sprite->SetPosition(320-(txtwidth>>1)+imgoffset, 240-30+yoffset);
    manager->AddTop(txt_sprite);
    is_showing = true;
    manager->Unlock();
    // selection
    if( yesno ) {
        return DoSelection(spr_yes, spr_no);
    }else{
        return true;
    }
}

void GuiMessageBox::Remove(void)
{
    if( is_showing ) {
        manager->Lock();
        if( txt_sprite ) {
            manager->Remove(txt_sprite);
            delete txt_sprite;
            txt_sprite = NULL;
        }
        if( txt_image ) {
            delete txt_image;
            txt_image = NULL;
        }
        if( spr_yes ) {
            manager->Remove(spr_yes);
            delete spr_yes;
            spr_yes = NULL;
        }
        if( spr_no ) {
            manager->Remove(spr_no);
            delete spr_no;
            spr_no = NULL;
        }
        if( img_sprite ) {
            manager->Remove(img_sprite);
            delete img_sprite;
            img_sprite = NULL;
        }
        if( container ) {
            manager->Remove(container->GetLayer());
            delete container;
            container = NULL;
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
    (void)Show(txt, txtwidth, image, false, alpha);
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
    spr_yes = NULL;
    spr_no = NULL;
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

