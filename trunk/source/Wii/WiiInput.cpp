/*****************************************************************************
** $Source: WiiInput.c,v $
**
** $Revision: 1.6 $
**
** $Date: 2006/06/24 02:27:08 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#include "ArchInput.h"
#include "../Gui/GuiManager.h"

#include "Language.h"
#include "InputEvent.h"
#include "IniFileParser.h"
#include "WiiShortcuts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_JOYSTICKS 8


#define KBD_TABLE_LEN 512

static GuiManager *winp_gui_manager = NULL;
static Shortcuts* shortcuts = NULL;
static int kbdTable[KBD_TABLE_LEN];
static int keyStatus[KBD_TABLE_LEN];
static int kbdModifiers;
static int hasFocus = 1;

static char keyboardConfigDir[512];

// initKbdTable initializes the keyboard table with default keys
static void initKbdTable()
{
    memset(kbdTable, 0, sizeof(kbdTable));

    kbdTable[BTN_1          ] = EC_1;
    kbdTable[BTN_2          ] = EC_2;
    kbdTable[BTN_3          ] = EC_3;
    kbdTable[BTN_4          ] = EC_4;
    kbdTable[BTN_5          ] = EC_5;
    kbdTable[BTN_6          ] = EC_6;
    kbdTable[BTN_7          ] = EC_7;
    kbdTable[BTN_8          ] = EC_8;
    kbdTable[BTN_9          ] = EC_9;
    kbdTable[BTN_0          ] = EC_0;

    kbdTable[BTN_EXCLAIM    ] = EC_1;
    kbdTable[BTN_AT         ] = EC_2;
    kbdTable[BTN_HASH       ] = EC_3;
    kbdTable[BTN_DOLLAR     ] = EC_4;
    kbdTable[BTN_PERCENT    ] = EC_5;
    kbdTable[BTN_CARET      ] = EC_6;
    kbdTable[BTN_AMPERSAND  ] = EC_7;
    kbdTable[BTN_ASTERISK   ] = EC_8;
    kbdTable[BTN_LEFTPAREN  ] = EC_9;
    kbdTable[BTN_RIGHTPAREN ] = EC_0;

    kbdTable[BTN_A          ] = EC_A;
    kbdTable[BTN_B          ] = EC_B;
    kbdTable[BTN_C          ] = EC_C;
    kbdTable[BTN_D          ] = EC_D;
    kbdTable[BTN_E          ] = EC_E;
    kbdTable[BTN_F          ] = EC_F;
    kbdTable[BTN_G          ] = EC_G;
    kbdTable[BTN_H          ] = EC_H;
    kbdTable[BTN_I          ] = EC_I;
    kbdTable[BTN_J          ] = EC_J;
    kbdTable[BTN_K          ] = EC_K;
    kbdTable[BTN_L          ] = EC_L;
    kbdTable[BTN_M          ] = EC_M;
    kbdTable[BTN_N          ] = EC_N;
    kbdTable[BTN_O          ] = EC_O;
    kbdTable[BTN_P          ] = EC_P;
    kbdTable[BTN_Q          ] = EC_Q;
    kbdTable[BTN_R          ] = EC_R;
    kbdTable[BTN_S          ] = EC_S;
    kbdTable[BTN_T          ] = EC_T;
    kbdTable[BTN_U          ] = EC_U;
    kbdTable[BTN_V          ] = EC_V;
    kbdTable[BTN_W          ] = EC_W;
    kbdTable[BTN_X          ] = EC_X;
    kbdTable[BTN_Y          ] = EC_Y;
    kbdTable[BTN_Z          ] = EC_Z;

    kbdTable[BTN_MINUS      ] = EC_NEG;
    kbdTable[BTN_EQUALS     ] = EC_CIRCFLX;
    kbdTable[BTN_BACKSLASH  ] = EC_BKSLASH;
    kbdTable[BTN_LEFTBRACKET   ] = EC_AT;
    kbdTable[BTN_RIGHTBRACKET  ] = EC_LBRACK;
    kbdTable[BTN_COLON      ] = EC_SEMICOL;
    kbdTable[BTN_QUOTE      ] = EC_COLON;
    kbdTable[BTN_BACKQUOTE  ] = EC_RBRACK;
    kbdTable[BTN_COMMA      ] = EC_COMMA;
    kbdTable[BTN_PERIOD     ] = EC_PERIOD;
    kbdTable[BTN_SLASH      ] = EC_DIV;

    kbdTable[BTN_UNDERSCORE ] = EC_NEG;
    kbdTable[BTN_PLUS       ] = EC_CIRCFLX;
    kbdTable[BTN_BAR        ] = EC_BKSLASH;
    kbdTable[BTN_LEFTBRACE  ] = EC_AT;
    kbdTable[BTN_RIGHTBRACE ] = EC_LBRACK;
    kbdTable[BTN_SEMICOLON  ] = EC_SEMICOL;
    kbdTable[BTN_QUOTEDBL   ] = EC_COLON;
    kbdTable[BTN_ASCIITILDE ] = EC_RBRACK;
    kbdTable[BTN_LESS       ] = EC_COMMA;
    kbdTable[BTN_GREATER    ] = EC_PERIOD;
    kbdTable[BTN_QUESTION   ] = EC_DIV;

    kbdTable[BTN_RCTRL      ] = EC_UNDSCRE;

    kbdTable[BTN_F1         ] = EC_F1;
    kbdTable[BTN_F2         ] = EC_F2;
    kbdTable[BTN_F3         ] = EC_F3;
    kbdTable[BTN_F4         ] = EC_F4;
    kbdTable[BTN_F5         ] = EC_F5;
    kbdTable[BTN_F6         ] = EC_SELECT;
    kbdTable[BTN_F7         ] = EC_STOP;
    kbdTable[BTN_F8         ] = EC_CLS;
    kbdTable[BTN_F9         ] = EC_INS;
    kbdTable[BTN_F10        ] = EC_DEL;
    kbdTable[BTN_F11        ] = EC_HOT_DISK_QUICK_CHANGE;
    kbdTable[BTN_ESCAPE     ] = EC_ESC;
    kbdTable[BTN_TAB        ] = EC_TAB;
    kbdTable[BTN_PAGEUP     ] = EC_STOP;
    kbdTable[BTN_BACKSPACE  ] = EC_BKSPACE;
    kbdTable[BTN_END        ] = EC_SELECT;
    kbdTable[BTN_RETURN     ] = EC_RETURN;
    kbdTable[BTN_SPACE      ] = EC_SPACE;
    kbdTable[BTN_HOME       ] = EC_CLS;
    kbdTable[BTN_INSERT     ] = EC_INS;
    kbdTable[BTN_DELETE     ] = EC_DEL;
    kbdTable[BTN_LEFT       ] = EC_LEFT;
    kbdTable[BTN_UP         ] = EC_UP;
    kbdTable[BTN_RIGHT      ] = EC_RIGHT;
    kbdTable[BTN_DOWN       ] = EC_DOWN;

    kbdTable[BTN_KP_MULTIPLY] = EC_NUMMUL;
    kbdTable[BTN_KP_PLUS    ] = EC_NUMADD;
    kbdTable[BTN_KP_DIVIDE  ] = EC_NUMDIV;
    kbdTable[BTN_KP_MINUS   ] = EC_NUMSUB;
    kbdTable[BTN_KP_PERIOD  ] = EC_NUMPER;
    kbdTable[BTN_PAGEDOWN   ] = EC_NUMCOM;
    kbdTable[BTN_KP0        ] = EC_NUM0;
    kbdTable[BTN_KP1        ] = EC_NUM1;
    kbdTable[BTN_KP2        ] = EC_NUM2;
    kbdTable[BTN_KP3        ] = EC_NUM3;
    kbdTable[BTN_KP4        ] = EC_NUM4;
    kbdTable[BTN_KP5        ] = EC_NUM5;
    kbdTable[BTN_KP6        ] = EC_NUM6;
    kbdTable[BTN_KP7        ] = EC_NUM7;
    kbdTable[BTN_KP8        ] = EC_NUM8;
    kbdTable[BTN_KP9        ] = EC_NUM9;
    kbdTable[BTN_KP_ENTER   ] = EC_RETURN;

    kbdTable[BTN_LWIN       ] = EC_TORIKE;
    kbdTable[BTN_RWIN       ] = EC_JIKKOU;
    kbdTable[BTN_LSHIFT     ] = EC_LSHIFT;
    kbdTable[BTN_RSHIFT     ] = EC_RSHIFT;
    kbdTable[BTN_LCTRL      ] = EC_CTRL;
    kbdTable[BTN_LALT       ] = EC_GRAPH;
    kbdTable[BTN_RALT       ] = EC_CODE;
    kbdTable[BTN_CAPSLOCK   ] = EC_CAPS;
    kbdTable[BTN_KP_ENTER   ] = EC_PAUSE;
    kbdTable[BTN_PRINT      ] = EC_PRINT;

    kbdTable[BTN_JOY1_UP        ] = EC_JOY1_UP;
    kbdTable[BTN_JOY1_DOWN      ] = EC_JOY1_DOWN;
    kbdTable[BTN_JOY1_LEFT      ] = EC_JOY1_LEFT;
    kbdTable[BTN_JOY1_RIGHT     ] = EC_JOY1_RIGHT;
    kbdTable[BTN_JOY1_WIIMOTE_A ] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_WIIMOTE_B ] = EC_JOY1_BUTTON2;
    kbdTable[BTN_JOY1_WIIMOTE_1 ] = EC_JOY1_BUTTON2;
    kbdTable[BTN_JOY1_WIIMOTE_2 ] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_NUNCHUCK_C] = EC_JOY1_BUTTON2;
    kbdTable[BTN_JOY1_NUNCHUCK_Z] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_CLASSIC_A ] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_CLASSIC_B ] = EC_JOY1_BUTTON2;
    kbdTable[BTN_JOY1_CLASSIC_X ] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_CLASSIC_Y ] = EC_JOY1_BUTTON2;
    kbdTable[BTN_JOY1_CLASSIC_L ] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_CLASSIC_R ] = EC_JOY1_BUTTON2;
    kbdTable[BTN_JOY1_CLASSIC_ZL] = EC_JOY1_BUTTON1;
    kbdTable[BTN_JOY1_CLASSIC_ZR] = EC_JOY1_BUTTON2;

    kbdTable[BTN_JOY2_UP        ] = EC_JOY2_UP;
    kbdTable[BTN_JOY2_DOWN      ] = EC_JOY2_DOWN;
    kbdTable[BTN_JOY2_LEFT      ] = EC_JOY2_LEFT;
    kbdTable[BTN_JOY2_RIGHT     ] = EC_JOY2_RIGHT;
    kbdTable[BTN_JOY2_WIIMOTE_A ] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_WIIMOTE_B ] = EC_JOY2_BUTTON2;
    kbdTable[BTN_JOY2_WIIMOTE_1 ] = EC_JOY2_BUTTON2;
    kbdTable[BTN_JOY2_WIIMOTE_2 ] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_NUNCHUCK_C] = EC_JOY2_BUTTON2;
    kbdTable[BTN_JOY2_NUNCHUCK_Z] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_CLASSIC_A ] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_CLASSIC_B ] = EC_JOY2_BUTTON2;
    kbdTable[BTN_JOY2_CLASSIC_X ] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_CLASSIC_Y ] = EC_JOY2_BUTTON2;
    kbdTable[BTN_JOY2_CLASSIC_L ] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_CLASSIC_R ] = EC_JOY2_BUTTON2;
    kbdTable[BTN_JOY2_CLASSIC_ZL] = EC_JOY2_BUTTON1;
    kbdTable[BTN_JOY2_CLASSIC_ZR] = EC_JOY2_BUTTON2;

}

void keyboardRemapKey(BTN key, int event)
{
    kbdTable[key] = event;
}

int keyboardGetMapping(BTN key)
{
    return kbdTable[key];
}

void keyboardSetDirectory(char* directory)
{
    strcpy(keyboardConfigDir, directory);
}

void keyboardInit(GuiManager *man)
{
    winp_gui_manager = man;

    initKbdTable();
    inputEventReset();

    kbdModifiers = 0;

    shortcuts = shortcutsCreate();
}

void keyboardClose(void)
{
    if( shortcuts ) {
        shortcutsDestroy(shortcuts);
        shortcuts = NULL;
    }
}

void keyboardReset(void)
{
    initKbdTable();
    inputEventReset();
}

void keyboardSetFocus(int handle, int focus)
{
    if (focus) {
        hasFocus |= handle;
    }
    else {
        hasFocus &= ~handle;
    }
}

static void keyboardHandleKeypress(BTN code, int pressed)
{
    int wasPressed = keyStatus[code];
    int keyCode = kbdTable[code];
    keyStatus[code] = pressed;

    if (pressed != wasPressed && keyCode != 0) {
        if (pressed) {
            inputEventSet(keyCode);
            shortcutCheckDown(shortcuts, keyCode);
        }
        else {
            inputEventUnset(keyCode);
            shortcutCheckUp(shortcuts, keyCode);
        }
    }
}

static void keyboardCallbackKeypress(void *context, BTN code, int pressed)
{
    keyboardHandleKeypress(code, pressed);
}

static void keyboardResetKbd()
{
    int i;
    for (i = 0; i < KBD_TABLE_LEN; i++) {
        kbdModifiers = 0;
        keyboardHandleKeypress((BTN)i, 0);
    }
    inputEventReset();
}

void keyboardUpdate(void)
{
    if (!hasFocus) {
        keyboardResetKbd();
        return;
    }
    winp_gui_manager->gwd.input.GetButtonEvents(keyboardCallbackKeypress, NULL);

    kbdModifiers = ((keyStatus[BTN_LSHIFT] ? 1 : 0) << 0) | ((keyStatus[BTN_RSHIFT] ? 1 : 0) << 1) |
                   ((keyStatus[BTN_LCTRL]  ? 1 : 0) << 2) | ((keyStatus[BTN_RCTRL]  ? 1 : 0) << 3) |
                   ((keyStatus[BTN_LALT]   ? 1 : 0) << 4) | ((keyStatus[BTN_RALT]   ? 1 : 0) << 5) |
                   ((keyStatus[BTN_LWIN]   ? 1 : 0) << 6) | ((keyStatus[BTN_RWIN]   ? 1 : 0) << 7);
}

int keyboardGetModifiers(void)
{
    return kbdModifiers;
}

void  archUpdateJoystick() {}

UInt8 archJoystickGetState(int joystickNo) { return 0; }
int  archJoystickGetCount() { return 0; }
char* archJoystickGetName(int index) { return ""; }
void archMouseSetForceLock(int lock) { }

void  archPollInput()
{
    keyboardUpdate();
}
void  archKeyboardSetSelectedKey(int keyCode) {}
char* archGetSelectedKey() { return ""; }
char* archGetMappedKey() { return ""; }
int   archKeyboardIsKeyConfigured(int msxKeyCode) { return 0; }
int   archKeyboardIsKeySelected(int msxKeyCode) { return 0; }
char* archKeyconfigSelectedKeyTitle() { return ""; }
char* archKeyconfigMappedToTitle() { return ""; }
char* archKeyconfigMappingSchemeTitle() { return ""; }

