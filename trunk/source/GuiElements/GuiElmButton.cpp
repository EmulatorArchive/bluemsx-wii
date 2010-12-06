
#include "GuiElmButton.h"

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiSprite.h"

#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"

#define HIHGLIGHT_OVERLAP             6

#define BUTTON_TRANSPARENCY_NORMAL    160
#define BUTTON_TRANSPARENCY_HIGHLIGHT 255

GuiElmButton::GuiElmButton(GuiContainer *parent, const char *name)
            : GuiElement(parent, name)
{
    sprImage = NULL;
    sprText = NULL;
    sprSelector = NULL;
    selected = false;
    shown = false;
}

GuiElmButton::~GuiElmButton()
{
    CleanUp();
}

//-----------------------

bool GuiElmButton::ElmSetSelectedOnCollision(GuiSprite *sprite)
{
    if( shown && sprite != NULL && sprImage != NULL &&
        sprite->CollidesWith(sprImage, true) == COLL_COLLISION )
    {
        ElmSetSelected(true, NULL, 0, 0);
        return true;
    }else{
        return false;
    }
}

void GuiElmButton::ElmSetSelected(bool sel, GuiSprite *pointer, int x, int y)
{
    if( !shown ) {
        return;
    }
    switch( type ) {
        case BTE_SELECTOR:
            if( !selected && sel && sprImage != NULL ) {
                sprSelector = new GuiSprite(this, "selector", g_imgSelector2, 0, 0);
                AddBehind(sprImage, sprSelector, new GuiEffectFade(fade_sel));
                selected = true;
            }
            if( selected && !sel ) {
                RemoveAndDelete(sprSelector, new GuiEffectFade(fade_sel));
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

bool GuiElmButton::ElmGetRegion(int *px, int *py, int *pw, int *ph)
{
    if( shown ) {
        *px = GetX();
        *py = GetY();
        *pw = GetWidth();
        *ph = GetHeight();
        return true;
    }else{
        return false;
    }
}

//-----------------------

void GuiElmButton::CleanUp(void)
{
    if( shown ) {
        if( sprText != NULL ) {
            RemoveAndDelete(sprText);
            sprText = NULL;
        }
        if( sprSelector != NULL ) {
            RemoveAndDelete(sprSelector);
            sprSelector = NULL;
        }
        if( sprImage != NULL ) {
            RemoveAndDelete(sprImage);
            sprImage = NULL;
        }
        shown = false;
    }
}

void GuiElmButton::CreateImageSelectorButton(GuiImage *image, int f_sel)
{
    CleanUp();
    type = BTE_SELECTOR;
    SetWidth(image->GetWidth() + 2*HIHGLIGHT_OVERLAP);
    SetHeight(image->GetHeight() + 2*HIHGLIGHT_OVERLAP);
    fade_sel = f_sel;
    selected = false;
    sprImage = new GuiSprite(this, "image", image, HIHGLIGHT_OVERLAP, HIHGLIGHT_OVERLAP);

    // Show
    AddTop(sprImage);
    shown = true;
}

void GuiElmButton::CreateImageHighlightButton(GuiImage *image, int f_sel)
{
    CleanUp();
    type = BTE_HIGHLIGHT;
    SetWidth(image->GetWidth());
    SetHeight(image->GetHeight());
    fade_sel = f_sel;
    selected = false;
    sprImage = new GuiSprite(this, "image", image, 0, 0);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);

    // Show
    AddTop(sprImage);
    shown = true;
}

void GuiElmButton::CreateImageTextHighlightButton(GuiImage *image, const char *txt, int f_sel)
{
    CleanUp();
    type = BTE_HIGHLIGHTTEXT;
    SetWidth(image->GetWidth());
    SetHeight(image->GetHeight());
    fade_sel = f_sel;
    selected = false;

    // GuiImage
    sprImage = new GuiSprite(this, "image", image, 0, 0);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
    int imgwidth = sprImage->GetWidth();
    int imgheight = sprImage->GetHeight();

    // Text
    GXColor white = {255,255,255,255};
    sprText= new GuiSprite(this, "text");
    sprText->CreateTextImage(g_fontImpact, 36, 0, 0, true, white, txt);
    sprText->SetPosition((imgwidth - (int)sprText->GetWidth()) / 2, (imgheight - (int)sprText->GetHeight()) / 2);
    sprText->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);

    // Show
    AddTop(sprImage);
    AddTop(sprText);
    shown = true;
}

