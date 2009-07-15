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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include <keyboard.h>

#include "kbdlib.h"
#include "../iodevice/led.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

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
    int leds;
    u8 keys[6];
    u8 keystatus[2][KEY_LAST-KEY_JOY1_BUTTON_A]; /* only save joystick buttons */
    int keyidx;
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

static u32 GetJoystickDirection(joystick_t *js)
{
    u32 buttons = 0;

    // do not calculate unused axes
    if( js->pos.x != 0 || js->pos.y != 0 ) {
      float x, y;

      // calculate relative x,y positions according to min, max and center values
      if (js->pos.x >= js->center.x) {
        x = (js->pos.x - js->center.x) / (float)(js->max.x - js->center.x);
      } else {
        x = (js->pos.x - js->center.x) / (float)(js->center.x - js->min.x);
      }

      if (js->pos.y >= js->center.y) {
        y = (js->pos.y - js->center.y) / (float)(js->max.y - js->center.y);
      } else {
        y = (js->pos.y - js->center.y) / (float)(js->center.y - js->min.y);
      }

      // 'dead zone' calculation
      if( sqrt(x*x+y*y) > 0.5f ) {
        // assign directons from angle
        float theta = atan2(y,x) * (180/PI);

        if (theta > 35 && theta < 145)
          buttons |= WPAD_CLASSIC_BUTTON_UP;
        if (theta > 125 || theta < -125)
          buttons |= WPAD_CLASSIC_BUTTON_LEFT;
        if (theta > -145 && theta < -35)
          buttons |= WPAD_CLASSIC_BUTTON_DOWN;
        if (theta > -55 && theta < 55)
          buttons |= WPAD_CLASSIC_BUTTON_RIGHT;
      }
    }

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
      buttons |= GetJoystickDirection(&data.exp.nunchuk.js);
    } else if( extensions == WPAD_EXP_CLASSIC ) {
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.classic.ljs);
      buttons |= GetJoystickDirection(&data.exp.classic.rjs);
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
    int i;
    int idx_prev = hndl->keyidx;
    int idx_new  = hndl->keyidx ^ 1;
    keyboardEvent kbdEvent;

    KEYBOARD_ScanKeyboards();
    while( KEYBOARD_getEvent(&kbdEvent) ) {
        switch( kbdEvent.type ) {
            case KEYBOARD_PRESSED:
                if( kbdEvent.keysym.sym == 0 ) {
                    /* handle special keys */
                    hndl->modifiers |= kbdEvent.keysym.mod;
                    for(i = 0; mods[i].key != KEY_NONE; i++)  {
                        if( kbdEvent.keysym.mod == mods[i].code ) {
                            if( cb ) cb(hndl, mods[i].key, 1);
                        }
                    }
                } else {
                    if( cb ) cb(hndl, kbdEvent.keysym.sym, 1);
                }
                break;
            case KEYBOARD_RELEASED:
                if( kbdEvent.keysym.sym == 0 ) {
                    hndl->modifiers &= ~kbdEvent.keysym.mod;
                    /* handle special keys */
                    for(i = 0; mods[i].key != KEY_NONE; i++)  {
                        if( kbdEvent.keysym.mod == mods[i].code ) {
                            if( cb ) cb(hndl, mods[i].key, 0);
                        }
                    }
                } else {
                    if( cb ) cb(hndl, kbdEvent.keysym.sym, 0);
                }
                break;
            case KEYBOARD_DISCONNECTED:
                hndl->connected--;
                break;
            case KEYBOARD_CONNECTED:
                hndl->connected++;
                break;
        }
    }

    /* fill new key status array */
    memset(hndl->keystatus[idx_new], 0, sizeof(hndl->keystatus[0]));

    /* handle WPAD buttons */
    WPAD_ScanPads();
    hndl->wpad[0] = KBD_GetPadButtonStatus(WPAD_CHAN_0);
    hndl->wpad[1] = KBD_GetPadButtonStatus(WPAD_CHAN_1);
    for(i = 0; wpad[i].key_a != KEY_NONE; i++)  {
        if( (hndl->wpad[0] & wpad[i].code) != 0  ) {
            hndl->keystatus[idx_new][wpad[i].key_a-KEY_JOY1_BUTTON_A] = 1;
        }
        if( (hndl->wpad[1] & wpad[i].code) != 0  ) {
            hndl->keystatus[idx_new][wpad[i].key_b-KEY_JOY1_BUTTON_A] = 1;
        }
    }

    /* compare with previous and call for each difference */
    for(i = 0; i < KEY_LAST-KEY_JOY1_BUTTON_A; i++) {
        if( hndl->keystatus[idx_prev][i] != hndl->keystatus[idx_new][i] ) {
            if( cb ) cb(hndl, (KEY)i+KEY_JOY1_BUTTON_A, hndl->keystatus[idx_new][i]);
        }
    }
    /* switch new<->previous */
    hndl->keyidx ^= 1;

    /* handle leds */
    int led = ledGetCapslock();
    if( hndl->leds != led ) {
        if( led ) {
            KEYBOARD_putOnLed(KEYBOARD_LEDCAPS);
        } else {
            KEYBOARD_putOffLed(KEYBOARD_LEDCAPS);
        }
        hndl->leds = led;
    }
}

int KBD_GetKeyStatus(KBDHANDLE hndl, KEY key)
{
    assert( key >= KEY_JOY1_BUTTON_A );
    assert( key < KEY_LAST );
    return hndl->keystatus[hndl->keyidx][key-KEY_JOY1_BUTTON_A];
}

void KBD_DeInit(KBDHANDLE hndl)
{
    hndl->quiting = 1;
    KEYBOARD_Deinit();
    free(hndl);
}

KBDHANDLE KBD_Init(void)
{
    static int wpad_initialized = 0;
    KBDHANDLE hndl = (KBDHANDLE)memalign(32, sizeof(KBDDATA));
    memset(hndl, 0, sizeof(KBDDATA));

    FillKeyNames();
    if( !wpad_initialized ) {
        WPAD_Init();
        wpad_initialized = 1;
    }
    hndl->connected = KEYBOARD_Init();
    return hndl;
}

