#ifndef _GUI_BACKGROUND_H
#define _GUI_BACKGROUND_H

#include "GuiManager.h"

#define BACKGROUND_FADE_FRAMES 10

class GuiBackground
{
	public:
		GuiBackground(GuiManager *man);
		virtual ~GuiBackground();
        void Show(int fade = BACKGROUND_FADE_FRAMES);
        void Hide(int fade = BACKGROUND_FADE_FRAMES, int delay = 0);
        void ShowVersion(int fade = BACKGROUND_FADE_FRAMES);
        void HideVersion(int fade = BACKGROUND_FADE_FRAMES, int delay = 0);

	private:
        GuiManager *manager;
		DrawableImage *imgTxt;
        Sprite *sprTxt;
        Sprite *sprBackground;
        bool is_shown;
};

#endif
