
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

void GuiDialog::OnKey(GuiRunner *runner, BTN key, bool pressed)
{
    void *rval;
    if( !pressed ) {
        return;
    }
    switch( key ) {
        case BTN_JOY1_WIIMOTE_A:
        case BTN_JOY1_CLASSIC_A:
        case BTN_JOY2_WIIMOTE_A:
        case BTN_JOY2_CLASSIC_A:
        case BTN_RETURN:
        case BTN_SPACE:
            rval = runner->GetSelected(true);
            if( rval != NULL ) {
                runner->Leave(rval);
            }
            break;
        case BTN_JOY1_WIIMOTE_B:
        case BTN_JOY1_CLASSIC_B:
        case BTN_JOY2_WIIMOTE_B:
        case BTN_JOY2_CLASSIC_B:
        case BTN_ESCAPE:
            runner->Leave(NULL);
            break;
        default:
            break;
    }
}

