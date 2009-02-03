/*****************************************************************************
** $Source: kbdlib.c,v $
**
** $Revision: 0.0 $
**
** $Date: 2008/12/09 00:00:00 $
**
** Copyright (C) 2008 Tim Brugman
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>

#include "kbdlib.h"

#define THRESHOLD_NUNCHUCK 50
#define THRESHOLD_CLASSIC  10

#define TIME_BEFORE_REPEATING 500
#define TIME_BETWEEN_REPEAT   200

typedef struct {
	KEY key;
	int code;
} KEYCODE;

typedef struct {
	KEY key_a;
	KEY key_b;
	int code;
} PADCODE;

typedef struct {
	u32 message;
	u32 id; // in fact it is a direction
	u8 modifiers;
	u8 unknown;
	u8 keys[6];
} key_data_t;

struct _kbd_data {
    key_data_t msg;
    u8 modifiers;
	u8 keys[6];
    u8 keystatus[2][KEY_LAST];
    int keyidx;
    int fhandle;
    int connected;
    int quiting;
    u32 wpad[2];
};

static const char *keynames[KEY_LAST];

static PADCODE wpad[] =
{
	{KEY_JOY1_BUTTON_A, KEY_JOY2_BUTTON_A, (WPAD_BUTTON_A   | WPAD_CLASSIC_BUTTON_A)    },
	{KEY_JOY1_BUTTON_B, KEY_JOY2_BUTTON_B, (WPAD_BUTTON_B   | WPAD_CLASSIC_BUTTON_B)    },
	{KEY_JOY1_BUTTON_1, KEY_JOY2_BUTTON_1,  WPAD_BUTTON_1                               },
	{KEY_JOY1_BUTTON_2, KEY_JOY2_BUTTON_2,  WPAD_BUTTON_2                               },
	{KEY_JOY1_UP,       KEY_JOY2_UP,       (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_UP)   },
	{KEY_JOY1_DOWN,     KEY_JOY2_DOWN,     (WPAD_BUTTON_LEFT  | WPAD_CLASSIC_BUTTON_DOWN) },
	{KEY_JOY1_LEFT,     KEY_JOY2_LEFT,     (WPAD_BUTTON_UP    | WPAD_CLASSIC_BUTTON_LEFT) },
	{KEY_JOY1_RIGHT,    KEY_JOY2_RIGHT,    (WPAD_BUTTON_DOWN  | WPAD_CLASSIC_BUTTON_RIGHT)},
	{KEY_JOY1_HOME,     KEY_JOY2_HOME,     (WPAD_BUTTON_HOME  | WPAD_CLASSIC_BUTTON_HOME) },
	{KEY_JOY1_PLUS,     KEY_JOY2_PLUS,     (WPAD_BUTTON_PLUS  | WPAD_CLASSIC_BUTTON_PLUS) },
    {KEY_JOY1_MINUS,    KEY_JOY2_MINUS,    (WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS)},
    {KEY_JOY1_BUTTON_X, KEY_JOY2_BUTTON_X,  WPAD_CLASSIC_BUTTON_X                         },
    {KEY_JOY1_BUTTON_Y, KEY_JOY2_BUTTON_Y,  WPAD_CLASSIC_BUTTON_Y                         },
    {KEY_JOY1_BUTTON_L, KEY_JOY2_BUTTON_L,  WPAD_CLASSIC_BUTTON_FULL_L                    },
    {KEY_JOY1_BUTTON_R, KEY_JOY2_BUTTON_R,  WPAD_CLASSIC_BUTTON_FULL_R                    },
    {KEY_JOY1_BUTTON_ZL,KEY_JOY2_BUTTON_ZL, WPAD_CLASSIC_BUTTON_ZL                        },
    {KEY_JOY1_BUTTON_ZR,KEY_JOY2_BUTTON_ZR, WPAD_CLASSIC_BUTTON_ZR                        },
	{KEY_NONE, KEY_NONE, 0}
};

static KEYCODE mods[] =
{
	{KEY_LCTRL,   0x01},
	{KEY_LSHIFT,  0x02},
	{KEY_LALT,    0x04},
	{KEY_LWIN,    0x08},
	{KEY_RCTRL,   0x10},
	{KEY_RSHIFT,  0x20},
	{KEY_RALT,    0x40},
	{KEY_RWIN,    0x80},
	{KEY_NONE, 0}
};

static KEYCODE keys[] =
{
	{KEY_A, 4},
	{KEY_B, 5},
	{KEY_C, 6},
	{KEY_D, 7},
	{KEY_E, 8},
	{KEY_F, 9},
	{KEY_G, 10},
	{KEY_H, 11},
	{KEY_I, 12},
	{KEY_J, 13},
	{KEY_K, 14},
	{KEY_L, 15},
	{KEY_M, 16},
	{KEY_N, 17},
	{KEY_O, 18},
	{KEY_P, 19},
	{KEY_Q, 20},
	{KEY_R, 21},
	{KEY_S, 22},
	{KEY_T, 23},
	{KEY_U, 24},
	{KEY_V, 25},
	{KEY_W, 26},
	{KEY_X, 27},
	{KEY_Y, 28},
	{KEY_Z, 29},
	{KEY_1, 30},
	{KEY_2, 31},
	{KEY_3, 32},
	{KEY_4, 33},
	{KEY_5, 34},
	{KEY_6, 35},
	{KEY_7, 36},
	{KEY_8, 37},
	{KEY_9, 38},
	{KEY_0, 39},
	{KEY_RETURN, 40},
	{KEY_ESCAPE, 41},
	{KEY_BACKSPACE, 42},
	{KEY_TAB, 43},
	{KEY_SPACE, 44},
	{KEY_MINUS, 45},
	{KEY_EQUALS, 46},
	{KEY_LEFTBRACKET, 47},
	{KEY_RIGHTBRACKET, 48},
	{KEY_BACKSLASH, 49},
	{KEY_SEMICOLON, 51},
	{KEY_QUOTE, 52},
	{KEY_BACKQUOTE, 53},
	{KEY_COMMA, 54},
	{KEY_PERIOD, 55},
	{KEY_SLASH, 56},
	{KEY_CAPSLOCK, 57},
	{KEY_F1, 58},
	{KEY_F2, 59},
	{KEY_F3, 60},
	{KEY_F4, 61},
	{KEY_F5, 62},
	{KEY_F6, 63},
	{KEY_F7, 64},
	{KEY_F8, 65},
	{KEY_F9, 66},
	{KEY_F10, 67},
	{KEY_F11, 68},
	{KEY_F12, 69},
	{KEY_PRINT, 70},
	{KEY_SCROLLOCK, 71},
	{KEY_PAUSE, 72},
	{KEY_INSERT, 73},
	{KEY_HOME, 74},
	{KEY_PAGEUP, 75},
	{KEY_DELETE, 76},
	{KEY_END, 77},
	{KEY_PAGEDOWN, 78},
	{KEY_RIGHT, 79},
	{KEY_LEFT, 80},
	{KEY_DOWN, 81},
	{KEY_UP, 82},
	{KEY_NUMLOCK, 83},
	{KEY_KP_DIVIDE, 84},
	{KEY_KP_MULTIPLY, 85},
	{KEY_KP_MINUS, 86},
	{KEY_KP_PLUS, 87},
	{KEY_KP_ENTER, 88},
	{KEY_KP1, 89},
	{KEY_KP2, 90},
	{KEY_KP3, 91},
	{KEY_KP4, 92},
	{KEY_KP5, 93},
	{KEY_KP6, 94},
	{KEY_KP7, 95},
	{KEY_KP8, 96},
	{KEY_KP9, 97},
	{KEY_KP0, 98},
	{KEY_KP_PERIOD, 99},
	{KEY_KP_EQUALS, 100},
	{KEY_NONE, 0}
};

static void FillKeyNames(void)
{
    /* Fill in the blanks in keynames */
    keynames[KEY_BACKSPACE] = "backspace";
    keynames[KEY_TAB]       = "tab";
    keynames[KEY_CLEAR]     = "clear";
    keynames[KEY_RETURN]    = "return";
    keynames[KEY_PAUSE]     = "pause";
    keynames[KEY_ESCAPE]    = "escape";
    keynames[KEY_SPACE]     = "space";
    keynames[KEY_EXCLAIM]   = "!";
    keynames[KEY_QUOTEDBL]  = "\"";
    keynames[KEY_HASH]      = "#";
    keynames[KEY_DOLLAR]    = "$";
    keynames[KEY_AMPERSAND] = "&";
    keynames[KEY_QUOTE]     = "'";
    keynames[KEY_LEFTPAREN] = "(";
    keynames[KEY_RIGHTPAREN]= ")";
    keynames[KEY_ASTERISK]  = "*";
    keynames[KEY_PLUS]      = "+";
    keynames[KEY_COMMA]     = ",";
    keynames[KEY_MINUS]     = "-";
    keynames[KEY_PERIOD]    = ".";
    keynames[KEY_SLASH]     = "/";
    keynames[KEY_0]         = "0";
    keynames[KEY_1]         = "1";
    keynames[KEY_2]         = "2";
    keynames[KEY_3]         = "3";
    keynames[KEY_4]         = "4";
    keynames[KEY_5]         = "5";
    keynames[KEY_6]         = "6";
    keynames[KEY_7]         = "7";
    keynames[KEY_8]         = "8";
    keynames[KEY_9]         = "9";
    keynames[KEY_COLON]     = ":";
    keynames[KEY_SEMICOLON] = ";";
    keynames[KEY_LESS]      = "<";
    keynames[KEY_EQUALS]    = "=";
    keynames[KEY_GREATER]   = ">";
    keynames[KEY_QUESTION]  = "?";
    keynames[KEY_AT]        = "@";
    keynames[KEY_LEFTBRACKET] = "[";
    keynames[KEY_BACKSLASH] = "\\";
    keynames[KEY_RIGHTBRACKET] = "]";
    keynames[KEY_CARET]     = "^";
    keynames[KEY_UNDERSCORE] = "_";
    keynames[KEY_BACKQUOTE] = "`";
    keynames[KEY_A]         = "a";
    keynames[KEY_B]         = "b";
    keynames[KEY_C]         = "c";
    keynames[KEY_D]         = "d";
    keynames[KEY_E]         = "e";
    keynames[KEY_F]         = "f";
    keynames[KEY_G]         = "g";
    keynames[KEY_H]         = "h";
    keynames[KEY_I]         = "i";
    keynames[KEY_J]         = "j";
    keynames[KEY_K]         = "k";
    keynames[KEY_L]         = "l";
    keynames[KEY_M]         = "m";
    keynames[KEY_N]         = "n";
    keynames[KEY_O]         = "o";
    keynames[KEY_P]         = "p";
    keynames[KEY_Q]         = "q";
    keynames[KEY_R]         = "r";
    keynames[KEY_S]         = "s";
    keynames[KEY_T]         = "t";
    keynames[KEY_U]         = "u";
    keynames[KEY_V]         = "v";
    keynames[KEY_W]         = "w";
    keynames[KEY_X]         = "x";
    keynames[KEY_Y]         = "y";
    keynames[KEY_Z]         = "z";
    keynames[KEY_DELETE]    = "delete";

    keynames[KEY_KP0]       = "[0]";
    keynames[KEY_KP1]       = "[1]";
    keynames[KEY_KP2]       = "[2]";
    keynames[KEY_KP3]       = "[3]";
    keynames[KEY_KP4]       = "[4]";
    keynames[KEY_KP5]       = "[5]";
    keynames[KEY_KP6]       = "[6]";
    keynames[KEY_KP7]       = "[7]";
    keynames[KEY_KP8]       = "[8]";
    keynames[KEY_KP9]       = "[9]";
    keynames[KEY_KP_PERIOD] = "[.]";
    keynames[KEY_KP_DIVIDE] = "[/]";
    keynames[KEY_KP_MULTIPLY] = "[*]";
    keynames[KEY_KP_MINUS]  = "[-]";
    keynames[KEY_KP_PLUS]   = "[+]";
    keynames[KEY_KP_ENTER]  = "enter";
    keynames[KEY_KP_EQUALS] = "equals";

    keynames[KEY_UP]        = "up";
    keynames[KEY_DOWN]      = "down";
    keynames[KEY_RIGHT]     = "right";
    keynames[KEY_LEFT]      = "left";
    keynames[KEY_DOWN]      = "down";
    keynames[KEY_INSERT]    = "insert";
    keynames[KEY_HOME]      = "home";
    keynames[KEY_END]       = "end";
    keynames[KEY_PAGEUP]    = "page_up";
    keynames[KEY_PAGEDOWN]  = "page_down";

    keynames[KEY_F1]        = "f1";
    keynames[KEY_F2]        = "f2";
    keynames[KEY_F3]        = "f3";
    keynames[KEY_F4]        = "f4";
    keynames[KEY_F5]        = "f5";
    keynames[KEY_F6]        = "f6";
    keynames[KEY_F7]        = "f7";
    keynames[KEY_F8]        = "f8";
    keynames[KEY_F9]        = "f9";
    keynames[KEY_F10]       = "f10";
    keynames[KEY_F11]       = "f11";
    keynames[KEY_F12]       = "f12";

    keynames[KEY_NUMLOCK]   = "numlock";
    keynames[KEY_CAPSLOCK]  = "caps_lock";
    keynames[KEY_SCROLLOCK] = "scroll_lock";
    keynames[KEY_RSHIFT]    = "right_shift";
    keynames[KEY_LSHIFT]    = "left_shift";
    keynames[KEY_RCTRL]     = "right_ctrl";
    keynames[KEY_LCTRL]     = "left_ctrl";
    keynames[KEY_RALT]      = "right_alt";
    keynames[KEY_LALT]      = "left_alt";
    keynames[KEY_LWIN]      = "left_win";
    keynames[KEY_RWIN]      = "right_win";
    keynames[KEY_MODE]      = "alt_gr";
    keynames[KEY_COMPOSE]   = "compose";

    keynames[KEY_PRINT]     = "print_screen";

    keynames[KEY_JOY1_BUTTON_A] = "buttonA1";
    keynames[KEY_JOY1_BUTTON_B] = "buttonB1";
    keynames[KEY_JOY1_BUTTON_1] = "button11";
    keynames[KEY_JOY1_BUTTON_2] = "button21";
    keynames[KEY_JOY1_UP]       = "up1";
    keynames[KEY_JOY1_DOWN]     = "down1";
    keynames[KEY_JOY1_LEFT]     = "left1";
    keynames[KEY_JOY1_RIGHT]    = "right1";
    keynames[KEY_JOY1_HOME]     = "home1";
    keynames[KEY_JOY1_PLUS]     = "plus1";
    keynames[KEY_JOY1_MINUS]    = "minus1";
    keynames[KEY_JOY1_BUTTON_X] = "buttonX1";
    keynames[KEY_JOY1_BUTTON_Y] = "buttonY1";
    keynames[KEY_JOY1_BUTTON_L] = "buttonL1";
    keynames[KEY_JOY1_BUTTON_R] = "buttonR1";
    keynames[KEY_JOY1_BUTTON_ZL] = "buttonZL1";
    keynames[KEY_JOY1_BUTTON_ZR] = "buttonZR1";

    keynames[KEY_JOY2_BUTTON_A] = "buttonA2";
    keynames[KEY_JOY2_BUTTON_B] = "buttonB2";
    keynames[KEY_JOY2_BUTTON_1] = "button12";
    keynames[KEY_JOY2_BUTTON_2] = "button22";
    keynames[KEY_JOY2_UP]       = "up2";
    keynames[KEY_JOY2_DOWN]     = "down2";
    keynames[KEY_JOY2_LEFT]     = "left2";
    keynames[KEY_JOY2_RIGHT]    = "right2";
    keynames[KEY_JOY2_HOME]     = "home2";
    keynames[KEY_JOY2_PLUS]     = "plus2";
    keynames[KEY_JOY2_MINUS]    = "minus2";
    keynames[KEY_JOY2_BUTTON_X] = "buttonX2";
    keynames[KEY_JOY2_BUTTON_Y] = "buttonY2";
    keynames[KEY_JOY2_BUTTON_L] = "buttonL2";
    keynames[KEY_JOY2_BUTTON_R] = "buttonR2";
    keynames[KEY_JOY2_BUTTON_ZL] = "buttonZL2";
    keynames[KEY_JOY2_BUTTON_ZR] = "buttonZR2";
}

