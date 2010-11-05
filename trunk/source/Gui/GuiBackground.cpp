
#include "GuiBackground.h"
#include "GuiEffectFade.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "version.h"
#include "../arch/archThread.h"

GuiBackground::GuiBackground(GuiContainer *cntr)
              :GuiContainer(cntr)
{
    sprBackground = NULL;
    sprTxt = NULL;
    is_shown = false;
}

GuiBackground::~GuiBackground()
{
    Hide();
}

void GuiBackground::Show(GuiEffect *effect)
{
    if( !is_shown ) {
        // Background picture
        sprBackground = new Sprite;
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

void GuiBackground::Hide(GuiEffect *effect)
{
    HideVersion();
    if( is_shown ) {
        RemoveAndDelete(sprBackground, effect);
        sprBackground = NULL;

        is_shown = false;
    }
}

void GuiBackground::ShowVersion(GuiEffect *effect)
{
    GXColor white = {255,255,255,255};
    if( sprTxt == NULL ) {
        // Version text sprite
        sprTxt = new Sprite;
        sprTxt->CreateTextImage(g_fontArial, 16, 0, 2, true, white, VERSION_AS_STRING);
        sprTxt->SetPosition(530, 384);
        sprTxt->SetTransparency(192);
        RegisterForDelete(sprTxt);

        // Show it
        AddTop(sprTxt, effect);
    }
}

void GuiBackground::HideVersion(GuiEffect *effect)
{
    if( sprTxt != NULL ) {
        RemoveAndDelete(sprTxt, effect);
        sprTxt = NULL;
    }
}

