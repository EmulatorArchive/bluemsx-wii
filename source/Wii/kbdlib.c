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

#include "kbdlib.h"
#include "../iodevice/led.h"
#include "../wiiusbkeyboard/keyboard.h"
#include "../wiiusbkeyboard/wsksymdef.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define TIME_BEFORE_REPEATING 500
#define TIME_BETWEEN_REPEAT   200

typedef struct {
    KEY key;
    int code_a;
    int code_b;
} KEYCODE;

typedef struct {
    KEY key_a;
    KEY key_b;
    int code;
} PADCODE;

typedef struct {
    int pad;
    int wpad;
} PAD2WPAD;

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
    u8 btnstatus[2][KEY_LAST]; /* save joystick buttons */
    u8 keystatus[KEY_LAST]; /* save key status */
    int keyidx;
    int connected;
    int quiting;
    u32 wpad[2];
};

static KBDHANDLE kbdHandle = NULL;

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

static PADCODE keypad[] =
{
    {KEY_UP,     KEY_NONE,  WPAD_BUTTON_UP    },
    {KEY_DOWN,   KEY_NONE,  WPAD_BUTTON_DOWN  },
    {KEY_LEFT,   KEY_NONE,  WPAD_BUTTON_LEFT  },
    {KEY_RIGHT,  KEY_NONE,  WPAD_BUTTON_RIGHT },
    {KEY_RETURN, KEY_SPACE, WPAD_BUTTON_A     },
    {KEY_ESCAPE, KEY_NONE,  WPAD_BUTTON_B     },
    {KEY_F12,    KEY_NONE,  WPAD_BUTTON_HOME  },
    {KEY_NONE,   KEY_NONE,  0}
};

static PAD2WPAD pad2wpad[] =
{
    {PAD_BUTTON_LEFT , WPAD_CLASSIC_BUTTON_LEFT  },
    {PAD_BUTTON_RIGHT, WPAD_CLASSIC_BUTTON_RIGHT },
    {PAD_BUTTON_DOWN , WPAD_CLASSIC_BUTTON_DOWN  },
    {PAD_BUTTON_UP   , WPAD_CLASSIC_BUTTON_UP    },
    {PAD_TRIGGER_Z   , WPAD_CLASSIC_BUTTON_ZR    },
    {PAD_TRIGGER_R   , WPAD_CLASSIC_BUTTON_FULL_R},
    {PAD_TRIGGER_L   , WPAD_CLASSIC_BUTTON_FULL_L},
    {PAD_BUTTON_A    , WPAD_CLASSIC_BUTTON_A     },
    {PAD_BUTTON_B    , WPAD_CLASSIC_BUTTON_B     },
    {PAD_BUTTON_X    , WPAD_CLASSIC_BUTTON_X     },
    {PAD_BUTTON_Y    , WPAD_CLASSIC_BUTTON_Y     },
    {PAD_BUTTON_MENU , WPAD_CLASSIC_BUTTON_HOME  },
    {PAD_BUTTON_START, WPAD_CLASSIC_BUTTON_PLUS  },
    {0               , 0                         }
};

