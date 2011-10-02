
#ifndef _GUI_MAIN_H
#define _GUI_MAIN_H

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiBase/GuiRootContainer.h"

class GameElement;

class GuiMain : public GuiRootContainer
{
public:
    GuiMain();
    virtual ~GuiMain();
private:
    GuiEffectFade effectDefault;
    GuiEffectFade effectZoom;

    void blueMsxInit(int resetProperties);
    static bool RenderEmuImage(void *context);
    void blueMsxRun(GameElement *game, char *game_dir);
    void Main(void);
};

#endif

