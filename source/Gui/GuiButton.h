#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "GuiManager.h"

class GuiButton {
public:
    GuiButton(GuiManager *man);
    virtual ~GuiButton();

    void ShowImageButton(Image *image, int posx=0, int posy=0, int f_show=0, int f_sel=0);
    void Remove(int fade=0, int delay=0);
	bool Update(Sprite *cursor);

private:
	int posx, posy, fade_sel;
    GuiManager *manager;
    Sprite *sprImage;
	Sprite *sprSelector;
};

#endif