s32 keyboardCallback(int ret, void *arg)
{
    KBDHANDLE hndl = (KBDHANDLE)arg;
    if( hndl->quiting ) {
        return 0;
    }
    switch( hndl->msg.message ) {
        case 2: /* keys */
            hndl->modifiers = hndl->msg.modifiers;
            memcpy(hndl->keys, hndl->msg.keys, 6);
            break;
        case 1: /* disconnect */
            hndl->connected = 0;
            hndl->modifiers = 0;
            memset(hndl->keys, 0, 6);
            break;
        case 0: /* connect */
            hndl->connected = 1;
            hndl->modifiers = 0;
            memset(hndl->keys, 0, 6);
            break;
    }
    IOS_IoctlAsync(hndl->fhandle, 1, (void *)&hndl->msg, 16, (void *)&hndl->msg, 16, keyboardCallback, arg);
    return 0;
}

int KBD_IsConnected(KBDHANDLE hndl)
{
    return hndl->connected;
}

const char *KBD_GetKeyName(KEY key)
{
    const char *keyname;

    keyname = NULL;
    if ( key < KEY_LAST ) {
        keyname = keynames[key];
    }
    return keyname;
}

static u32 GetJoystickDirection(joystick_t *js, int threshold)
{
    u32 buttons = 0;
    // handle right
    if( js->pos.x > js->center.x + threshold)
        buttons |= WPAD_CLASSIC_BUTTON_RIGHT;
    // handle left
    if( js->pos.x < js->center.x - threshold)
        buttons |= WPAD_CLASSIC_BUTTON_LEFT;
    // handle up
    if( js->pos.y > js->center.y + threshold )
        buttons |= WPAD_CLASSIC_BUTTON_UP;
    // handle down
    if( js->pos.y < js->center.y - threshold )
        buttons |= WPAD_CLASSIC_BUTTON_DOWN;
    return buttons;
}

