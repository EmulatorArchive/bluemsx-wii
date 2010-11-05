
#include "GuiButton.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "GuiEffectFade.h"

#include "../WiiSprite/Sprite.h"

#define HIHGLIGHT_OVERLAP             6

#define BUTTON_TRANSPARENCY_NORMAL    160
#define BUTTON_TRANSPARENCY_HIGHLIGHT 255

GuiButton::GuiButton() : GuiElement()
{
    sprImage = NULL;
    sprText = NULL;
    sprSelector = NULL;
    selected = false;
    shown = false;
}

GuiButton::~GuiButton()
{
    CleanUp();
}

//-----------------------

bool GuiButton::ElmSetSelectedOnCollision(Sprite *sprite)
{
    if( shown && sprite != NULL && sprImage != NULL &&
        sprite->CollidesWith(sprImage, true) )
    {
        ElmSetSelected(true, 0, 0);
        return true;
    }else{
        return false;
    }
}

void GuiButton::ElmSetSelected(bool sel, int x, int y)
{
    if( !shown ) {
        return;
    }
    switch( type ) {
        case BTE_SELECTOR:
            if( !selected && sel && sprImage != NULL ) {
                sprSelector = new Sprite(g_imgSelector2, 0, 0);
                RegisterForDelete(sprSelector);
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

bool GuiButton::ElmGetRegion(int *px, int *py, int *pw, int *ph)
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

void GuiButton::CleanUp(void)
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

void GuiButton::CreateImageSelectorButton(Image *image, int f_sel)
{
    CleanUp();
    type = BTE_SELECTOR;
    SetWidth(image->GetWidth() + 2*HIHGLIGHT_OVERLAP);
    SetHeight(image->GetHeight() + 2*HIHGLIGHT_OVERLAP);
    fade_sel = f_sel;
    selected = false;
    sprImage = new Sprite(image, HIHGLIGHT_OVERLAP, HIHGLIGHT_OVERLAP);
    RegisterForDelete(sprImage);

    // Show
    AddTop(sprImage);
    shown = true;
}

void GuiButton::CreateImageHighlightButton(Image *image, int f_sel)
{
    CleanUp();
    type = BTE_HIGHLIGHT;
    SetWidth(image->GetWidth());
    SetHeight(image->GetHeight());
    fade_sel = f_sel;
    selected = false;
    sprImage = new Sprite(image, 0, 0);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
    RegisterForDelete(sprImage);

    // Show
    AddTop(sprImage);
    shown = true;
}

void GuiButton::CreateImageTextHighlightButton(Image *image, const char *txt, int f_sel)
{
    CleanUp();
    type = BTE_HIGHLIGHTTEXT;
    SetWidth(image->GetWidth());
    SetHeight(image->GetHeight());
    fade_sel = f_sel;
    selected = false;

    // Image
    sprImage = new Sprite(image, 0, 0);
    sprImage->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
    int imgwidth = sprImage->GetWidth();
    int imgheight = sprImage->GetHeight();
    RegisterForDelete(sprImage);

    // Text
    GXColor white = {255,255,255,255};
    sprText= new Sprite();
    sprText->CreateTextImage(g_fontImpact, 36, 0, 0, true, white, txt);
    sprText->SetPosition((imgwidth - (int)sprText->GetWidth()) / 2, (imgheight - (int)sprText->GetHeight()) / 2);
    sprText->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
    RegisterForDelete(sprText);

    // Show
    AddTop(sprImage);
    AddTop(sprText);
    shown = true;
}

