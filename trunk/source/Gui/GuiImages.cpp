
#include "GuiImages.h"

#include <assert.h>

#include "image_gui.inc"

#include "../GuiBase/GuiRootContainer.h"
#include "../GuiBase/GuiImage.h"
#include "../GuiBase/GuiSprite.h"

GuiRootContainer* GuiImages::root_container = NULL;
GuiImage* GuiImages::imgGui = NULL;

struct _IMGPOS {
    char *name;
    float posx;
    float posy;
    float width;
    float height;
};

static IMGPOS positions_bluemsx[] = {
#include "../Resource/image_gui_pos.inc"
  { NULL, 0, 0, 0, 0 }
};

void GuiImages::Init(GuiRootContainer *root)
{
    root_container = root;
    imgGui = new GuiImage();
    if(imgGui->LoadImage(image_gui) != IMG_LOAD_ERROR_NONE) exit(0);
}

void GuiImages::Close()
{
    root_container->Delete(imgGui);
    imgGui = NULL;
}

IMGPOS* GuiImages::FindImage(const char *name, GuiImage **image)
{
    IMGPOS *p;
    for(p = positions_bluemsx; p->name != NULL; p++) {
      if( strcmp(name, p->name)==0 ) {
        if( image != NULL ) *image = imgGui;
        return p; // match
      }
    }
    assert(0); // name not found
    return NULL;
}

void GuiImages::GetImage(const char *name, GuiImage **image, GuiRect *rect)
{
    IMGPOS *p = FindImage(name, image);
    if( p != NULL ) {
        rect->x = p->posx;
        rect->y = p->posy;
        rect->width = p->width;
        rect->height = p->height;
    }
}

float GuiImages::GetWidth(const char *name)
{
    IMGPOS *p = FindImage(name);
    return p? p->width : 0;
}

float GuiImages::GetHeight(const char *name)
{
    IMGPOS *p = FindImage(name);
    return p? p->height : 0;
}

void GuiImages::AssignSpriteToImage(GuiSprite *spr, const char *name, GuiRect rect)
{
    GuiImage *img;
    IMGPOS *p = FindImage(name, &img);
    if( p != NULL ) {
        IMGPOS imgpos = *p;
        if( rect.x > 0 ) {
            imgpos.posx += rect.x;
            imgpos.width -= rect.x;
        }
        if( rect.y > 0 ) {
            imgpos.posy += rect.y;
            imgpos.height -= rect.y;
        }
        if( rect.width > 0 && rect.width < imgpos.width ) {
            imgpos.width = rect.width;
        }
        if( rect.height > 0 && rect.height < imgpos.height ) {
            imgpos.height = rect.height;
        }
        rect.x = imgpos.posx;
        rect.y = imgpos.posy;
        rect.width = imgpos.width;
        rect.height = imgpos.height;
        spr->SetImage(img, rect);
    }
}

