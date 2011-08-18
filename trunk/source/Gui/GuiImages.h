#ifndef _GUI_IMAGES_H
#define _GUI_IMAGES_H

class GuiImage;
class GuiRootContainer;

extern GuiImage *g_imgArrow;
extern GuiImage *g_imgBackground;
extern GuiImage *g_imgMousecursor;
extern GuiImage *g_imgNoise;
extern GuiImage *g_imgSelector;
extern GuiImage *g_imgSelector2;
extern GuiImage *g_imgFloppyDisk;
extern GuiImage *g_imgKeyboard;
extern GuiImage *g_imgKeyboard2;
extern GuiImage *g_imgButtonRed;
extern GuiImage *g_imgButtonGreen;
extern GuiImage *g_imgButtonBlue;
extern GuiImage *g_imgButtonYellow;
extern GuiImage *g_imgCheckboxChecked;
extern GuiImage *g_imgCheckbox;
extern GuiImage *g_imgUp;
extern GuiImage *g_imgDown;
extern GuiImage *g_imgAdd;
extern GuiImage *g_imgDelete;
extern GuiImage *g_imgDelete2;
extern GuiImage *g_imgSettings;
extern GuiImage *g_imgFrameA;
extern GuiImage *g_imgFrameB;
extern GuiImage *g_imgFrameC;
extern GuiImage *g_imgInputBar;

extern void GuiImageInit(void);
extern void GuiImageClose(GuiRootContainer *root);

#endif
