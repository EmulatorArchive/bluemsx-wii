
#include "GuiButton.h"
#include "GuiImages.h"

GuiButton::GuiButton(GuiManager *man)
{
    manager = man;
    sprImage = NULL;
    sprSelector = NULL;
}

GuiButton::~GuiButton()
{
    Remove();
}

void GuiButton::ShowImageButton(Image *image, int x, int y, int f_show, int f_sel)
{
    posx = x;
    posy = y;
    fade_sel = f_sel;
    sprImage = new Sprite(image, x, y);
    manager->Lock();
    manager->AddTop(sprImage, f_show);
    manager->Unlock();
}

void GuiButton::Remove(int fade, int delay)
{
    manager->Lock();

    if( sprSelector != NULL ) {
        manager->RemoveAndDelete(sprSelector, NULL, fade, delay);
    }
    manager->RemoveAndDelete(sprImage, NULL, fade, delay);
    sprImage = NULL;

    manager->Unlock();
}

bool GuiButton::Update(Sprite *cursor)
{
    manager->Lock();

    if( sprImage != NULL && cursor != NULL ) {
        if( sprSelector == NULL && cursor->CollidesWith(sprImage, true) ) {
            sprSelector = new Sprite(g_imgSelector2, posx-6, posy-6);
            manager->AddBehind(sprImage, sprSelector, fade_sel);
        }
        if( sprSelector != NULL && !cursor->CollidesWith(sprImage, true) ) {
            manager->RemoveAndDelete(sprSelector, NULL, fade_sel);
            sprSelector = NULL;
        }
    }else{
        if( sprSelector != NULL ) {
            manager->RemoveAndDelete(sprSelector, NULL, fade_sel);
            sprSelector = NULL;
        }
    }
    
    manager->Unlock();

    return (sprSelector != NULL);
}

