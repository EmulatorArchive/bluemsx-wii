
#include "GuiImages.h"

#include "image_arrow.inc"
#include "image_background.inc"
#include "image_mousecursor.inc"
#include "image_noise.inc"
#include "image_selector.inc"
#include "image_selector2.inc"
#include "image_floppy_disk.inc"
#include "image_keyboard.inc"
#include "image_keyboard2.inc"
#include "image_button_red.inc"
#include "image_button_green.inc"
#include "image_button_blue.inc"
#include "image_button_yellow.inc"
#include "image_checkbox_checked.inc"
#include "image_checkbox.inc"
#include "image_up.inc"
#include "image_down.inc"
#include "image_add.inc"
#include "image_delete.inc"
#include "image_delete2.inc"
#include "image_settings.inc"
#include "image_frame_a.inc"
#include "image_frame_b.inc"
#include "image_frame_c.inc"
#include "image_inputbar.inc"

#include "../GuiBase/GuiImage.h"

GuiImage *g_imgArrow           = NULL;
GuiImage *g_imgBackground      = NULL;
GuiImage *g_imgMousecursor     = NULL;
GuiImage *g_imgNoise           = NULL;
GuiImage *g_imgSelector        = NULL;
GuiImage *g_imgSelector2       = NULL;
GuiImage *g_imgFloppyDisk      = NULL;
GuiImage *g_imgKeyboard        = NULL;
GuiImage *g_imgKeyboard2       = NULL;
GuiImage *g_imgButtonRed       = NULL;
GuiImage *g_imgButtonGreen     = NULL;
GuiImage *g_imgButtonBlue      = NULL;
GuiImage *g_imgButtonYellow    = NULL;
GuiImage *g_imgCheckboxChecked = NULL;
GuiImage *g_imgCheckbox        = NULL;
GuiImage *g_imgUp              = NULL;
GuiImage *g_imgDown            = NULL;
GuiImage *g_imgAdd             = NULL;
GuiImage *g_imgDelete          = NULL;
GuiImage *g_imgDelete2         = NULL;
GuiImage *g_imgSettings        = NULL;
GuiImage *g_imgFrameA          = NULL;
GuiImage *g_imgFrameB          = NULL;
GuiImage *g_imgFrameC          = NULL;
GuiImage *g_imgInputBar        = NULL;


void GuiImageInit(void)
{
    g_imgArrow = new GuiImage;
    if(g_imgArrow->LoadImage(image_arrow) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgBackground = new GuiImage;
    if(g_imgBackground->LoadImage(image_background) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgMousecursor = new GuiImage;
    if(g_imgMousecursor->LoadImage(image_mousecursor) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgNoise = new GuiImage;
    if(g_imgNoise->LoadImage(image_noise) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSelector = new GuiImage;
    if(g_imgSelector->LoadImage(image_selector) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSelector2 = new GuiImage;
    if(g_imgSelector2->LoadImage(image_selector2) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFloppyDisk = new GuiImage;
    if(g_imgFloppyDisk->LoadImage(image_floppy_disk) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgKeyboard = new GuiImage;
    if(g_imgKeyboard->LoadImage(image_keyboard) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgKeyboard2 = new GuiImage;
    if(g_imgKeyboard2->LoadImage(image_keyboard2) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonRed = new GuiImage;
    if(g_imgButtonRed->LoadImage(image_button_red) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonGreen = new GuiImage;
    if(g_imgButtonGreen->LoadImage(image_button_green) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonBlue = new GuiImage;
    if(g_imgButtonBlue->LoadImage(image_button_blue) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonYellow = new GuiImage;
    if(g_imgButtonYellow->LoadImage(image_button_yellow) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgCheckboxChecked = new GuiImage;
    if(g_imgCheckboxChecked->LoadImage(image_checkbox_checked) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgCheckbox = new GuiImage;
    if(g_imgCheckbox->LoadImage(image_checkbox) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgUp = new GuiImage;
    if(g_imgUp->LoadImage(image_up) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgDown = new GuiImage;
    if(g_imgDown->LoadImage(image_down) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgAdd = new GuiImage;
    if(g_imgAdd->LoadImage(image_add) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgDelete = new GuiImage;
    if(g_imgDelete->LoadImage(image_delete) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgDelete2 = new GuiImage;
    if(g_imgDelete2->LoadImage(image_delete2) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSettings = new GuiImage;
    if(g_imgSettings->LoadImage(image_settings) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFrameA = new GuiImage;
    if(g_imgFrameA->LoadImage(image_frame_a) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFrameB = new GuiImage;
    if(g_imgFrameB->LoadImage(image_frame_b) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFrameC = new GuiImage;
    if(g_imgFrameC->LoadImage(image_frame_c) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgInputBar = new GuiImage;
    if(g_imgInputBar->LoadImage(image_inputbar) != IMG_LOAD_ERROR_NONE) exit(0);
}

void GuiImageClose(void)
{
    delete g_imgButtonRed;
    g_imgButtonRed = NULL;
    delete g_imgButtonGreen;
    g_imgButtonGreen = NULL;
    delete g_imgButtonBlue;
    g_imgButtonBlue = NULL;
    delete g_imgButtonYellow;
    g_imgButtonYellow = NULL;
    delete g_imgKeyboard;
    g_imgKeyboard = NULL;
    delete g_imgKeyboard2;
    g_imgKeyboard2 = NULL;
    delete g_imgFloppyDisk;
    g_imgFloppyDisk = NULL;
    delete g_imgSelector;
    g_imgSelector = NULL;
    delete g_imgSelector2;
    g_imgSelector2 = NULL;
    delete g_imgNoise;
    g_imgNoise = NULL;
    delete g_imgMousecursor;
    g_imgMousecursor = NULL;
    delete g_imgBackground;
    g_imgBackground = NULL;
    delete g_imgArrow;
    g_imgArrow = NULL;
    delete g_imgCheckboxChecked;
    g_imgCheckboxChecked = NULL;
    delete g_imgCheckbox;
    g_imgCheckbox = NULL;
    delete g_imgUp;
    g_imgUp = NULL;
    delete g_imgDown;
    g_imgDown = NULL;
    delete g_imgAdd;
    g_imgAdd = NULL;
    delete g_imgDelete;
    g_imgDelete = NULL;
    delete g_imgDelete2;
    g_imgDelete2 = NULL;
    delete g_imgSettings;
    g_imgSettings = NULL;
    delete g_imgFrameA;
    g_imgFrameA = NULL;
    delete g_imgFrameB;
    g_imgFrameB = NULL;
    delete g_imgFrameC;
    g_imgFrameC = NULL;
    delete g_imgInputBar;
    g_imgInputBar = NULL;
}

