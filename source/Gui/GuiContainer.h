#ifndef _GUI_CONTAINER_H
#define _GUI_CONTAINER_H

#include <wiisprite.h>

class GuiContainer {
	public:
		GuiContainer(int posx, int posy, int width, int height, int alpha = 128);
		virtual ~GuiContainer();
		void SetVisible(bool state);
        void Render(void);
        void SetPosition( int posx,  int posy);
        wsp::Layer *GetLayer(void);

	private:
		int x, y, cols, rows;
		wsp::Image win_img;
		wsp::TiledLayer* win_layer;
};

#endif
