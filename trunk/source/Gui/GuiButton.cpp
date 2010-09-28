
#include "GuiButton.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "DrawableImage.h"

#define BUTTON_TRANSPARENCY_NORMAL    160
#define BUTTON_TRANSPARENCY_HIGHLIGHT 255

GuiButton::GuiButton(GuiManager *man) : GuiElement()
{
    manager = man;
    imgText = NULL;
    sprImage = NULL;
    sprText = NULL;
    sprSelector = NULL;
    selected = false;
    shown = false;
}

GuiButton::~GuiButton()
{
    ElmRemoveLayers(manager, false, 0, 0);
}

//-----------------------

void GuiButton::ElmAddLayers(GuiManager *manager, int index, bool fix, int fade, int delay)
{
    switch( effect ) {
        case BTE_SELECTOR:
        case BTE_HIGHLIGHT:
            manager->AddTop(sprImage, fade, delay);
            shown = true;
            break;
        case BTE_HIGHLIGHTTEXT:
            manager->AddTop(sprImage, fade, delay);
            manager->AddTop(sprText, fade, delay);
            shown = true;
            break;
        default:
            break;
    }
}

void GuiButton::ElmRemoveLayers(GuiManager *manager, bool del, int fade, int delay)
{
    if( shown ) {
        manager->RemoveAndDelete(sprText, imgText, fade, delay);
        sprText = NULL;
        imgText = NULL;
        manager->RemoveAndDelete(sprSelector, NULL, fade, delay);
        sprSelector = NULL;
        manager->RemoveAndDelete(sprImage, NULL, fade, delay);
        sprImage = NULL;
        shown = false;
    }
}

Layer* GuiButton::ElmGetTopLayer(void)
{
    return sprImage;
}

Layer* GuiButton::ElmGetBottomLayer(void)
{
    return sprText? sprText : sprImage;
}

//-----------------------

bool GuiButton::ElmSetSelectedOnCollision(GuiRunner *runner, Sprite *sprite)
{
    if( shown && sprite != NULL && sprImage != NULL &&
        sprite->CollidesWith(sprImage, true) )
    {
        ElmSetSelected(runner, true, 0, 0);
        return true;
    }else{
        return false;
    }
}

void GuiButton::ElmSetSelected(GuiRunner *runner, bool sel, int x, int y)
{
    if( !shown ) {
        return;
    }
    switch( effect ) {
        case BTE_SELECTOR:
            if( !selected && sel && sprImage != NULL ) {
                sprSelector = new Sprite(g_imgSelector2, posx-6, posy-6);
                manager->AddBehind(sprImage, sprSelector, fade_sel);
                selected = true;
            }
            if( selected && !sel ) {
                manager->RemoveAndDelete(sprSelector, NULL, fade_sel);
                sprSelector = NULL;
                selected = false;
            }
            break;
        case BTE_HIGHLIGHT:
            if( !selected && sel && sprImage != NULL ) {
                sprImage->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
                selected = true;
            }
            if( selected && !sel && sprImage != NULL ) {
                sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
                selected = false;
            }
            break;
        case BTE_HIGHLIGHTTEXT:
            if( !selected && sel && sprImage != NULL ) {
                sprImage->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
                sprText->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
                selected = true;
            }
            if( selected && !sel && sprImage != NULL ) {
                sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
                sprText->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
                selected = false;
            }
            break;
        default:
            break;
    }
}

bool GuiButton::ElmGetRegion(GuiRunner *runner, int *px, int *py, int *pw, int *ph)
{
    if( shown ) {
        *px = posx;
        *py = posy;
        *pw = width;
        *ph = height;
        return true;
    }else{
        return false;
    }
}

//-----------------------

void GuiButton::CreateImageSelectorButton(Image *image, int x, int y, int f_sel)
{
    ElmRemoveLayers(manager, false, 0, 0);
    effect = BTE_SELECTOR;
    posx = x;
    posy = y;
    width = image->GetWidth();
    height = image->GetHeight();
    fade_sel = f_sel;
    selected = false;
    sprImage = new Sprite(image, x, y);
}

void GuiButton::CreateImageHighlightButton(Image *image, int x, int y, int f_sel)
{
    ElmRemoveLayers(manager, false, 0, 0);
    effect = BTE_HIGHLIGHT;
    posx = x;
    posy = y;
    width = image->GetWidth();
    height = image->GetHeight();
    fade_sel = f_sel;
    selected = false;
    sprImage = new Sprite(image, x, y);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
}

void GuiButton::CreateImageTextHighlightButton(Image *image, const char *txt, int x, int y, int f_sel)
{
    ElmRemoveLayers(manager, false, 0, 0);
    effect = BTE_HIGHLIGHTTEXT;
    posx = x;
    posy = y;
    width = image->GetWidth();
    height = image->GetHeight();
    fade_sel = f_sel;
    selected = false;

    // Image
    sprImage = new Sprite(image, x, y);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
    int imgwidth = sprImage->GetWidth();
    int imgheight = sprImage->GetHeight();

    // Prepare text
    int txtwidth;
    int txtheight;
    imgText = new DrawableImage;
    imgText->SetFont(g_fontImpact);
    imgText->SetSize(36);
    GXColor white = {255,255,255,255};
    imgText->SetColor(white);
    imgText->GetTextSize(&txtwidth, &txtheight, txt);
    txtwidth = (txtwidth + 3) & ~3;
    txtheight = (txtheight + 3) & ~3;
    imgText->CreateImage(txtwidth, txtheight);
    imgText->RenderText(true, txt);

    // Text image
    sprText= new Sprite(imgText, x + (imgwidth - txtwidth) / 2, y + (imgheight - (txtheight * 10)/9) / 2);
    sprText->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
}

