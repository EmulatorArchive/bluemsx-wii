#ifndef _WII_INPUT_H
#define _WII_INPUT_H

#include "../Gui/GuiManager.h"

class GuiManager;
extern void keyboardInit(GuiManager *man);

extern void keyboardSetDirectory(char* directory);
extern void keyboardClose(void);
extern void keyboardReset(void);
extern void keyboardRemapKey(BTN key, int event);
extern int keyboardGetMapping(BTN key);
extern void keyboardSetFocus(int handle, int focus);
extern void keyboardUpdate(void);
extern int keyboardGetModifiers(void);

#endif

