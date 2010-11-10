#ifndef _GUI_BACKGROUND_H
#define _GUI_BACKGROUND_H

#include "../GuiBase/GuiContainer.h"

class GuiEffect;
class GuiSprite;

class GuiElmBackground : public GuiContainer
{
    public:
        GuiElmBackground(GuiContainer *cntr);
        virtual ~GuiElmBackground();
        void Show(GuiEffect *effect = NULL);
        void Hide(GuiEffect *effect = NULL);
        void ShowVersion(GuiEffect *effect = NULL);
        void HideVersion(GuiEffect *effect = NULL);

    private:
        GuiSprite *sprTxt;
        GuiSprite *sprBackground;
        bool is_shown;
};

#endif
