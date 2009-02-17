
#ifndef _GUI_KEYBOARD_H
#define _GUI_KEYBOARD_H

#include "GuiManager.h"
#include "DrawableImage.h"

class GuiKeyboard {
public:
    GuiKeyboard(GuiManager *man);
    virtual ~GuiKeyboard();

    void Show(void);
    void Remove(void);
    bool IsShowing(void);
    void Render(void);

private:
    static void RenderWrapper(void *arg);

    bool is_showing;
    int xpos;
    int ypos;
    float xscale;
    float yscale;
    int keymap1;
    int keymap2;

    GuiManager *manager;
    DrawableImage *img_hover;
    DrawableImage *img_pressed;
    Sprite *spr_image;
    Sprite *spr_cursor;
    Sprite *spr_hover;
};

#endif


