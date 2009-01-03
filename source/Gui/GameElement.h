#ifndef _H_GAMEELEMENT
#define _H_GAMEELEMENT

#include <wiisprite.h>
#include "expat.h"

class GameElement
{
public:
    GameElement();
    virtual ~GameElement();

    void SetName(const char *str);
    void SetCommandLine(const char *str);
    void SetScreenShot(int number, const char *str);
    char *GetName();
    char *GetCommandLine();
    char *GetScreenShot(int number);
    void FreeImage(int number);
    wsp::Image* GetImage(int number);
    GameElement *next;
private:
    char *name;
    char *cmdline;
    char *screenshot[2];
    wsp::Image *image[2];
};

#endif

