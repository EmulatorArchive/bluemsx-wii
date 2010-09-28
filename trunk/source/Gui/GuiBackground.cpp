
#include "GuiBackground.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "version.h"
#include "../arch/archThread.h"

GuiBackground::GuiBackground(GuiManager *man)
{
    manager = man;
    sprBackground = NULL;
    sprTxt = NULL;
}

GuiBackground::~GuiBackground()
{
    Hide(10, 0);
}

void GuiBackground::Show(int fade)
{
    if( sprBackground == NULL ) {
        manager->Lock();

        // Background picture
        sprBackground = new Sprite;
        sprBackground->SetImage(g_imgBackground);
        sprBackground->SetStretchWidth((float)manager->GetWidth() /
                                       (float)g_imgBackground->GetWidth());
        sprBackground->SetStretchHeight((float)manager->GetHeight() /
                                        (float)g_imgBackground->GetHeight());
        sprBackground->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprBackground->SetRefPixelPosition(0, 0);
        sprBackground->SetPosition(0, 0);

        // Show it
        manager->AddBottom(sprBackground, fade);
        ShowVersion(fade);

        manager->Unlock();
    }
}

void GuiBackground::Hide(int fade, int delay)
{
    if( sprBackground != NULL ) {
        manager->Lock();
        HideVersion();
        manager->RemoveAndDelete(sprBackground, NULL, fade, delay);
        sprBackground = NULL;
        manager->Unlock();
    }
}

void GuiBackground::ShowVersion(int fade)
{
    GXColor white = {255,255,255,255};
    if( sprBackground != NULL && sprTxt == NULL ) {
        int txtwidth;
        int txtheight;

        manager->Lock();

        // Create image with version text
        imgTxt = new DrawableImage;
        imgTxt->SetFont(g_fontArial);
        imgTxt->SetSize(16);
        imgTxt->SetYSpacing(2);
        imgTxt->SetColor(white);
        imgTxt->GetTextSize(&txtwidth, &txtheight, VERSION_AS_STRING);
        txtwidth = (txtwidth + 3) & ~3;
        txtheight = (txtheight + 3) & ~3;
        imgTxt->CreateImage(txtwidth, txtheight);
        imgTxt->RenderText(true, VERSION_AS_STRING);

        // Version text sprite
        sprTxt = new Sprite;
        sprTxt->SetImage(imgTxt);
        sprTxt->SetPosition(530, 384);
        sprTxt->SetTransparency(192);

        // Show it
        manager->AddOnTopOf(sprBackground, sprTxt, fade);

        manager->Unlock();
    }
}

void GuiBackground::HideVersion(int fade, int delay)
{
    if( sprTxt != NULL ) {
        manager->Lock();
        manager->RemoveAndDelete(sprTxt, imgTxt, fade, delay);
        sprTxt = NULL;
        manager->Unlock();
    }
}

