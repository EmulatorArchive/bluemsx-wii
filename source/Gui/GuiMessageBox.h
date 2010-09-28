
#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include "GuiManager.h"
#include "GuiDialog.h"
#include "GuiButton.h"
#include "GuiContainer.h"
#include "DrawableImage.h"

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
    GuiMessageBox(GuiManager *man);
    virtual ~GuiMessageBox();

    void ShowPopup(const char *txt, Image *image = NULL, int alpha = 128);
    MSGBTN Show(const char *txt, Image *image = NULL, MSGT type = MSGT_TEXT, int alpha = 128);
    void Remove(void);
    void SetText(const char *fmt, ...);

    void MessageBoxPopupThread(void);
private:
    GuiRunner *runner;
    bool is_showing;
    GuiManager *manager;
    GuiContainer *container;
    Sprite *txt_sprite;
    Sprite *img_sprite;
    GuiButton *button[3];
    DrawableImage *txt_image;
    void *thread_popup;
    bool quit_thread;
    int no_buttons;
    int default_button;

    int DoSelection(void);
};

#endif


