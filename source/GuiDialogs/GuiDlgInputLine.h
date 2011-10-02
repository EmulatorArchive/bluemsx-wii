
#ifndef _GUI_DLG_INPUT_LINE_H
#define _GUI_DLG_INPUT_LINE_H

#include "../GuiBase/GuiDialog.h"

class GuiSprite;
class GuiElmFrame;

class GuiDlgInputLine : public GuiDialog {
public:
    GuiDlgInputLine(GuiContainer *parent, const char *name);
    virtual ~GuiDlgInputLine();

    int Create(void);
    char* DoModal(void);
private:
    GuiElmFrame *frame;
    GuiSprite *keybd;
    GuiSprite *inpbar;
    float sizex;
    float sizey;
};

#endif

