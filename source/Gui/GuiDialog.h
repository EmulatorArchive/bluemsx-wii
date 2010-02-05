#ifndef _GUI_DIALOG_H
#define _GUI_DIALOG_H

#include "GuiManager.h"
#include "kbdlib.h"

class GuiRunner;

class GuiDialog {
public:
    GuiDialog();
    virtual ~GuiDialog();

    virtual void OnUpdateScreen(GuiRunner *runner);
    virtual void OnKey(GuiRunner *runner, KEY key, bool pressed);
};

#endif
