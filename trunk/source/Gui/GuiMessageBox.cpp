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

#define MESSAGE_BOX_FADE_FRAMES       10

int GuiMessageBox::DoSelection(void)
{
    int i;
    bool use_keyboard = true;
    int active_button = -1;

    manager->Lock();

    // Cursor
    Sprite *cursor = new Sprite;
    cursor->SetImage(g_imgMousecursor);
    cursor->SetPosition(0, 0);
    cursor->SetVisible(false);
    manager->AddTop(cursor);

    // Set default button
    active_button = default_button;
    for(i = 0; i < no_buttons; i++) {
        button[i]->SetSelected( i == default_button );
    }

    manager->Unlock();

    (void)KBD_GetPadButtons(); // flush first

    for(;;) {
        // Buttons
        WPAD_ScanPads();
        u32 buttons = KBD_GetPadButtons();

        manager->Lock();

        // Infrared
        int x, y, angle;
        if( manager->GetWiiMoteIR(&x, &y, &angle) ) {
            cursor->SetPosition(x, y);
            cursor->SetRotation(angle/2);
            cursor->SetVisible(true);
        }else{
            cursor->SetVisible(false);
            cursor->SetPosition(0, 0);
        }

        // Check mouse cursor colisions
        bool is_above_button = false;
        for(i = 0; i < no_buttons; i++) {
            if( cursor->IsVisible() && button[i]->CollidesWith(cursor) ) {
                active_button = i;
                use_keyboard = false;
                is_above_button = true;
                break;
            }
        }

        // Check arrow keys
        if( !is_above_button && active_button != -1 ) {
            if( active_button > 0 && (buttons & (WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT)) ) {
                active_button--;
                use_keyboard = true;
            }
            if( active_button < no_buttons-1 && (buttons & (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT)) ) {
                active_button++;
                use_keyboard = true;
            }
        }

        // Set selected button
        for(i = 0; i < no_buttons; i++) {
            button[i]->SetSelected(i == active_button);
        }

        // Check A button
        if( (use_keyboard || is_above_button) && (buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) ) {
            break;
        }
        if( buttons & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME |
                       WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)) {
            active_button = -1;
            break;
        }

        manager->Unlock();
        VIDEO_WaitVSync();
    }
    manager->Unlock();

    // Cleanup
    manager->Remove(cursor);
    delete cursor;

    return active_button;
}

void GuiMessageBox::SetText(const char *fmt, ...)
{
    manager->Lock();

    va_list marker;
    va_start(marker,fmt);
    txt_image->RenderTextVA(true, fmt, marker);
    va_end(marker);

    manager->Unlock();
}

