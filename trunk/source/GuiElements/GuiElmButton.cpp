
#include "GuiElmButton.h"

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiRect.h"
#include "../GuiBase/GuiSprite.h"

#include "../Gui/GuiImages.h"
#include "../Gui/GuiFonts.h"

#define HIHGLIGHT_OVERLAP             6

#define BUTTON_TRANSPARENCY_NORMAL    0.6f
#define BUTTON_TRANSPARENCY_HIGHLIGHT 1.0f

GuiElmButton::GuiElmButton(GuiElement *parent, const char *name)
            : GuiElement(parent, name)
{
    sprImage = NULL;
    sprText = NULL;
    sprSelector = NULL;
    selected = false;
    is_created = false;
}

GuiElmButton::~GuiElmButton()
{
    CleanUp();
}

//-----------------------

bool GuiElmButton::OnTestActiveArea(float x, float y)
{
    if( is_created ) {
        return IsInVisibleArea(x, y);
    }else{
        return false;
    }
}

void GuiElmButton::OnFocus(bool focus)
{
    if( !is_created ) {
        return;
    }
    switch( type ) {
        case BTE_SELECTOR:
            if( !selected && focus && sprImage != NULL ) {
                sprSelector = new GuiSprite(this, "selector", g_imgSelector2, 0, 0);
                AddBehind(sprImage, sprSelector, GuiEffectFade(fade_sel));
                selected = true;
            }
            if( selected && !focus ) {
                RemoveAndDelete(sprSelector, GuiEffectFade(fade_sel));
                sprSelector = NULL;
                selected = false;
            }
            break;
        case BTE_HIGHLIGHT:
            if( !selected && focus && sprImage != NULL ) {
                sprImage->SetAlpha(BUTTON_TRANSPARENCY_HIGHLIGHT);
                selected = true;
            }
            if( selected && !focus && sprImage != NULL ) {
                sprImage->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);
                selected = false;
            }
            break;
        case BTE_HIGHLIGHTTEXT:
            if( !selected && focus && sprImage != NULL ) {
                sprImage->SetAlpha(BUTTON_TRANSPARENCY_HIGHLIGHT);
                sprText->SetAlpha(BUTTON_TRANSPARENCY_HIGHLIGHT);
                selected = true;
            }
            if( selected && !focus && sprImage != NULL ) {
                sprImage->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);
                sprText->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);
                selected = false;
            }
            break;
        default:
            break;
    }
}

//-----------------------

void GuiElmButton::CleanUp(void)
{
    if( is_created ) {
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
        is_created = false;
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
    is_created = true;
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
    sprImage->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);

    // Show
    AddTop(sprImage);
    is_created = true;
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
    sprImage->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);
    int imgwidth = (int)sprImage->GetWidth();
    int imgheight = (int)sprImage->GetHeight();

    // Text
    GXColor white = {255,255,255,255};
    sprText= new GuiSprite(this, "text");
    sprText->CreateTextImage(g_fontImpact, 36, 0, 0, true, white, txt);
    sprText->SetPosition((imgwidth - (int)sprText->GetWidth()) / 2, (imgheight - (int)sprText->GetHeight()) / 2);
    sprText->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);

    // Show
    AddTop(sprImage);
    AddTop(sprText);
    is_created = true;
}

