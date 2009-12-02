
#include "GuiButton.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "DrawableImage.h"

#define BUTTON_TRANSPARENCY_NORMAL    160
#define BUTTON_TRANSPARENCY_HIGHLIGHT 255

GuiButton::GuiButton(GuiManager *man)
{
    manager = man;
    imgText = NULL;
    sprImage = NULL;
    sprText = NULL;
    sprSelector = NULL;
    selected = false;
}

GuiButton::~GuiButton()
{
    Remove();
}

void GuiButton::ShowImageSelectorButton(Image *image, int x, int y, int f_show, int f_sel)
{
    effect = BTE_SELECTOR;
    posx = x;
    posy = y;
    fade_sel = f_sel;
    selected = false;
    sprImage = new Sprite(image, x, y);
    manager->Lock();
    manager->AddTop(sprImage, f_show);
    manager->Unlock();
}

void GuiButton::ShowImageHighlightButton(Image *image, int x, int y, int f_show, int f_sel)
{
    effect = BTE_HIGHLIGHT;
    posx = x;
    posy = y;
    fade_sel = f_sel;
    selected = false;
    sprImage = new Sprite(image, x, y);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
    manager->Lock();
    manager->AddTop(sprImage, f_show);
    manager->Unlock();
}

void GuiButton::ShowImageTextHighlightButton(Image *image, const char *txt, int x, int y, int f_show, int f_sel)
{
    effect = BTE_HIGHLIGHTTEXT;
    posx = x;
    posy = y;
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
    imgText->SetColor((GXColor){255,255,255,255});
    imgText->GetTextSize(&txtwidth, &txtheight, txt);
    txtwidth = (txtwidth + 3) & ~3;
    txtheight = (txtheight + 3) & ~3;
    imgText->CreateImage(txtwidth, txtheight);
    imgText->RenderText(true, txt);

    // Text image
    sprText= new Sprite(imgText, x + (imgwidth - txtwidth) / 2, y + (imgheight - (txtheight * 10)/9) / 2);
    sprText->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);

    // Add to screen
    manager->Lock();
    manager->AddTop(sprImage, f_show);
    manager->AddTop(sprText, f_show);
    manager->Unlock();
}

void GuiButton::SetSelected(bool sel)
{
    manager->Lock();

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
    
    manager->Unlock();
}

bool GuiButton::CollidesWith(Sprite *cursor)
{
    if( cursor != NULL && sprImage != NULL ) {
        return cursor->CollidesWith(sprImage, true);
    }
    return false;
}

bool GuiButton::Update(Sprite *cursor)
{
    if( cursor != NULL && sprImage != NULL ) {
        SetSelected(cursor->CollidesWith(sprImage, true));
    }else{
        SetSelected(false);
    }
    return selected;
}

void GuiButton::Remove(int fade, int delay)
{
    manager->Lock();

    manager->RemoveAndDelete(sprText, imgText, fade, delay);
    sprText = NULL;
    imgText = NULL;
    manager->RemoveAndDelete(sprSelector, NULL, fade, delay);
    sprSelector = NULL;
    manager->RemoveAndDelete(sprImage, NULL, fade, delay);
    sprImage = NULL;

    manager->Unlock();
}

