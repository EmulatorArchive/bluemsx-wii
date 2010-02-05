
#include "GuiDialog.h"
#include "GuiRunner.h"

GuiDialog::GuiDialog()
{
}

GuiDialog::~GuiDialog()
{
}

void GuiDialog::OnUpdateScreen(GuiRunner *runner)
{
}

void GuiDialog::OnKey(GuiRunner *runner, KEY key, bool pressed)
{
    void *rval;
    if( !pressed ) {
        return;
    }
    switch( key ) {
        case KEY_JOY1_BUTTON_A:
        case KEY_JOY1_CLASSIC_BUTTON_A:
        case KEY_JOY2_BUTTON_A:
        case KEY_JOY2_CLASSIC_BUTTON_A:
        case KEY_RETURN:
        case KEY_SPACE:
            rval = runner->GetSelected(true);
            if( rval != NULL ) {
                runner->Leave(rval);
            }
            break;
        case KEY_JOY1_BUTTON_B:
        case KEY_JOY1_CLASSIC_BUTTON_B:
        case KEY_JOY2_BUTTON_B:
        case KEY_JOY2_CLASSIC_BUTTON_B:
        case KEY_ESCAPE:
            runner->Leave(NULL);
            break;
        default:
            break;
    }
}

