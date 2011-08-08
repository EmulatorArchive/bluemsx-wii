
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

#include "../GuiBase/GuiRootContainer.h"
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


void GuiImageInit(GuiRootContainer *root)
{
    g_imgArrow = root->CreateImage();
    if(g_imgArrow->LoadImage(image_arrow) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgBackground = root->CreateImage();
    if(g_imgBackground->LoadImage(image_background) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgMousecursor = root->CreateImage();
    if(g_imgMousecursor->LoadImage(image_mousecursor) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgNoise = root->CreateImage();
    if(g_imgNoise->LoadImage(image_noise) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSelector = root->CreateImage();
    if(g_imgSelector->LoadImage(image_selector) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSelector2 = root->CreateImage();
    if(g_imgSelector2->LoadImage(image_selector2) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFloppyDisk = root->CreateImage();
    if(g_imgFloppyDisk->LoadImage(image_floppy_disk) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgKeyboard = root->CreateImage();
    if(g_imgKeyboard->LoadImage(image_keyboard) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgKeyboard2 = root->CreateImage();
    if(g_imgKeyboard2->LoadImage(image_keyboard2) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonRed = root->CreateImage();
    if(g_imgButtonRed->LoadImage(image_button_red) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonGreen = root->CreateImage();
    if(g_imgButtonGreen->LoadImage(image_button_green) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonBlue = root->CreateImage();
    if(g_imgButtonBlue->LoadImage(image_button_blue) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonYellow = root->CreateImage();
    if(g_imgButtonYellow->LoadImage(image_button_yellow) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgCheckboxChecked = root->CreateImage();
    if(g_imgCheckboxChecked->LoadImage(image_checkbox_checked) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgCheckbox = root->CreateImage();
    if(g_imgCheckbox->LoadImage(image_checkbox) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgUp = root->CreateImage();
    if(g_imgUp->LoadImage(image_up) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgDown = root->CreateImage();
    if(g_imgDown->LoadImage(image_down) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgAdd = root->CreateImage();
    if(g_imgAdd->LoadImage(image_add) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgDelete = root->CreateImage();
    if(g_imgDelete->LoadImage(image_delete) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgDelete2 = root->CreateImage();
    if(g_imgDelete2->LoadImage(image_delete2) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSettings = root->CreateImage();
    if(g_imgSettings->LoadImage(image_settings) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFrameA = root->CreateImage();
    if(g_imgFrameA->LoadImage(image_frame_a) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFrameB = root->CreateImage();
    if(g_imgFrameB->LoadImage(image_frame_b) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFrameC = root->CreateImage();
    if(g_imgFrameC->LoadImage(image_frame_c) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgInputBar = root->CreateImage();
    if(g_imgInputBar->LoadImage(image_inputbar) != IMG_LOAD_ERROR_NONE) exit(0);
}

void GuiImageClose(GuiRootContainer *root)
{
    root->ReleaseImage(g_imgButtonRed);
    g_imgButtonRed = NULL;
    root->ReleaseImage(g_imgButtonGreen);
    g_imgButtonGreen = NULL;
    root->ReleaseImage(g_imgButtonBlue);
    g_imgButtonBlue = NULL;
    root->ReleaseImage(g_imgButtonYellow);
    g_imgButtonYellow = NULL;
    root->ReleaseImage(g_imgKeyboard);
    g_imgKeyboard = NULL;
    root->ReleaseImage(g_imgKeyboard2);
    g_imgKeyboard2 = NULL;
    root->ReleaseImage(g_imgFloppyDisk);
    g_imgFloppyDisk = NULL;
    root->ReleaseImage(g_imgSelector);
    g_imgSelector = NULL;
    root->ReleaseImage(g_imgSelector2);
    g_imgSelector2 = NULL;
    root->ReleaseImage(g_imgNoise);
    g_imgNoise = NULL;
    root->ReleaseImage(g_imgMousecursor);
    g_imgMousecursor = NULL;
    root->ReleaseImage(g_imgBackground);
    g_imgBackground = NULL;
    root->ReleaseImage(g_imgArrow);
    g_imgArrow = NULL;
    root->ReleaseImage(g_imgCheckboxChecked);
    g_imgCheckboxChecked = NULL;
    root->ReleaseImage(g_imgCheckbox);
    g_imgCheckbox = NULL;
    root->ReleaseImage(g_imgUp);
    g_imgUp = NULL;
    root->ReleaseImage(g_imgDown);
    g_imgDown = NULL;
    root->ReleaseImage(g_imgAdd);
    g_imgAdd = NULL;
    root->ReleaseImage(g_imgDelete);
    g_imgDelete = NULL;
    root->ReleaseImage(g_imgDelete2);
    g_imgDelete2 = NULL;
    root->ReleaseImage(g_imgSettings);
    g_imgSettings = NULL;
    root->ReleaseImage(g_imgFrameA);
    g_imgFrameA = NULL;
    root->ReleaseImage(g_imgFrameB);
    g_imgFrameB = NULL;
    root->ReleaseImage(g_imgFrameC);
    g_imgFrameC = NULL;
    root->ReleaseImage(g_imgInputBar);
    g_imgInputBar = NULL;
}

