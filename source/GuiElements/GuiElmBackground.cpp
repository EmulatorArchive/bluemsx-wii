
#include "GuiElmBackground.h"

#include <assert.h>

#include "version.h"

#include "../GuiBase/GuiSprite.h"
#include "../Gui/GuiFonts.h"
#include "../Gui/GuiImages.h"

GuiElmBackground::GuiElmBackground(GuiContainer *parent, const char *name) :
                  GuiContainer(parent, name),
                  effectVersion(50)
{
    // Background picture
    sprBackground = new GuiSprite(this, "background");
    GuiImages::AssignSpriteToImage(sprBackground, "image_background");
    sprBackground->SetScaledWidth(GetWidth());
    sprBackground->SetScaledHeight(GetHeight());
    sprBackground->SetRefPixelPosition(0, 0);
    sprBackground->SetPosition(0, 0);
    AddTop(sprBackground);
    
    // Version
    GXColor white = {255,255,255,255};
    sprTxt = new GuiSprite(this, "version");
    sprTxt->CreateTextImage(g_fontArial, 16, 0, 2, true, white, VERSION_AS_STRING);
    sprTxt->SetPosition(530, 384);
    sprTxt->SetAlpha(0.75f);
    AddTop(sprTxt);
}

GuiElmBackground::~GuiElmBackground()
{
    RemoveAndDelete(sprBackground);
    sprBackground = NULL;
    RemoveAndDelete(sprTxt);
    sprTxt = NULL;
}

void GuiElmBackground::ShowVersion(void)
{
    assert( sprTxt );
    Show(sprTxt, effectVersion);
}

void GuiElmBackground::HideVersion(void)
{
    assert( sprTxt );
    Hide(sprTxt, effectVersion);
}