u32 KBD_GetPadButtonStatus(int channel)
{
    u32 extensions;
    WPADData data;
    u32 buttons;

    // Check standard buttons
    buttons = WPAD_ButtonsHeld(channel);

    // Check extensions
    WPAD_Probe(channel, &extensions);
    if( extensions == WPAD_EXP_NUNCHUK ) {
      if( buttons & WPAD_NUNCHUK_BUTTON_Z ) {
        buttons &= ~WPAD_NUNCHUK_BUTTON_Z;
        buttons |= WPAD_CLASSIC_BUTTON_A;
      }
      if( buttons & WPAD_NUNCHUK_BUTTON_C ) {
        buttons &= ~WPAD_NUNCHUK_BUTTON_C;
        buttons |= WPAD_CLASSIC_BUTTON_B;
      }
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.nunchuk.js, THRESHOLD_NUNCHUCK);
    }
    if( extensions == WPAD_EXP_CLASSIC ) {
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.classic.ljs, THRESHOLD_CLASSIC);
      buttons |= GetJoystickDirection(&data.exp.classic.rjs, THRESHOLD_CLASSIC);
    }
    return buttons;
}

u32 KBD_GetPadButtons(int channel)
{
    static u32 prev_buttons[WPAD_MAX_WIIMOTES] = {0, 0, 0, 0};
    static u64 repeat_time = 0;
    u32 buttons = KBD_GetPadButtonStatus(channel);
    if( buttons != prev_buttons[channel] ) {
        prev_buttons[channel] = buttons;
        repeat_time = ticks_to_millisecs(gettime()) + TIME_BEFORE_REPEATING;
        return buttons;
    }
    if( buttons != 0 && ticks_to_millisecs(gettime()) > repeat_time ) {
        repeat_time = ticks_to_millisecs(gettime()) + TIME_BETWEEN_REPEAT;
        return buttons;
    }
    return 0;
 }

