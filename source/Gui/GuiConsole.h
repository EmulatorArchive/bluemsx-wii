#ifndef _GUI_CONSOLE_H
#define _GUI_CONSOLE_H

#include <wiisprite.h>
#include "GuiContainer.h"
#include "DrawableImage.h"

class GuiConsole {
	public:
		GuiConsole(wsp::LayerManager *manager, int posx, int posy, int width, int height);
		virtual ~GuiConsole();
		void SetVisible(bool state);
        bool IsVisible(void);
        void Render(void);
        void SetPosition( int posx,  int posy);

	private:
        bool _visible;
		int _width, _height, _imgwidth, _imgheight;
        u16 *_console_buffer;
        GuiContainer *_container;
        wsp::LayerManager *_manager;
		DrawableImage *_image;
		wsp::Sprite* _sprite;
};

#endif