static KEYCODE syms[] =
{
    // Function keys
    {KEY_F1,            KS_F1,  KS_f1},
    {KEY_F2,            KS_F2,  KS_f2},
    {KEY_F3,            KS_F3,  KS_f3},
    {KEY_F4,            KS_F4,  KS_f4},
    {KEY_F5,            KS_F5,  KS_f5},
    {KEY_F6,            KS_F6,  KS_f6},
    {KEY_F7,            KS_F7,  KS_f7},
    {KEY_F8,            KS_F8,  KS_f8},
    {KEY_F9,            KS_F9,  KS_f9},
    {KEY_F10,           KS_F10, KS_f10},
    {KEY_F11,           KS_F11, KS_f11},
    {KEY_F12,           KS_F12, KS_f12},

    // ASCII mapped keysyms
    {KEY_1,             KS_1},
    {KEY_2,             KS_2},
    {KEY_3,             KS_3},
    {KEY_4,             KS_4},
    {KEY_5,             KS_5},
    {KEY_6,             KS_6},
    {KEY_7,             KS_7},
    {KEY_8,             KS_8},
    {KEY_9,             KS_9},
    {KEY_0,             KS_0},

    {KEY_EXCLAIM,       KS_exclam},
    {KEY_AT,            KS_at},
    {KEY_HASH,          KS_numbersign},
    {KEY_DOLLAR,        KS_dollar},
    {KEY_PERCENT,       KS_percent},
    {KEY_CARET,         KS_asciicircum},
    {KEY_AMPERSAND,     KS_ampersand},
    {KEY_ASTERISK,      KS_asterisk},
    {KEY_LEFTPAREN,     KS_parenleft},
    {KEY_RIGHTPAREN,    KS_parenright},

    {KEY_A,             KS_A,   KS_a},
    {KEY_B,             KS_B,   KS_b},
    {KEY_C,             KS_C,   KS_c},
    {KEY_D,             KS_D,   KS_d},
    {KEY_E,             KS_E,   KS_e},
    {KEY_F,             KS_F,   KS_f},
    {KEY_G,             KS_G,   KS_g},
    {KEY_H,             KS_H,   KS_h},
    {KEY_I,             KS_I,   KS_i},
    {KEY_J,             KS_J,   KS_j},
    {KEY_K,             KS_K,   KS_k},
    {KEY_L,             KS_L,   KS_l},
    {KEY_M,             KS_M,   KS_m},
    {KEY_N,             KS_N,   KS_n},
    {KEY_O,             KS_O,   KS_o},
    {KEY_P,             KS_P,   KS_p},
    {KEY_Q,             KS_Q,   KS_q},
    {KEY_R,             KS_R,   KS_r},
    {KEY_S,             KS_S,   KS_s},
    {KEY_T,             KS_T,   KS_t},
    {KEY_U,             KS_U,   KS_u},
    {KEY_V,             KS_V,   KS_v},
    {KEY_W,             KS_W,   KS_w},
    {KEY_X,             KS_X,   KS_x},
    {KEY_Y,             KS_Y,   KS_y},
    {KEY_Z,             KS_Z,   KS_z},

    {KEY_MINUS,         KS_minus},
    {KEY_EQUALS,        KS_equal},
    {KEY_BACKSLASH,     KS_backslash},
    {KEY_LEFTBRACKET,   KS_bracketleft},
    {KEY_RIGHTBRACKET,  KS_bracketright},
    {KEY_COLON,         KS_colon},
    {KEY_QUOTE,         KS_apostrophe},
    {KEY_BACKQUOTE,     KS_grave},
    {KEY_COMMA,         KS_comma},
    {KEY_PERIOD,        KS_period},
    {KEY_SLASH,         KS_slash},

    {KEY_UNDERSCORE,    KS_underscore},
    {KEY_PLUS,          KS_plus},
    {KEY_BAR,           KS_bar},
    {KEY_LEFTBRACE,     KS_braceleft},
    {KEY_RIGHTBRACE,    KS_braceright},
    {KEY_SEMICOLON,     KS_semicolon},
    {KEY_QUOTEDBL,      KS_quotedbl},
    {KEY_ASCIITILDE,    KS_asciitilde},
    {KEY_LESS,          KS_less},
    {KEY_GREATER,       KS_greater},
    {KEY_QUESTION,      KS_question},

    {KEY_BACKSPACE,     KS_BackSpace},
    {KEY_TAB,           KS_Tab},
    {KEY_RETURN,        KS_Return},
    {KEY_ESCAPE,        KS_Escape},
    {KEY_SPACE,         KS_space},

    // Numeric keypad
    {KEY_KP0,           KS_KP_0},
    {KEY_KP1,           KS_KP_1},
    {KEY_KP2,           KS_KP_2},
    {KEY_KP3,           KS_KP_3},
    {KEY_KP4,           KS_KP_4},
    {KEY_KP5,           KS_KP_5},
    {KEY_KP6,           KS_KP_6},
    {KEY_KP7,           KS_KP_7},
    {KEY_KP8,           KS_KP_8},
    {KEY_KP9,           KS_KP_9},
    {KEY_KP_PERIOD,     KS_KP_Decimal},
    {KEY_KP_DIVIDE,     KS_KP_Divide},
    {KEY_KP_MULTIPLY,   KS_KP_Multiply},
    {KEY_KP_MINUS,      KS_KP_Subtract},
    {KEY_KP_PLUS,       KS_KP_Add},
    {KEY_KP_ENTER,      KS_KP_Enter},

    /* Arrows + Home/End pad */
    {KEY_UP,            KS_Up,      KS_KP_Up},
    {KEY_DOWN,          KS_Down,    KS_KP_Down},
    {KEY_RIGHT,         KS_Right,   KS_KP_Right},
    {KEY_LEFT,          KS_Left,    KS_KP_Left},
    {KEY_INSERT,        KS_Insert,  KS_KP_Insert},
    {KEY_DELETE,        KS_Delete,  KS_KP_Delete},
    {KEY_HOME,          KS_Home,    KS_KP_Home},
    {KEY_END,           KS_End,     KS_KP_End},
    {KEY_PAGEUP,        KS_Prior,   KS_KP_Prior},
    {KEY_PAGEDOWN,      KS_Next,    KS_KP_Next},

    {KEY_NUMLOCK,       KS_Num_Lock},
    {KEY_CAPSLOCK,      KS_Caps_Lock},
    {KEY_SCROLLOCK,     KS_Hold_Screen},
    {KEY_RSHIFT,        KS_Shift_R},
    {KEY_LSHIFT,        KS_Shift_L},
    {KEY_RCTRL,         KS_Control_R},
    {KEY_LCTRL,         KS_Control_L},
    {KEY_RALT,          KS_Alt_R},
    {KEY_LALT,          KS_Alt_L},
    {KEY_COMPOSE,       KS_Multi_key},      /* Multi-key compose key */

    /* Miscellaneous function keys */
    {KEY_PRINT,         KS_Print_Screen},
    {KEY_NONE, 0 }
};

