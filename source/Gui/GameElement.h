#ifndef _H_GAMEELEMENT
#define _H_GAMEELEMENT

#include "../expat/expat.h"
#include "../GuiBase/InputDevices.h"

typedef enum {
  GEP_KEYBOARD_JOYSTICK,
} GEP;

class GuiRootContainer;
class GuiImage;

class GameElement
{
public:
    GameElement(GuiRootContainer *root);
    GameElement(GuiRootContainer *root, GameElement* parent);
    virtual ~GameElement();

    unsigned CalcCRC(unsigned crc = 0);
    void SetName(const char *str);
    void SetCommandLine(const char *str);
    void SetScreenShot(int number, const char *str);
    void SetKeyMapping(BTN key, int event);
    void SetCheatFile(const char *str);
    void SetProperty(GEP prop, bool value);
    char *GetName(void);
    char *GetCommandLine(void);
    char *GetScreenShot(int number);
    char *GetCheatFile(void);
    bool GetProperty(GEP prop);
    int GetKeyMapping(BTN key);
    void FreeImage(int number);
    GuiImage* GetImage(int number);
    void DeleteImage(int number);
    GameElement *next;
private:
    GuiRootContainer *root_container;
    char *name;
    char *cmdline;
    char *screenshot[2];
    char *cheatfile;
    GuiImage *image[2];
    unsigned properties;
    int key_map[BTN_LAST];
};

#endif

