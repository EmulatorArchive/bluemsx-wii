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
    button[default_button]->SetFocus(true);

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

void GuiDlgMessageBox::ShowPopup(GuiContainer *parent, const char *name, const char *image, float alpha, int delay, GuiEffect *effa, GuiEffect *effb, const char *txt, ...)
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

void GuiDlgMessageBox::ShowPopup(GuiContainer *parent, const char *name, const char *image, float alpha, int delay, GuiEffect &effa, GuiEffect &effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(parent, name);
    msgbox->CreateVA(MSGT_TEXT, image, alpha, txt, marker);
    parent->AddTop(msgbox, effa.Clone());
    archThreadSleep(delay);
    parent->RemoveAndDelete(msgbox, effb.Clone());

    va_end(marker);
}

MSGBTN GuiDlgMessageBox::RunModal(GuiDlgMessageBox *msgbox, MSGT type)
{
    MSGBTN return_value;
    if( type != MSGT_TEXT ) {
        int i = msgbox->DoSelection();
        return_value = ( i >= 0 )? msgbox->buttons[i] : MSGBTN_NONE;
    }else{
        return_value = MSGBTN_NONE;
    }
    return return_value;
}

MSGBTN GuiDlgMessageBox::ShowModal(GuiContainer *parent, const char *name, MSGT type, const char *image, float alpha, GuiEffect *effa, GuiEffect *effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    // Create and show
    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(parent, name);
    msgbox->CreateVA(type, image, alpha, txt, marker);
    parent->AddTop(msgbox, effa);

    // selection
    MSGBTN return_value = RunModal(msgbox, type);

    // Remove
    parent->RemoveAndDelete(msgbox, effb);

    return return_value;
}

MSGBTN GuiDlgMessageBox::ShowModal(GuiContainer *parent, const char *name, MSGT type, const char *image, float alpha, GuiEffect &effa, GuiEffect &effb, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    // Create and show
    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(parent, name);
    msgbox->CreateVA(type, image, alpha, txt, marker);
    parent->AddTop(msgbox, effa.Clone());

    // selection
    MSGBTN return_value = RunModal(msgbox, type);

    // Remove
    parent->RemoveAndDelete(msgbox, effb.Clone());

    return return_value;
}


void GuiDlgMessageBox::Create(MSGT type, const char *image, float alpha, const char *txt, ...)
{
    va_list marker;
    va_start(marker, txt);

    CreateVA(type, image, alpha, txt, marker);

    va_end(marker);
}

void GuiDlgMessageBox::CreateVA(MSGT type, const char *image, float alpha, const char *txt, va_list valist)
{
    CleanUp();

    btn_type = type;

    // prepare text
    GXColor txtcol={255,255,255,255};
    txt_sprite = new GuiSprite(this, "text");
    txt_sprite->CreateTextImageVA(g_fontArial, 32, 0, 0, true, txtcol, txt, valist);

    // prepare image (optional)
    if( image ) {
        img_sprite = new GuiSprite(this, "image");
        GuiImages::AssignSpriteToImage(img_sprite, image);
    }

    // frame
    float minsizex = image? 560.0f : 400.0f;
    float sizex = txt_sprite->GetWidth() + (image? img_sprite->GetWidth()+24 : 0) + 48;
    float sizey = txt_sprite->GetHeight() + 100;
    if( sizex < minsizex ) {
        sizex = minsizex;
    }
    if( type != MSGT_TEXT ) {
        sizex += 120;
        sizey += 40;
    }
    SetPosition(GetParentContainer()->GetWidth()/2-sizex/2,
                GetParentContainer()->GetHeight()/2-sizey/2);
    SetWidth(sizex);
    SetHeight(sizey);
    SetRefPixelPosition(sizex/2, sizey/2);
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, 0, 0, sizex, sizey, alpha);
    AddTop(frame);

    float x = 0, y = 0;
    // image (optional)
    if( image ) {
        img_sprite->SetPosition(x+24, y+sizey/2-img_sprite->GetHeight()/2);
        AddTop(img_sprite);
        x += img_sprite->GetWidth() + 24;
        sizex -= img_sprite->GetWidth() + 24;
    }
    // yes/no/ok/cancel buttons (optional)
    char const *btntxt[3];
    const char *btnimg[3];
    switch( type ) {
        case MSGT_OK:
            no_buttons = 1; default_button = 0;
            btntxt[0] = "Ok";     buttons[0] = MSGBTN_OK;     btnimg[0] = "image_button_blue";
            break;
        case MSGT_OKCANCEL:
            no_buttons = 2; default_button = 0;
            btntxt[0] = "Ok";     buttons[0] = MSGBTN_OK;     btnimg[0] = "image_button_green";
            btntxt[1] = "Cancel"; buttons[1] = MSGBTN_CANCEL; btnimg[1] = "image_button_red";
            break;
        case MSGT_YESNO:
            no_buttons = 2; default_button = 0;
            btntxt[0] = "Yes";    buttons[0] = MSGBTN_YES;    btnimg[0] = "image_button_green";
            btntxt[1] = "No";     buttons[1] = MSGBTN_NO;     btnimg[1] = "image_button_red";
            break;
        case MSGT_YESNOCANCEL:
            no_buttons = 3; default_button = 0;
            btntxt[0] = "Yes";    buttons[0] = MSGBTN_YES;    btnimg[0] = "image_button_green";
            btntxt[1] = "No";     buttons[1] = MSGBTN_NO;     btnimg[1] = "image_button_red";
            btntxt[2] = "Cancel"; buttons[2] = MSGBTN_CANCEL; btnimg[2] = "image_button_yellow";
            break;
        case MSGT_TEXT:
        default:
            no_buttons = 0;
            break;
    }
    if( no_buttons > 0 ) {
        float bx, by;
        for(int i = 0; i < no_buttons; i++) {
            button[i] = new GuiElmButton(this, "button");
            button[i]->CreateImageTextHighlightButton(btnimg[i], btntxt[i]);
            AddTop(button[i]);
        }
        bx = x + sizex/2 - (no_buttons - 1) * 12 - 6;
        for(int i = 0; i < no_buttons; i++) {
            bx -= button[i]->GetWidth() / 2;
        }
        by = y + sizey - button[0]->GetHeight() - 32;
        for(int i = 0; i < no_buttons; i++) {
            button[i]->SetPosition(bx, by);
            bx += button[i]->GetWidth() + 24;
        }
        sizey -= button[0]->GetHeight() + 32;
    }

    // text
    txt_sprite->SetPosition(x+(sizex-txt_sprite->GetWidth())/2, y+(sizey-txt_sprite->GetHeight())/2);
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