static void FillKeyNames(void)
{
    /* Fill in the blanks in keynames */
    keynames[KEY_BACKSPACE] = "backspace";
    keynames[KEY_TAB]       = "tab";
    keynames[KEY_RETURN]    = "return";
    keynames[KEY_ESCAPE]    = "escape";
    keynames[KEY_SPACE]     = "space";
    keynames[KEY_EXCLAIM]   = "!";
    keynames[KEY_QUOTEDBL]  = "\"";
    keynames[KEY_HASH]      = "#";
    keynames[KEY_DOLLAR]    = "$";
    keynames[KEY_PERCENT]   = "%";
    keynames[KEY_AMPERSAND] = "&";
    keynames[KEY_QUOTE]     = "'";
    keynames[KEY_LEFTPAREN] = "(";
    keynames[KEY_RIGHTPAREN] = ")";
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
    keynames[KEY_LEFTBRACE] = "{",
    keynames[KEY_BAR]       = "|",
    keynames[KEY_RIGHTBRACE] = "}",
    keynames[KEY_ASCIITILDE] = "~",
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

int KBD_IsConnected(void)
{
    return kbdHandle->connected;
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
    u32 buttons, gcbuttons;
    PAD2WPAD *p2w;
    joystick_t padjoy;
    static int count;

    // Check standard buttons
    buttons = WPAD_ButtonsHeld(channel);

    // Add GameCube buttons
    gcbuttons = PAD_ButtonsHeld(channel);
    p2w = pad2wpad;
    while( p2w->pad ) {
        if( gcbuttons & p2w->pad ) {
            buttons |= p2w->wpad;
        }
        p2w++;
    }

    // Check extensions
    WPAD_Probe(channel, &extensions);
    if( extensions == WPAD_EXP_NUNCHUK ) {
      // Special nunchuk mappings
      if( buttons & WPAD_NUNCHUK_BUTTON_Z ) {
        buttons &= ~WPAD_NUNCHUK_BUTTON_Z;
        buttons |= WPAD_CLASSIC_BUTTON_A;
      }
      if( buttons & WPAD_NUNCHUK_BUTTON_C ) {
        buttons &= ~WPAD_NUNCHUK_BUTTON_C;
        buttons |= WPAD_CLASSIC_BUTTON_B;
      }
      // Nunchuk stick
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.nunchuk.js);
    } else if( extensions == WPAD_EXP_CLASSIC ) {
      // Both classic controller sticks
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.classic.ljs);
      buttons |= GetJoystickDirection(&data.exp.classic.rjs);
    }

    // Scan GameCube sticks
    padjoy.min.x = 0;
    padjoy.min.y = 0;
    padjoy.max.x = 255;
    padjoy.max.y = 255;
    padjoy.center.x = 128;
    padjoy.center.y = 128;

    padjoy.pos.x = (int)PAD_StickX(channel) + 128;
    padjoy.pos.y = (int)PAD_StickY(channel) + 128;
    buttons |= GetJoystickDirection(&padjoy);

    padjoy.pos.x = (int)PAD_SubStickX(channel) + 128;
    padjoy.pos.y = (int)PAD_SubStickY(channel) + 128;
    buttons |= GetJoystickDirection(&padjoy);

    return buttons;
}

u32 KBD_GetPadButtons(void)
{
    int i;
    static u32 prev_buttons = 0;
    static u64 repeat_time = 0;
    u32 buttons = KBD_GetPadButtonStatus(WPAD_CHAN_0) | KBD_GetPadButtonStatus(WPAD_CHAN_1);
    KBD_GetKeys(NULL);
    for( i = 0; keypad[i].code != 0; i++ ) {
        if( KBD_GetKeyStatus(keypad[i].key_a) ||
            KBD_GetKeyStatus(keypad[i].key_b) ) {
            buttons |= keypad[i].code;
        }
    }
    if( buttons != prev_buttons ) {
        prev_buttons = buttons;
        repeat_time = ticks_to_millisecs(gettime()) + TIME_BEFORE_REPEATING;
        return buttons;
    }
    if( buttons != 0 && ticks_to_millisecs(gettime()) > repeat_time ) {
        repeat_time = ticks_to_millisecs(gettime()) + TIME_BETWEEN_REPEAT;
        return buttons;
    }
    return 0;
}

