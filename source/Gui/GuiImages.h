#ifndef _GUI_IMAGES_H
#define _GUI_IMAGES_H

#include "../GuiBase/GuiSprite.h"
#include "../GuiBase/GuiRect.h"

struct _IMGPOS;
typedef struct _IMGPOS IMGPOS;

class GuiImage;
class GuiSprite;
class GuiRootContainer;

class GuiImages {
public:
    static void Init(GuiRootContainer *root);
    static void Close();

    static void GetImage(const char *name, GuiImage **image, GuiRect *rect);
    static float GetWidth(const char *name);
    static float GetHeight(const char *name);
    static void AssignSpriteToImage(GuiSprite *spr, const char *name, GuiRect rect = GuiRect());
private:
    static IMGPOS* GuiImages::FindImage(const char *name, GuiImage **image = NULL);

    static GuiRootContainer* root_container;
    static GuiImage* imgGui;
};

#endif
