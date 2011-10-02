
#include "GuiElmFrame.h"

#include "../Gui/GuiImages.h"
#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiSprite.h"

GuiElmFrame::GuiElmFrame(GuiContainer *parent, const char *name, FRAMETYPE type,
                         float posx, float posy, float width, float height, float alpha)
           : GuiContainer(parent, name)
{
    GuiImage *image;
    float tilesize;
    float offset;
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

    float imgwidth = image->GetWidth();
    float imgheight = image->GetHeight();

    spr_left_top      = new GuiSprite(this, "left_top", image,        -offset, -offset,
                                      tilesize, tilesize,             0, 0);
    spr_middle_top    = new GuiSprite(this, "middle_top", image,      tilesize-offset, -offset,
                                      imgwidth-2*tilesize, tilesize,  tilesize, 0);
    spr_right_top     = new GuiSprite(this, "right_top", image,       width-tilesize+offset, -offset,
                                      tilesize, tilesize,             imgwidth-tilesize, 0);
    spr_left_bottom   = new GuiSprite(this, "left_bottom", image,     -offset, height-tilesize+offset,
                                      tilesize, tilesize,             0, imgheight-tilesize);
    spr_middle_bottom = new GuiSprite(this, "middle_bottom", image,   tilesize-offset, height-tilesize+offset,
                                      imgwidth-2*tilesize, tilesize,  tilesize, imgheight-tilesize);
    spr_right_bottom  = new GuiSprite(this, "right_bottom", image,    width-tilesize+offset, height-tilesize+offset,
                                      tilesize, tilesize,             imgwidth-tilesize, imgheight-tilesize);
    spr_middle_left   = new GuiSprite(this, "middle_left", image,     -offset, tilesize-offset,
                                      tilesize, imgheight-2*tilesize, 0, tilesize);
    spr_middle_right  = new GuiSprite(this, "middle_right", image,    width-tilesize+offset, tilesize-offset,
                                      tilesize, imgheight-2*tilesize, imgwidth-tilesize, tilesize);
    spr_middle        = new GuiSprite(this, "middle", image,          tilesize-offset, tilesize-offset,
                                      imgwidth-2*tilesize, imgheight-2*tilesize, tilesize, tilesize);

    spr_middle->SetZoomX((width-2*tilesize+2*offset)/(imgwidth-2*tilesize));
    spr_middle->SetZoomY((height-2*tilesize+2*offset)/(imgheight-2*tilesize));
    spr_middle_top->SetZoomX((width-2*tilesize+2*offset)/(imgwidth-2*tilesize));
    spr_middle_bottom->SetZoomX((width-2*tilesize+2*offset)/(imgwidth-2*tilesize));
    spr_middle_left->SetZoomY((height-2*tilesize+2*offset)/(imgheight-2*tilesize));
    spr_middle_right->SetZoomY((height-2*tilesize+2*offset)/(imgheight-2*tilesize));

    AddTop(spr_left_top);
    AddTop(spr_middle_top);
    AddTop(spr_right_top);
    AddTop(spr_left_bottom);
    AddTop(spr_middle_bottom);
    AddTop(spr_right_bottom);
    AddTop(spr_middle_left);
    AddTop(spr_middle_right);
    AddTop(spr_middle);

    SetAlpha(alpha);
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

