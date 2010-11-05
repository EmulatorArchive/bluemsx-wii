#ifndef _GUI_FRAME_H
#define _GUI_FRAME_H

#include "../wiisprite/wiisprite.h"

class GuiFrame : public TiledLayer
{
    public:
        GuiFrame(int posx, int posy, int width, int height, int alpha = 128);
        virtual ~GuiFrame();
        void Render(void);
        void SetPosition(int posx,  int posy);
        int GetWidth(void);
        int GetHeight(void);

    private:
        int cols, rows;
        Image image;
};

#endif
