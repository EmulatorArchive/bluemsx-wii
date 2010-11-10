#include "GuiDlgMessageBox.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../arch/archThread.h"

#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"
#include "../GuiLayers/GuiLayFrame.h"


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

void GuiDlgMessageBox::ShowPopup(GuiContainer *cntr, GuiImage *image, int alpha, int delay, GuiEffect *effa, GuiEffect *effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(cntr);
    cntr->RegisterForDelete(msgbox);
    msgbox->CreateVA(MSGT_TEXT, image, alpha, txt, marker);
    cntr->AddTop(msgbox, effa);
    archThreadSleep(delay);
    cntr->RemoveAndDelete(msgbox, effb);

    va_end(marker);
}

MSGBTN GuiDlgMessageBox::ShowModal(GuiContainer *cntr, MSGT type, GuiImage *image, int alpha, GuiEffect *effa, GuiEffect *effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    // Create and show
    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(cntr);
    cntr->RegisterForDelete(msgbox);
    msgbox->CreateVA(type, image, alpha, txt, marker);
    cntr->AddTop(msgbox, effa);

    // selection
    MSGBTN return_value;
    if( type != MSGT_TEXT ) {
        int i = msgbox->DoSelection();
        return_value = ( i >= 0 )? msgbox->buttons[i] : MSGBTN_NONE;
    }else{
        return_value = MSGBTN_NONE;
    }

    // Remove
    cntr->RemoveAndDelete(msgbox, effb);

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
    txt_sprite = new GuiSprite;
    txt_sprite->CreateTextImageVA(g_fontArial, 32, 0, 0, true, white, txt, valist);
    RegisterForDelete(txt_sprite);

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
    frame = new GuiLayFrame(0, 0, sizex, sizey, alpha);
    RegisterForDelete(frame);
    AddTop(frame);
    sizex = frame->GetWidth();
    sizey = frame->GetHeight();

    s32 x = 0, y = 0;
    // image (optional)
    if( image ) {
        img_sprite = new GuiSprite;
        img_sprite->SetImage(image);
        img_sprite->SetRefPixelPosition(0, 0);
        img_sprite->SetPosition(x+24, y+sizey/2-image->GetHeight()/2);
        RegisterForDelete(img_sprite);
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
        by = y + sizey - btnimg[0]->GetHeight() - 36;
        for(i = 0; i < no_buttons; i++) {
            button[i] = new GuiElmButton();
            button[i]->CreateImageTextHighlightButton(btnimg[i], btntxt[i]);
            button[i]->SetPosition(bx, by);
            RegisterForDelete(button[i]);
            AddTop(button[i]);
            bx += btnimg[i]->GetWidth() + 24;
        }
        sizey -= btnimg[0]->GetHeight() + 36;
    }

    // text
    txt_sprite->SetPosition(x+(sizex-(s32)txt_sprite->GetWidth())/2, y+(sizey-(s32)txt_sprite->GetHeight())/2-4);
    AddTop(txt_sprite);
}

GuiDlgMessageBox::GuiDlgMessageBox(GuiContainer *cntr)
              :GuiDialog(cntr)
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