void KBD_GetKeys(KBD_CALLBACK cb)
{
    int i;
    int idx_prev = kbdHandle->keyidx;
    int idx_new  = kbdHandle->keyidx ^ 1;
    keyboard_event kbdEvent;

    while( KEYBOARD_GetEvent(&kbdEvent) ) {
        switch( kbdEvent.type ) {
            case KEYBOARD_PRESSED:
                for(i = 0; syms[i].key != KEY_NONE; i++)  {
                    if( kbdEvent.symbol == syms[i].code_a || kbdEvent.symbol == syms[i].code_b ) {
                        kbdHandle->keystatus[syms[i].key] = 1;
                        if( cb ) cb(syms[i].key, 1);
                        //printf("Key_press %s\n", KBD_GetKeyName(syms[i].key));
                    }
                }
                break;
            case KEYBOARD_RELEASED:
                for(i = 0; syms[i].key != KEY_NONE; i++)  {
                    if( kbdEvent.symbol == syms[i].code_a || kbdEvent.symbol == syms[i].code_b ) {
                        kbdHandle->keystatus[syms[i].key] = 0;
                        if( cb ) cb(syms[i].key, 0);
                        //printf("Key_release %s\n", KBD_GetKeyName(syms[i].key));
                    }
                }
                break;
            case KEYBOARD_DISCONNECTED:
                memset(kbdHandle->keystatus, 0, sizeof(kbdHandle->keystatus));
                memset(kbdHandle->btnstatus, 0, sizeof(kbdHandle->btnstatus));
                kbdHandle->connected--;
                break;
            case KEYBOARD_CONNECTED:
                kbdHandle->connected++;
                break;
        }
    }

    // fill new key status array
    memset(kbdHandle->btnstatus[idx_new], 0, sizeof(kbdHandle->btnstatus[0]));

    // handle WPAD buttons
    WPAD_ScanPads();
    PAD_ScanPads();
    kbdHandle->wpad[0] = KBD_GetPadButtonStatus(WPAD_CHAN_0);
    kbdHandle->wpad[1] = KBD_GetPadButtonStatus(WPAD_CHAN_1);
    for(i = 0; wpad[i].key_a != KEY_NONE; i++)  {
        if( (kbdHandle->wpad[0] & wpad[i].code) != 0  ) {
            kbdHandle->btnstatus[idx_new][wpad[i].key_a-KEY_JOY1_BUTTON_A] = 1;
        }
        if( (kbdHandle->wpad[1] & wpad[i].code) != 0  ) {
            kbdHandle->btnstatus[idx_new][wpad[i].key_b-KEY_JOY1_BUTTON_A] = 1;
        }
    }

    // compare with previous and call for each difference
    for(i = 0; i < KEY_LAST-KEY_JOY1_BUTTON_A; i++) {
        if( kbdHandle->btnstatus[idx_prev][i] != kbdHandle->btnstatus[idx_new][i] ) {
            if( cb ) cb((KEY)i+KEY_JOY1_BUTTON_A, kbdHandle->btnstatus[idx_new][i]);
        }
    }
    // switch new<->previous
    kbdHandle->keyidx ^= 1;

    // handle leds
    int led = ledGetCapslock();
    if( kbdHandle->leds != led ) {
        KEYBOARD_SetLed(KEYBOARD_LEDCAPS, led);
        kbdHandle->leds = led;
    }
}

int KBD_GetKeyStatus(KEY key)
{
    assert( key < KEY_LAST );
    if( key < KEY_JOY1_BUTTON_A ) {
        return kbdHandle->keystatus[key];
    }else{
        return kbdHandle->btnstatus[kbdHandle->keyidx][key-KEY_JOY1_BUTTON_A];
    }
}

void KBD_DeInit(void)
{
    if( kbdHandle != NULL ) {
        kbdHandle->quiting = 1;
        KEYBOARD_Deinit();
        free(kbdHandle);
        kbdHandle = NULL;
    }
}

int KBD_Init(void)
{
    static int wpad_initialized = 0;

    if( kbdHandle != NULL ) {
        KBD_DeInit();
    }

    kbdHandle = (KBDHANDLE)memalign(32, sizeof(KBDDATA));
    memset(kbdHandle, 0, sizeof(KBDDATA));

    FillKeyNames();

    if( !wpad_initialized ) {
        WPAD_Init();
        wpad_initialized = 1;
    }

    kbdHandle->connected = KEYBOARD_Init(NULL);
    return kbdHandle->connected;
}

