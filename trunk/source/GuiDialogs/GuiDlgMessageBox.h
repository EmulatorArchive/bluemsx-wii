
#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include <stdarg.h>

#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiSprite.h"

#include "../GuiElements/GuiElmButton.h"
#include "../GuiElements/GuiElmFrame.h"

typedef enum {
  MSGT_TEXT,
  MSGT_OK,
  MSGT_OKCANCEL,
  MSGT_YESNO,
  MSGT_YESNOCANCEL
} MSGT;

typedef enum {
  MSGBTN_NONE,
  MSGBTN_OK,
  MSGBTN_CANCEL,
  MSGBTN_YES,
  MSGBTN_NO
} MSGBTN;

class GuiDlgMessageBox : public GuiDialog {
public:
    GuiDlgMessageBox(GuiContainer *parent, const char *name);
    virtual ~GuiDlgMessageBox();

    static void ShowPopup(GuiContainer *parent, const char *name, const char *image, float alpha, int delay,
                          GuiEffect *effa, GuiEffect *effb, const char *txt, ...);
    static MSGBTN ShowModal(GuiContainer *parent, const char *name, MSGT type, const char *image, float alpha,
                            GuiEffect *effa, GuiEffect *effb, const char *txt, ...);

    static void ShowPopup(GuiContainer *parent, const char *name, const char *image, float alpha, int delay,
                          GuiEffect &effa, GuiEffect &effb, const char *txt, ...);
    static MSGBTN ShowModal(GuiContainer *parent, const char *name, MSGT type, const char *image, float alpha,
                            GuiEffect &effa, GuiEffect &effb, const char *txt, ...);

    void Create(MSGT type, const char *image, float alpha, const char *txt, ...);
    void CreateVA(MSGT type, const char *image, float alpha, const char *txt, va_list valist);

private:
    static MSGBTN RunModal(GuiDlgMessageBox *msgbox, MSGT type);
    void CleanUp(void);

    MSGT btn_type;
    MSGBTN buttons[3];
    GuiContainer *container;
    GuiElmFrame *frame;
    GuiSprite *txt_sprite;
    GuiSprite *img_sprite;
    GuiElmButton *button[3];
    int no_buttons;
    int default_button;

    int DoSelection(void);
};

#endif


