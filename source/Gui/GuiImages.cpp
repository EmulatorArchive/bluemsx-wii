
#include "GuiImages.h"
#include "image_arrow.h"
#include "image_background.h"
#include "image_mousecursor.h"
#include "image_noise.h"
#include "image_selector.h"
#include "image_floppy_disk.h"

Image *g_imgArrow       = NULL;
Image *g_imgBackground  = NULL;
Image *g_imgMousecursor = NULL;
Image *g_imgNoise       = NULL;
Image *g_imgSelector    = NULL;
Image *g_imgFloppyDisk  = NULL;

void GuiImageInit(void)
{
    g_imgArrow = new Image;
    if(g_imgArrow->LoadImage(image_arrow) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgBackground = new Image;
    if(g_imgBackground->LoadImage(image_background) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgMousecursor = new Image;
    if(g_imgMousecursor->LoadImage(image_mousecursor) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgNoise = new Image;
    if(g_imgNoise->LoadImage(image_noise) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSelector = new Image;
    if(g_imgSelector->LoadImage(image_selector) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFloppyDisk = new Image;
    if(g_imgFloppyDisk->LoadImage(image_floppy_disk) != IMG_LOAD_ERROR_NONE) exit(0);
}

void GuiImageClose(void)
{
    delete g_imgFloppyDisk;
    g_imgFloppyDisk = NULL;
    delete g_imgSelector;
    g_imgSelector = NULL;
    delete g_imgNoise;
    g_imgNoise = NULL;
    delete g_imgMousecursor;
    g_imgMousecursor = NULL;
    delete g_imgBackground;
    g_imgBackground = NULL;
    delete g_imgArrow;
    g_imgArrow = NULL;
}

