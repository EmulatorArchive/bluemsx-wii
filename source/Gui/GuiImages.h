#ifndef _GUI_IMAGES_H
#define _GUI_IMAGES_H

#include "../wiisprite/wiisprite.h"

extern Image *g_imgArrow;
extern Image *g_imgBackground;
extern Image *g_imgMousecursor;
extern Image *g_imgNoise;
extern Image *g_imgSelector;
extern Image *g_imgSelector2;
extern Image *g_imgFloppyDisk;
extern Image *g_imgKeyboard;
extern Image *g_imgButtonRed;
extern Image *g_imgButtonGreen;
extern Image *g_imgButtonBlue;
extern Image *g_imgButtonYellow;
extern Image *g_imgCheckboxChecked;
extern Image *g_imgCheckbox;
extern Image *g_imgUp;
extern Image *g_imgDown;
extern Image *g_imgAdd;
extern Image *g_imgDelete;
extern Image *g_imgDelete2;
extern Image *g_imgSettings;

extern void GuiImageInit(void);
extern void GuiImageClose(void);

#endif
