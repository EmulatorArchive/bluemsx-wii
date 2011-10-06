#ifndef _GUI_ELM_FRAME_H
#define _GUI_ELM_FRAME_H

#include "../GuiBase/GuiContainer.h"

typedef enum {
  FRAMETYPE_BLUE,
  FRAMETYPE_GRAY,
  FRAMETYPE_BRUSHED,
  FRAMETYPE_SELECTOR,
} FRAME;

class GuiSprite;

class GuiElmFrame : public GuiContainer
{
    public:
        GuiElmFrame(GuiContainer *parent, const char *name, FRAME type, float posx, float posy, float width, float height, float alpha = 1.0f);
        virtual ~GuiElmFrame();

    private:
        GuiSprite *spr_left_top;
        GuiSprite *spr_middle_top;
        GuiSprite *spr_right_top;
        GuiSprite *spr_left_bottom;
        GuiSprite *spr_middle_bottom;
        GuiSprite *spr_right_bottom;
        GuiSprite *spr_middle_left;
        GuiSprite *spr_middle_right;
        GuiSprite *spr_middle;
};

#endif
