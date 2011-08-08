
#include "GuiElmFrame.h"

#include "../Gui/GuiImages.h"
#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiSprite.h"

GuiElmFrame::GuiElmFrame(GuiContainer *parent, const char *name, FRAMETYPE type,
                         int posx, int posy, int width, int height, int alpha)
           : GuiContainer(parent, name)
{
    GuiImage *image;
    u32 tilesize;
    u32 offset;
    frame_type = type;
    switch( type ) {
        case FRAMETYPE_GRAY:
            image = g_imgFrameB;
            tilesize = 24;
            offset = 12;
            break;
        case FRAMETYPE_BRUSHED:
            image = g_imgFrameC;
            tilesize = 24;
            offset = 12;
            break;
        case FRAMETYPE_BLUE:
        default:
            image = g_imgFrameA;
            tilesize = 12;
            offset = 0;
            break;
    }

    int imgwidth = image->GetWidth();
    int imgheight = image->GetHeight();

    spr_left_top      = new GuiSprite(this, "left_top", image,        -(int)offset, -(int)offset,
                                      tilesize, tilesize,             0, 0);
    spr_middle_top    = new GuiSprite(this, "middle_top", image,      tilesize-offset, -(int)offset,
                                      imgwidth-2*tilesize, tilesize,  tilesize, 0);
    spr_right_top     = new GuiSprite(this, "right_top", image,       width-tilesize+offset, -(int)offset,
                                      tilesize, tilesize,             imgwidth-tilesize, 0);
    spr_left_bottom   = new GuiSprite(this, "left_bottom", image,     -(int)offset, height-tilesize+offset,
                                      tilesize, tilesize,             0, imgheight-tilesize);
    spr_middle_bottom = new GuiSprite(this, "middle_bottom", image,   tilesize-offset, height-tilesize+offset,
                                      imgwidth-2*tilesize, tilesize,  tilesize, imgheight-tilesize);
    spr_right_bottom  = new GuiSprite(this, "right_bottom", image,    width-tilesize+offset, height-tilesize+offset,
                                      tilesize, tilesize,             imgwidth-tilesize, imgheight-tilesize);
    spr_middle_left   = new GuiSprite(this, "middle_left", image,     -(int)offset, tilesize-offset,
                                      tilesize, imgheight-2*tilesize, 0, tilesize);
    spr_middle_right  = new GuiSprite(this, "middle_right", image,    width-tilesize+offset, tilesize-offset,
                                      tilesize, imgheight-2*tilesize, imgwidth-tilesize, tilesize);
    spr_middle        = new GuiSprite(this, "middle", image,          tilesize-offset, tilesize-offset,
                                      imgwidth-2*tilesize, imgheight-2*tilesize, tilesize, tilesize);

    spr_middle->SetStretchWidth((f32)(width-2*tilesize+2*offset)/(imgwidth-2*tilesize));
    spr_middle->SetStretchHeight((f32)(height-2*tilesize+2*offset)/(imgheight-2*tilesize));
    spr_middle_top->SetStretchWidth((f32)(width-2*tilesize+2*offset)/(imgwidth-2*tilesize));
    spr_middle_bottom->SetStretchWidth((f32)(width-2*tilesize+2*offset)/(imgwidth-2*tilesize));
    spr_middle_left->SetStretchHeight((f32)(height-2*tilesize+2*offset)/(imgheight-2*tilesize));
    spr_middle_right->SetStretchHeight((f32)(height-2*tilesize+2*offset)/(imgheight-2*tilesize));

    AddTop(spr_left_top);
    AddTop(spr_middle_top);
    AddTop(spr_right_top);
    AddTop(spr_left_bottom);
    AddTop(spr_middle_bottom);
    AddTop(spr_right_bottom);
    AddTop(spr_middle_left);
    AddTop(spr_middle_right);
    AddTop(spr_middle);

    SetTransparency(alpha);
    SetPosition(posx, posy);
    SetWidth(width);
    SetHeight(height);
}

GuiElmFrame::~GuiElmFrame()
{
    RemoveAndDelete(spr_left_top);
    RemoveAndDelete(spr_middle_top);
    RemoveAndDelete(spr_right_top);
    RemoveAndDelete(spr_left_bottom);
    RemoveAndDelete(spr_middle_bottom);
    RemoveAndDelete(spr_right_bottom);
    RemoveAndDelete(spr_middle_left);
    RemoveAndDelete(spr_middle_right);
    RemoveAndDelete(spr_middle);
}

