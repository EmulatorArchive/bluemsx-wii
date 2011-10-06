
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
    shown = false;
}

GuiElmButton::~GuiElmButton()
{
    CleanUp();
}

//-----------------------

bool GuiElmButton::OnTestActiveArea(float x, float y)
{
    if( shown ) {
        return IsInVisibleArea(x, y);
    }else{
        return false;
    }
}

void GuiElmButton::OnFocus(bool focus)
{
    if( !shown ) {
        return;
    }
    switch( type ) {
        case BTE_SELECTOR:
            if( !selected && focus && sprImage != NULL ) {
                sprSelector = new GuiSprite(this, "selector");
                GuiImages::AssignSpriteToImage(sprSelector, "image_selector2");
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

void GuiElmButton::CreateImageSelectorButton(const char *name, int f_sel)
{
    CleanUp();
    type = BTE_SELECTOR;
    fade_sel = f_sel;
    selected = false;
    sprImage = new GuiSprite(this, "image", NULL, HIHGLIGHT_OVERLAP, HIHGLIGHT_OVERLAP);
    GuiImages::AssignSpriteToImage(sprImage, name);
    SetWidth(sprImage->GetWidth() + 2*HIHGLIGHT_OVERLAP);
    SetHeight(sprImage->GetHeight() + 2*HIHGLIGHT_OVERLAP);

    // Show
    AddTop(sprImage);
    shown = true;
}

void GuiElmButton::CreateImageHighlightButton(const char *name, int f_sel)
{
    CleanUp();
    type = BTE_HIGHLIGHT;
    fade_sel = f_sel;
    selected = false;
    sprImage = new GuiSprite(this, "image", NULL, 0, 0);
    GuiImages::AssignSpriteToImage(sprImage, name);
    SetWidth(sprImage->GetWidth());
    SetHeight(sprImage->GetHeight());
    sprImage->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);

    // Show
    AddTop(sprImage);
    shown = true;
}

void GuiElmButton::CreateImageTextHighlightButton(const char *name, const char *txt, int f_sel)
{
    CleanUp();
    type = BTE_HIGHLIGHTTEXT;
    fade_sel = f_sel;
    selected = false;

    // GuiImage
    sprImage = new GuiSprite(this, "image", NULL, 0, 0);
    GuiImages::AssignSpriteToImage(sprImage, name);
    SetWidth(sprImage->GetWidth());
    SetHeight(sprImage->GetHeight());
    sprImage->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);
    float imgwidth = sprImage->GetWidth();
    float imgheight = sprImage->GetHeight();

    // Text
    GXColor white = {255,255,255,255};
    sprText= new GuiSprite(this, "text");
    sprText->CreateTextImage(g_fontImpact, 36, 0, 0, true, white, txt);
    sprText->SetPosition((imgwidth - sprText->GetWidth()) / 2, (imgheight - sprText->GetHeight()) / 2);
    sprText->SetAlpha(BUTTON_TRANSPARENCY_NORMAL);

    // Show
    AddTop(sprImage);
    AddTop(sprText);
    shown = true;
}

