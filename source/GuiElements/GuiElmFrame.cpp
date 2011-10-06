
#include <assert.h>
#include "GuiElmFrame.h"

#include "../Gui/GuiImages.h"
#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiRect.h"
#include "../GuiBase/GuiSprite.h"


typedef struct {
  FRAME frame;
  const char *image;
  float tilesize_L;
  float tilesize_T;
  float tilesize_R;
  float tilesize_B;
  float offset_L;
  float offset_T;
  float offset_R;
  float offset_B;
} FRAMEINFO;

FRAMEINFO frameinfo[] = { //              .png image name                     L   T   R   B    L  T  R  B
{ FRAMETYPE_BLUE,                         "image_frame_a",                    12, 12, 12, 12,  0, 0, 0, 0 },
{ FRAMETYPE_GRAY,                         "image_frame_b",                    24, 24, 24, 24,  12, 12, 12, 12 },
{ FRAMETYPE_BRUSHED,                      "image_frame_c",                    24, 24, 24, 24,  12, 12, 12, 12 },
{ FRAMETYPE_SELECTOR,                     "image_selector",                   1,  0,  1,  0,   1, 0, 1, 0 },
};

GuiElmFrame::GuiElmFrame(GuiContainer *parent, const char *name, FRAME type,
                         float posx, float posy, float width, float height, float alpha)
           : GuiContainer(parent, name)
{
    FRAMEINFO *info;

    // Find frame type info
    info = NULL;
    for(int i = 0; i < sizeof(frameinfo)/sizeof(frameinfo[0]); i++) {
        if( frameinfo[i].frame == type ) {
            info = &frameinfo[i];
            break;
        }
    }
    assert( info != NULL );

    spr_left_top = NULL;
    spr_middle_top = NULL;
    spr_right_top = NULL;
    spr_left_bottom = NULL;
    spr_middle_bottom = NULL;
    spr_right_bottom = NULL;
    spr_middle_left = NULL;
    spr_middle_right = NULL;
    spr_middle = NULL;

    float imgwidth = GuiImages::GetWidth(info->image);
    float imgheight = GuiImages::GetHeight(info->image);

    if( info->tilesize_L > 1 && info->tilesize_T > 1 ) {
        spr_left_top      = new GuiSprite(this, "left_top", NULL,      -info->offset_L, -info->offset_T);
        GuiImages::AssignSpriteToImage(spr_left_top, info->image,      GuiRect(0, 0, info->tilesize_L, info->tilesize_T));
    }
    if( info->tilesize_T > 1 ) {
        spr_middle_top    = new GuiSprite(this, "middle_top", NULL,    info->tilesize_L-info->offset_L, -info->offset_T);
        GuiImages::AssignSpriteToImage(spr_middle_top, info->image,    GuiRect(info->tilesize_L, 0, imgwidth-info->tilesize_L-info->tilesize_R, info->tilesize_T));
    }
    if( info->tilesize_R > 1 && info->tilesize_T > 1 ) {
        spr_right_top     = new GuiSprite(this, "right_top", NULL,     width-info->tilesize_R+info->offset_R, -info->offset_T);
        GuiImages::AssignSpriteToImage(spr_right_top, info->image,     GuiRect(imgwidth-info->tilesize_R, 0, info->tilesize_R, info->tilesize_T));
    }
    if( info->tilesize_L > 1 && info->tilesize_B > 1 ) {
        spr_left_bottom   = new GuiSprite(this, "left_bottom", NULL,   -info->offset_L, height-info->tilesize_B+info->offset_B);
        GuiImages::AssignSpriteToImage(spr_left_bottom, info->image,   GuiRect(0, imgheight-info->tilesize_B, info->tilesize_L, info->tilesize_B));
    }
    if( info->tilesize_B > 1 ) {
        spr_middle_bottom = new GuiSprite(this, "middle_bottom", NULL, info->tilesize_L-info->offset_L, height-info->tilesize_B+info->offset_B);
        GuiImages::AssignSpriteToImage(spr_middle_bottom, info->image, GuiRect(info->tilesize_L, imgheight-info->tilesize_B, imgwidth-info->tilesize_L-info->tilesize_R, info->tilesize_B));
    }
    if( info->tilesize_R > 1 && info->tilesize_B > 1 ) {
        spr_right_bottom  = new GuiSprite(this, "right_bottom", NULL,  width-info->tilesize_R+info->offset_R, height-info->tilesize_B+info->offset_B);
        GuiImages::AssignSpriteToImage(spr_right_bottom, info->image,  GuiRect(imgwidth-info->tilesize_R, imgheight-info->tilesize_B, info->tilesize_R, info->tilesize_B));
    }
    if( info->tilesize_L > 1 ) {
        spr_middle_left   = new GuiSprite(this, "middle_left", NULL,   -info->offset_L, info->tilesize_T-info->offset_T);
        GuiImages::AssignSpriteToImage(spr_middle_left, info->image,   GuiRect(0, info->tilesize_T, info->tilesize_L, imgheight-info->tilesize_T-info->tilesize_B));
    }
    if( info->tilesize_R > 1 ) {
        spr_middle_right  = new GuiSprite(this, "middle_right", NULL,  width-info->tilesize_R+info->offset_R, info->tilesize_T-info->offset_T);
        GuiImages::AssignSpriteToImage(spr_middle_right, info->image,  GuiRect(imgwidth-info->tilesize_R, info->tilesize_T, info->tilesize_R, imgheight-info->tilesize_T-info->tilesize_B));
    }
    spr_middle            = new GuiSprite(this, "middle", NULL,        info->tilesize_L-info->offset_L, info->tilesize_T-info->offset_T);
    GuiImages::AssignSpriteToImage(spr_middle, info->image,            GuiRect(info->tilesize_L, info->tilesize_T, imgwidth-info->tilesize_R-info->tilesize_L, imgheight-info->tilesize_T-info->tilesize_B));

    float swidth = (float)(width-info->tilesize_L-info->tilesize_R+info->offset_L+info->offset_R)/(imgwidth-info->tilesize_L-info->tilesize_R);
    float sheight = (float)(height-info->tilesize_T-info->tilesize_B+info->offset_T+info->offset_B)/(imgheight-info->tilesize_T-info->tilesize_B);
    if( spr_middle != NULL ) {
        spr_middle->SetZoomX(swidth);
        spr_middle->SetZoomY(sheight);
        AddTop(spr_middle);
    }
    if( spr_middle_top != NULL ) {
        spr_middle_top->SetZoomX(swidth);
        AddTop(spr_middle_top);
    }
    if( spr_middle_bottom != NULL ) {
        spr_middle_bottom->SetZoomX(swidth);
        AddTop(spr_middle_bottom);
    }
    if( spr_middle_left != NULL ) {
        spr_middle_left->SetZoomY(sheight);
        AddTop(spr_middle_left);
    }
    if( spr_middle_right != NULL ) {
        spr_middle_right->SetZoomY(sheight);
        AddTop(spr_middle_right);
    }
    if( spr_left_top != NULL ) {
        AddTop(spr_left_top);
    }
    if( spr_right_top != NULL ) {
        AddTop(spr_right_top);
    }
    if( spr_left_bottom != NULL ) {
        AddTop(spr_left_bottom);
    }
    if( spr_right_bottom != NULL ) {
        AddTop(spr_right_bottom);
    }

    SetAlpha(alpha);
    SetPosition(posx, posy);
    SetWidth(width);
    SetHeight(height);
}

GuiElmFrame::~GuiElmFrame()
{
    if( spr_left_top != NULL )      RemoveAndDelete(spr_left_top);
    if( spr_middle_top != NULL )    RemoveAndDelete(spr_middle_top);
    if( spr_right_top != NULL )     RemoveAndDelete(spr_right_top);
    if( spr_left_bottom != NULL )   RemoveAndDelete(spr_left_bottom);
    if( spr_middle_bottom != NULL ) RemoveAndDelete(spr_middle_bottom);
    if( spr_right_bottom != NULL )  RemoveAndDelete(spr_right_bottom);
    if( spr_middle_left != NULL )   RemoveAndDelete(spr_middle_left);
    if( spr_middle_right != NULL )  RemoveAndDelete(spr_middle_right);
    if( spr_middle != NULL )        RemoveAndDelete(spr_middle);
}

