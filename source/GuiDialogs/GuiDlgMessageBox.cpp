#include "GuiDlgMessageBox.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../arch/archThread.h"

#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"
#include "../GuiElements/GuiElmFrame.h"


int GuiDlgMessageBox::DoSelection(void)
{
    int i;

    // Set default button
    SetSelected(button[default_button]);

    // Run GUI
    GuiElmButton *selected = (GuiElmButton *)( Run() );

    // Return
    for(i = 0; i < no_buttons; i++) {
        if( button[i] == selected ) {
            return i;
        }
    }
    return -1;
}

void GuiDlgMessageBox::CleanUp(void)
{
    if( txt_sprite ) {
        RemoveAndDelete(txt_sprite);
        txt_sprite = NULL;
    }
    for(int i = 0; i < 3; i++) {
        if( button[i] ) {
            Remove(button[i]);
            Delete(button[i]);
            button[i] = NULL;
        }
    }
    if( img_sprite ) {
        RemoveAndDelete(img_sprite);
        img_sprite = NULL;
    }
    if( frame ) {
        RemoveAndDelete(frame);
        frame = NULL;
    }
}

void GuiDlgMessageBox::ShowPopup(GuiContainer *parent, const char *name, GuiImage *image, int alpha, int delay, GuiEffect *effa, GuiEffect *effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(parent, name);
    msgbox->CreateVA(MSGT_TEXT, image, alpha, txt, marker);
    parent->AddTop(msgbox, effa);
    archThreadSleep(delay);
    parent->RemoveAndDelete(msgbox, effb);

    va_end(marker);
}

MSGBTN GuiDlgMessageBox::ShowModal(GuiContainer *parent, const char *name, MSGT type, GuiImage *image, int alpha, GuiEffect *effa, GuiEffect *effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    // Create and show
    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(parent, name);
    msgbox->CreateVA(type, image, alpha, txt, marker);
    parent->AddTop(msgbox, effa);

    // selection
    MSGBTN return_value;
    if( type != MSGT_TEXT ) {
        int i = msgbox->DoSelection();
        return_value = ( i >= 0 )? msgbox->buttons[i] : MSGBTN_NONE;
    }else{
        return_value = MSGBTN_NONE;
    }

    // Remove
    parent->RemoveAndDelete(msgbox, effb);

    return return_value;
}


void GuiDlgMessageBox::Create(MSGT type, GuiImage *image, int alpha, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    CreateVA(type, image, alpha, txt, marker);

    va_end(marker);
}

void GuiDlgMessageBox::CreateVA(MSGT type, GuiImage *image, int alpha, const char *txt, va_list valist)
{
    CleanUp();

    btn_type = type;

    // prepare text
    GXColor white={255,255,255,255};
    txt_sprite = new GuiSprite(this, "text");
    txt_sprite->CreateTextImageVA(g_fontArial, 32, 0, 0, true, white, txt, valist);

    // frame
    int minsizex = image? 560 : 400;
    int sizex = txt_sprite->GetWidth() + (image? image->GetWidth()+24 : 0) + 48;
    int sizey = 100 + txt_sprite->GetHeight();
    if( sizex < minsizex ) {
        sizex = minsizex;
    }
    if( type != MSGT_TEXT ) {
        sizex += 120;
        sizey += 40;
    }
    SetPosition((int)GetParentContainer()->GetWidth()/2-sizex/2,
                (int)GetParentContainer()->GetHeight()/2-sizey/2);
    SetWidth(sizex);
    SetHeight(sizey);
    SetRefPixelPosition((f32)(sizex/2), (f32)(sizey/2));
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, 0, 0, sizex, sizey, alpha);
    AddTop(frame);

    s32 x = 0, y = 0;
    // image (optional)
    if( image ) {
        img_sprite = new GuiSprite(this, "image");
        img_sprite->SetImage(image);
        img_sprite->SetRefPixelPosition(0, 0);
        img_sprite->SetPosition(x+24, y+sizey/2-image->GetHeight()/2);
        AddTop(img_sprite);
        x += 24+image->GetWidth();
        sizex -= 24+image->GetWidth();
    }
    // yes/no/ok/cancel buttons (optional)
    char const *btntxt[3];
    GuiImage *btnimg[3];
    switch( type ) {
        case MSGT_OK:
            no_buttons = 1; default_button = 0;
            btntxt[0] = "Ok";     buttons[0] = MSGBTN_OK;     btnimg[0] = g_imgButtonBlue;
            break;
        case MSGT_OKCANCEL:
            no_buttons = 2; default_button = 0;
            btntxt[0] = "Ok";     buttons[0] = MSGBTN_OK;     btnimg[0] = g_imgButtonGreen;
            btntxt[1] = "Cancel"; buttons[1] = MSGBTN_CANCEL; btnimg[1] = g_imgButtonRed;
            break;
        case MSGT_YESNO:
            no_buttons = 2; default_button = 0;
            btntxt[0] = "Yes";    buttons[0] = MSGBTN_YES;    btnimg[0] = g_imgButtonGreen;
            btntxt[1] = "No";     buttons[1] = MSGBTN_NO;     btnimg[1] = g_imgButtonRed;
            break;
        case MSGT_YESNOCANCEL:
            no_buttons = 3; default_button = 0;
            btntxt[0] = "Yes";    buttons[0] = MSGBTN_YES;    btnimg[0] = g_imgButtonGreen;
            btntxt[1] = "No";     buttons[1] = MSGBTN_NO;     btnimg[1] = g_imgButtonRed;
            btntxt[2] = "Cancel"; buttons[2] = MSGBTN_CANCEL; btnimg[2] = g_imgButtonYellow;
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
        by = y + sizey - btnimg[0]->GetHeight() - 32;
        for(i = 0; i < no_buttons; i++) {
            button[i] = new GuiElmButton(this, "button");
            button[i]->CreateImageTextHighlightButton(btnimg[i], btntxt[i]);
            button[i]->SetPosition(bx, by);
            AddTop(button[i]);
            bx += btnimg[i]->GetWidth() + 24;
        }
        sizey -= btnimg[0]->GetHeight() + 32;
    }

    // text
    txt_sprite->SetPosition(x+(sizex-(s32)txt_sprite->GetWidth())/2, y+(sizey-(s32)txt_sprite->GetHeight())/2);
    AddTop(txt_sprite);
}

GuiDlgMessageBox::GuiDlgMessageBox(GuiContainer *parent, const char *name)
                 :GuiDialog(parent, name)
{
    memset(button, 0, sizeof(button));
    btn_type = MSGT_TEXT;
    frame = NULL;
    txt_sprite = NULL;
    img_sprite = NULL;
    for(int i = 0; i < 3; i++) {
        button[i] = NULL;
    }
}

GuiDlgMessageBox::~GuiDlgMessageBox()
{
    CleanUp();
}