BTN GuiMessageBox::Show(const char *txt, Image *image, MSGT type, int alpha)
{
    manager->Lock();

    if( is_showing ) {
        Remove();
    }

    // Prepare text
    int txtwidth;
    int textheight;
    txt_image = new DrawableImage;
    txt_image->SetFont(g_fontArial);
    txt_image->SetSize(32);
    txt_image->SetYSpacing(2);
    txt_image->SetColor((GXColor){255,255,255,255});
    txt_image->GetTextSize(&txtwidth, &textheight, txt);
    txtwidth = (txtwidth + 3) & ~3;
    textheight = (textheight + 3) & ~3;
    txt_image->CreateImage(txtwidth, textheight);
    txt_image->RenderText(true, txt);

    // container
    int minsizex = image? 560 : 400;
    int sizex = txtwidth + (image? image->GetWidth()+24 : 0) + 48;
    int sizey = 100 + textheight;
    if( sizex < minsizex ) {
        sizex = minsizex;
    }
    if( type != MSGT_TEXT ) {
        sizex += 120;
        sizey += 40;
    }
    int x = manager->GetWidth()/2-sizex/2;
    int y = manager->GetHeight()/2-sizey/2;
    container = new GuiContainer(x, y, sizex, sizey, alpha);
    manager->AddTop(container, MESSAGE_BOX_FADE_FRAMES);
    sizex = container->GetWidth();
    sizey = container->GetHeight();

    // image (optional)
    if( image ) {
        img_sprite = new Sprite;
        img_sprite->SetImage(image);
        img_sprite->SetRefPixelPosition(0, 0);
        img_sprite->SetPosition(x+24, y+sizey/2-image->GetHeight()/2);
        manager->AddTop(img_sprite, MESSAGE_BOX_FADE_FRAMES);
        x += 24+image->GetWidth();
        sizex -= 24+image->GetWidth();
    }
    // yes/no/ok/cancel buttons (optional)
    char const *btntxt[3];
    Image *btnimg[3];
    BTN btnret[3];
    switch( type ) {
        case MSGT_OK:
            no_buttons = 1; default_button = 0;
            btntxt[0] = "Ok";     btnret[0] = BTN_OK;     btnimg[0] = g_imgButtonBlue;
            break;
        case MSGT_OKCANCEL:
            no_buttons = 2; default_button = 0;
            btntxt[0] = "Ok";     btnret[0] = BTN_OK;     btnimg[0] = g_imgButtonGreen;
            btntxt[1] = "Cancel"; btnret[1] = BTN_CANCEL; btnimg[1] = g_imgButtonRed;
            break;
        case MSGT_YESNO:
            no_buttons = 2; default_button = 0;
            btntxt[0] = "Yes";    btnret[0] = BTN_YES;    btnimg[0] = g_imgButtonGreen;
            btntxt[1] = "No";     btnret[1] = BTN_NO;     btnimg[1] = g_imgButtonRed;
            break;
        case MSGT_YESNOCANCEL:
            no_buttons = 3; default_button = 0;
            btntxt[0] = "Yes";    btnret[0] = BTN_YES;    btnimg[0] = g_imgButtonGreen;
            btntxt[1] = "No";     btnret[1] = BTN_NO;     btnimg[1] = g_imgButtonRed;
            btntxt[2] = "Cancel"; btnret[2] = BTN_CANCEL; btnimg[2] = g_imgButtonYellow;
            break;
        case MSGT_TEXT:
        default:
            no_buttons = 0;
            break;
    }
    if( no_buttons > 0 ) {
        int bx, by, i;
        bx = x + sizex/2 - (no_buttons - 1) * 12 - 6;
        for(i = 0; i < no_buttons; i++) {
            bx -= btnimg[i]->GetWidth() / 2;
        }
        by = y + sizey - btnimg[0]->GetHeight() - 36;
        for(i = 0; i < no_buttons; i++) {
            button[i] = new GuiButton(manager);
            button[i]->ShowImageTextHighlightButton(btnimg[i], btntxt[i], bx, by, MESSAGE_BOX_FADE_FRAMES);
            bx += btnimg[i]->GetWidth() + 24;
        }
        sizey -= btnimg[0]->GetHeight() + 36;
    }

    // text
    txt_sprite = new Sprite;
    txt_sprite->SetImage(txt_image);
    txt_sprite->SetPosition(x+(sizex-txtwidth)/2, y+(sizey-textheight)/2-4);
    manager->AddTop(txt_sprite, MESSAGE_BOX_FADE_FRAMES);
    is_showing = true;
    manager->Unlock();
    // selection
    if( type != MSGT_TEXT ) {
        int i = DoSelection();
        return ( i >= 0 )? btnret[i] : BTN_NONE;
    }else{
        return BTN_NONE;
    }
}

void GuiMessageBox::Remove(void)
{
    if( is_showing ) {
        manager->Lock();
        if( txt_sprite ) {
            manager->RemoveAndDelete(txt_sprite, txt_image, MESSAGE_BOX_FADE_FRAMES);
            txt_sprite = NULL;
            txt_image = NULL;
        }
        for(int i = 0; i < 3; i++) {
            if( button[i] ) {
                button[i]->Remove(MESSAGE_BOX_FADE_FRAMES);
                delete button[i];
                button[i] = NULL;
            }
        }
        if( img_sprite ) {
            manager->RemoveAndDelete(img_sprite, NULL, MESSAGE_BOX_FADE_FRAMES);
            img_sprite = NULL;
        }
        if( container ) {
            manager->RemoveAndDelete(container, NULL, MESSAGE_BOX_FADE_FRAMES);
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

void GuiMessageBox::ShowPopup(const char *txt, Image *image, int alpha)
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
    (void)Show(txt, image, MSGT_TEXT, alpha);
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
    for(int i = 0; i < 3; i++) {
        button[i] = NULL;
    }
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

