#ifndef _GUI_FRAME_H
#define _GUI_FRAME_H

#include "../GuiBase/GuiTiles.h"

class GuiContainer;

class GuiLayFrame : public GuiTiles
{
    public:
        GuiLayFrame(GuiContainer *parent, const char *name, int posx, int posy, int width, int height, int alpha = 128);
        virtual ~GuiLayFrame();
        void Render(void);
        void SetPosition(int posx,  int posy);
        int GetWidth(void);
        int GetHeight(void);

    private:
        int cols, rows;
        GuiImage image;
};

#endif
