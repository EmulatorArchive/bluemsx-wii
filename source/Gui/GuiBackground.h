#ifndef _GUI_BACKGROUND_H
#define _GUI_BACKGROUND_H

#include "GuiContainer.h"

class GuiEffect;

class GuiBackground : public GuiContainer
{
    public:
        GuiBackground(GuiContainer *cntr);
        virtual ~GuiBackground();
        void Show(GuiEffect *effect = NULL);
        void Hide(GuiEffect *effect = NULL);
        void ShowVersion(GuiEffect *effect = NULL);
        void HideVersion(GuiEffect *effect = NULL);

    private:
        Sprite *sprTxt;
        Sprite *sprBackground;
        bool is_shown;
};

#endif
