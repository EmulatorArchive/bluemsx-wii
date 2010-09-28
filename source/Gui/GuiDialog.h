#ifndef _GUI_DIALOG_H
#define _GUI_DIALOG_H

#include "GuiManager.h"

class GuiRunner;

class GuiDialog {
public:
    GuiDialog();
    virtual ~GuiDialog();

    virtual void OnUpdateScreen(GuiRunner *runner);
    virtual void OnKey(GuiRunner *runner, BTN key, bool pressed);
};

#endif
