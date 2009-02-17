
#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include "GuiManager.h"
#include "GuiContainer.h"
#include "DrawableImage.h"

class GuiMessageBox {
public:
    GuiMessageBox(GuiManager *man);
    virtual ~GuiMessageBox();

    void ShowPopup(const char *txt, int txtwidth, Image *image = NULL, int alpha = 128);
    void Show(const char *txt, int txtwidth, Image *image = NULL, int alpha = 128);
    void Remove(void);

    void MessageBoxPopupThread(void);
private:
    bool is_showing;
    GuiManager *manager;
    GuiContainer *container;
    Sprite *txt_sprite;
    Sprite *img_sprite;
    DrawableImage *txt_image;
    void *thread_popup;
    bool quit_thread;
};

#endif


