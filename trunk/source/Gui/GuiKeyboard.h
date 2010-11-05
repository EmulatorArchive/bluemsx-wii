
#ifndef _GUI_KEYBOARD_H
#define _GUI_KEYBOARD_H

class GuiContainer;
class GuiRootContainer;

class GuiKeyboard {
public:
    GuiKeyboard(GuiRootContainer *man, GuiContainer *cntr);
    virtual ~GuiKeyboard();

    void Show(void);
    void Hide(void);
    bool IsShowing(void);
    void SetEnabled(bool enable);
    void Render(void);

private:
    static bool RenderWrapper(void *arg);

    bool is_showing;
    bool is_enabled;
    bool is_hidden;
    int xpos;
    int ypos;
    int xsize;
    int ysize;
    float xscale;
    float yscale;
    int keymap1;
    int keymap2;

    GuiContainer *container;
    GuiRootContainer *manager;
    Sprite *spr_image;
    Sprite *spr_cursor;
    Sprite *spr_hover;
    Sprite *spr_pressed;
};

#endif


