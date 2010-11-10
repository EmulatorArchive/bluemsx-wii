
#include "GuiElmBackground.h"

#include "version.h"
#include "../arch/archThread.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiSprite.h"

#include "../Gui/GuiFonts.h"
#include "../Gui/GuiImages.h"

GuiElmBackground::GuiElmBackground(GuiContainer *cntr)
              :GuiContainer(cntr)
{
    sprBackground = NULL;
    sprTxt = NULL;
    is_shown = false;
}

GuiElmBackground::~GuiElmBackground()
{
    Hide();
}

void GuiElmBackground::Show(GuiEffect *effect)
{
    if( !is_shown ) {
        // Background picture
        sprBackground = new GuiSprite;
        sprBackground->SetImage(g_imgBackground);
        sprBackground->SetStretchWidth((float)GetWidth() /
                                       (float)g_imgBackground->GetWidth());
        sprBackground->SetStretchHeight((float)GetHeight() /
                                        (float)g_imgBackground->GetHeight());
        sprBackground->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprBackground->SetRefPixelPosition(0, 0);
        sprBackground->SetPosition(0, 0);
        RegisterForDelete(sprBackground);
        AddTop(sprBackground, effect);

        // Show version
        ShowVersion(new GuiEffectFade(10));

        is_shown = true;
    }
}

void GuiElmBackground::Hide(GuiEffect *effect)
{
    HideVersion();
    if( is_shown ) {
        RemoveAndDelete(sprBackground, effect);
        sprBackground = NULL;

        is_shown = false;
    }
}

void GuiElmBackground::ShowVersion(GuiEffect *effect)
{
    GXColor white = {255,255,255,255};
    if( sprTxt == NULL ) {
        // Version text sprite
        sprTxt = new GuiSprite;
        sprTxt->CreateTextImage(g_fontArial, 16, 0, 2, true, white, VERSION_AS_STRING);
        sprTxt->SetPosition(530, 384);
        sprTxt->SetTransparency(192);
        RegisterForDelete(sprTxt);

        // Show it
        AddTop(sprTxt, effect);
    }
}

void GuiElmBackground::HideVersion(GuiEffect *effect)
{
    if( sprTxt != NULL ) {
        RemoveAndDelete(sprTxt, effect);
        sprTxt = NULL;
    }
}

