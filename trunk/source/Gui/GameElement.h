#ifndef _H_GAMEELEMENT
#define _H_GAMEELEMENT

#include <wiisprite.h>
#include "expat.h"
#include "kbdlib.h"

using namespace wsp;

typedef enum {
  GEP_KEYBOARD_JOYSTICK,
} GEP;

class GameElement
{
public:
    GameElement();
    GameElement(GameElement* parent);
    virtual ~GameElement();

	unsigned CalcCRC(unsigned crc = 0);
    void SetName(const char *str);
    void SetCommandLine(const char *str);
    void SetScreenShot(int number, const char *str);
    void SetKeyMapping(KEY key, int event);
    void SetCheatFile(const char *str);
	void SetProperty(GEP prop, bool value);
    char *GetName(void);
    char *GetCommandLine(void);
    char *GetScreenShot(int number);
    char *GetCheatFile(void);
	bool GetProperty(GEP prop);
    int GetKeyMapping(KEY key);
    void FreeImage(int number);
    Image* GetImage(int number);
	void DeleteImage(int number);
    GameElement *next;
private:
    char *name;
    char *cmdline;
    char *screenshot[2];
    char *cheatfile;
    Image *image[2];
	unsigned properties;
    int key_map[KEY_LAST];
};

#endif

