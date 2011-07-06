#include "GuiDlgInputLine.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiDialogs/GuiDlgMessageBox.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmSelectionList.h"

// Resources
#include "../Gui/GuiFonts.h"
#include "../Gui/GuiImages.h"


char* GuiDlgInputLine::DoModal(void)
{
    // Menu loop
    int sel;
    do {
        // Run GUI
        sel = -1;
        if( Run() ) {
            break;
        }else{
            break;
        }
    }while(sel >= 0);

    return "";
}

GuiDlgInputLine::GuiDlgInputLine(GuiContainer *parent, const char *name)
               : GuiDialog(parent, name)
{
    #define DLGINPUT_WIDH   (602+6+6)
    #define DLGINPUT_HEIGHT (180+6+6 + 64+6)

    // Set our position and size
    SetPosition((640-DLGINPUT_WIDH)/2, (440-DLGINPUT_HEIGHT)/2);
    SetWidth(DLGINPUT_WIDH);
    SetHeight(DLGINPUT_HEIGHT);

    // Frame
    sizex = DLGINPUT_WIDH;
    sizey = DLGINPUT_HEIGHT;
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BRUSHED, 0, 0, sizex, sizey, 255);
    AddTop(frame);

    // Input bar
    inpbar = new GuiSprite(this, "inpbar", g_imgInputBar, 6, 6);
    AddTop(inpbar);

    // Keyboard
    keybd = new GuiSprite(this, "keybd", g_imgKeyboard2, 6, 64+6+6);
    AddTop(keybd);
}

GuiDlgInputLine::~GuiDlgInputLine()
{
    // Remove UI elements
    RemoveAndDelete(keybd);
    RemoveAndDelete(inpbar);
    RemoveAndDelete(frame);
}

