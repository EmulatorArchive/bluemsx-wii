
#ifndef _GUI_MAIN_H
#define _GUI_MAIN_H

#include "../GuiBase/GuiRootContainer.h"

class GameElement;

class GuiMain : public GuiRootContainer
{
public:
    GuiMain();
    virtual ~GuiMain();
private:
    void blueMsxInit(int resetProperties);
    static bool RenderEmuImage(void *context);
    void blueMsxRun(GameElement *game, char *game_dir);
    void Main(void);
};

#endif

