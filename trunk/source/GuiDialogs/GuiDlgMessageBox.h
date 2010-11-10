
#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include <stdarg.h>

#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiDialog.h"
#include "../GuiBase/GuiSprite.h"

#include "../GuiElements/GuiElmButton.h"
#include "../GuiLayers/GuiLayFrame.h"

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
    GuiDlgMessageBox(GuiContainer *cntr);
    virtual ~GuiDlgMessageBox();

    static void ShowPopup(GuiContainer *cntr, GuiImage *image, int alpha, int delay,
                          GuiEffect *effa, GuiEffect *effb, const char *txt, ...);
    static MSGBTN ShowModal(GuiContainer *cntr, MSGT type, GuiImage *image, int alpha,
                            GuiEffect *effa, GuiEffect *effb, const char *txt, ...);

    void Create(MSGT type, GuiImage *image, int alpha, const char *txt, ...);
    void CreateVA(MSGT type, GuiImage *image, int alpha, const char *txt, va_list valist);

private:
    void CleanUp(void);

    MSGT btn_type;
    MSGBTN buttons[3];
    GuiContainer *container;
    GuiLayFrame *frame;
    GuiSprite *txt_sprite;
    GuiSprite *img_sprite;
    GuiElmButton *button[3];
    int no_buttons;
    int default_button;

    int DoSelection(void);
};

#endif


