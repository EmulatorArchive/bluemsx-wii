
#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include "GuiContainer.h"
#include "GuiDialog.h"
#include "GuiButton.h"
#include "GuiFrame.h"
#include "../WiiSprite/Sprite.h"

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

class GuiMessageBox : public GuiDialog {
public:
    GuiMessageBox(GuiContainer *cntr);
    virtual ~GuiMessageBox();

    static void ShowPopup(GuiContainer *cntr, Image *image, int alpha, int delay,
                          GuiEffect *effa, GuiEffect *effb, const char *txt, ...);
    static MSGBTN ShowModal(GuiContainer *cntr, MSGT type, Image *image, int alpha,
                            GuiEffect *effa, GuiEffect *effb, const char *txt, ...);

    void Create(MSGT type, Image *image, int alpha, const char *txt, ...);
    void CreateVA(MSGT type, Image *image, int alpha, const char *txt, va_list valist);

private:
    void CleanUp(void);

    MSGT btn_type;
    MSGBTN buttons[3];
    GuiContainer *container;
    GuiFrame *frame;
    Sprite *txt_sprite;
    Sprite *img_sprite;
    GuiButton *button[3];
    int no_buttons;
    int default_button;

    int DoSelection(void);
};

#endif