void KBD_GetKeys(KBDHANDLE hndl, KBD_CALLBACK cb)
{
	int i, k;
    int idx_prev = hndl->keyidx;
    int idx_new  = hndl->keyidx ^ 1;

    /* fill new key status array */
    memset(hndl->keystatus[idx_new], 0, sizeof(hndl->keystatus[0]));
    for(i = 0; i < 6; i++) {
        if( hndl->keys[i] != 0 ) {
        	for(k = 0; keys[k].key != KEY_NONE; k++)	{
        		if( keys[k].code == hndl->keys[i] ) {
        			hndl->keystatus[idx_new][keys[k].key] = 1;
                    break;
        		}
        	}
        }
	}
    for(i = 0; mods[i].key != KEY_NONE; i++)  {
        if( (hndl->modifiers & mods[i].code) != 0  ) {
            hndl->keystatus[idx_new][mods[i].key] = 1;
        }
    }

    /* handle WPAD buttons */
    WPAD_ScanPads();
    hndl->wpad[0] = KBD_GetPadButtonStatus(WPAD_CHAN_0);
    hndl->wpad[1] = KBD_GetPadButtonStatus(WPAD_CHAN_1);
    for(i = 0; wpad[i].key_a != KEY_NONE; i++)  {
        if( (hndl->wpad[0] & wpad[i].code) != 0  ) {
            hndl->keystatus[idx_new][wpad[i].key_a] = 1;
        }
        if( (hndl->wpad[1] & wpad[i].code) != 0  ) {
            hndl->keystatus[idx_new][wpad[i].key_b] = 1;
        }
    }

    /* compare with previous and call for each difference */
    for(i = 0; i < KEY_LAST; i++) {
        if( hndl->keystatus[idx_prev][i] != hndl->keystatus[idx_new][i] ) {
            cb(hndl, (KEY)i, hndl->keystatus[idx_new][i]);
        }
    }
    /* switch new<->previous */
    hndl->keyidx ^= 1;
}

int KBD_GetKeyStatus(KBDHANDLE hndl, KEY key)
{
    return hndl->keystatus[hndl->keyidx][key];
}

void KBD_DeInit(KBDHANDLE hndl)
{
    hndl->quiting = 1;
}

KBDHANDLE KBD_Init(void)
{
    static int wpad_initialized = 0;
    int dev;
    FillKeyNames();
    if( !wpad_initialized ) {
    	WPAD_Init();
        wpad_initialized = 1;
    }
    dev = IOS_Open("/dev/usb/kbd", 1);
    if( dev >= 0 ) {
        KBDHANDLE hndl = (KBDHANDLE)memalign(32, sizeof(KBDDATA));
        memset(hndl, 0, sizeof(KBDDATA));
        hndl->fhandle = dev;
        IOS_IoctlAsync(hndl->fhandle, 1, (void *)&hndl->msg, 16, (void *)&hndl->msg, 16, keyboardCallback, (void*)hndl);
        return hndl;
    }else{
        return NULL;
    }
}

