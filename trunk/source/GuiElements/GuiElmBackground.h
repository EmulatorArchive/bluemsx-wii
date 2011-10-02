#ifndef _GUI_BACKGROUND_H
#define _GUI_BACKGROUND_H

#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiEffectFade.h"

class GuiEffect;
class GuiSprite;

class GuiElmBackground : public GuiContainer
{
    public:
        GuiElmBackground(GuiContainer *parent, const char *name);
        virtual ~GuiElmBackground();
        void ShowVersion(void);
        void HideVersion(void);

    private:
        GuiSprite *sprTxt;
        GuiSprite *sprBackground;
        GuiEffectFade effectVersion;
};

#endif
