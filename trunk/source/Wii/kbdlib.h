/*****************************************************************************
** $Source: kbdlib.h,v $
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
#ifndef _KBDLIB_H
#define _KBDLIB_H

#include <gctypes.h>

typedef enum {
    /* The numbers maps to ascii where possible */
    KEY_NONE        = 0,
    KEY_BACKSPACE   = 8,
    KEY_TAB         = 9,
    KEY_RETURN      = 13,
    KEY_ESCAPE      = 27,
    KEY_SPACE       = 32,
    KEY_EXCLAIM     = 33,
    KEY_QUOTEDBL    = 34,
    KEY_HASH        = 35,
    KEY_DOLLAR      = 36,
    KEY_PERCENT     = 37,
    KEY_AMPERSAND   = 38,
    KEY_QUOTE       = 39,
    KEY_LEFTPAREN   = 40,
    KEY_RIGHTPAREN  = 41,
    KEY_ASTERISK    = 42,
    KEY_PLUS        = 43,
    KEY_COMMA       = 44,
    KEY_MINUS       = 45,
    KEY_PERIOD      = 46,
    KEY_SLASH       = 47,
    KEY_0           = 48,
    KEY_1           = 49,
    KEY_2           = 50,
    KEY_3           = 51,
    KEY_4           = 52,
    KEY_5           = 53,
    KEY_6           = 54,
    KEY_7           = 55,
    KEY_8           = 56,
    KEY_9           = 57,
    KEY_COLON       = 58,
    KEY_SEMICOLON   = 59,
    KEY_LESS        = 60,
    KEY_EQUALS      = 61,
    KEY_GREATER     = 62,
    KEY_QUESTION    = 63,
    KEY_AT          = 64,
    KEY_LEFTBRACKET = 91,
    KEY_BACKSLASH   = 92,
    KEY_RIGHTBRACKET= 93,
    KEY_CARET       = 94,
    KEY_UNDERSCORE  = 95,
    KEY_BACKQUOTE   = 96,
    KEY_A           = 97,
    KEY_B           = 98,
    KEY_C           = 99,
    KEY_D           = 100,
    KEY_E           = 101,
    KEY_F           = 102,
    KEY_G           = 103,
    KEY_H           = 104,
    KEY_I           = 105,
    KEY_J           = 106,
    KEY_K           = 107,
    KEY_L           = 108,
    KEY_M           = 109,
    KEY_N           = 110,
    KEY_O           = 111,
    KEY_P           = 112,
    KEY_Q           = 113,
    KEY_R           = 114,
    KEY_S           = 115,
    KEY_T           = 116,
    KEY_U           = 117,
    KEY_V           = 118,
    KEY_W           = 119,
    KEY_X           = 120,
    KEY_Y           = 121,
    KEY_Z           = 122,
    KEY_LEFTBRACE   = 123,
    KEY_BAR         = 124,
    KEY_RIGHTBRACE  = 125,
    KEY_ASCIITILDE  = 126,
    KEY_DELETE      = 127,
    /* End of ASCII mapped keysyms */

    /* Numeric keypad */
    KEY_KP0         = 256,
    KEY_KP1         = 257,
    KEY_KP2         = 258,
    KEY_KP3         = 259,
    KEY_KP4         = 260,
    KEY_KP5         = 261,
    KEY_KP6         = 262,
    KEY_KP7         = 263,
    KEY_KP8         = 264,
    KEY_KP9         = 265,
    KEY_KP_PERIOD   = 266,
    KEY_KP_DIVIDE   = 267,
    KEY_KP_MULTIPLY = 268,
    KEY_KP_MINUS    = 269,
    KEY_KP_PLUS     = 270,
    KEY_KP_ENTER    = 271,
    KEY_KP_EQUALS   = 272,

    /* Arrows + Home/End pad */
    KEY_UP          = 273,
    KEY_DOWN        = 274,
    KEY_RIGHT       = 275,
    KEY_LEFT        = 276,
    KEY_INSERT      = 277,
    KEY_HOME        = 278,
    KEY_END         = 279,
    KEY_PAGEUP      = 280,
    KEY_PAGEDOWN    = 281,

    /* Function keys */
    KEY_F1          = 282,
    KEY_F2          = 283,
    KEY_F3          = 284,
    KEY_F4          = 285,
    KEY_F5          = 286,
    KEY_F6          = 287,
    KEY_F7          = 288,
    KEY_F8          = 289,
    KEY_F9          = 290,
    KEY_F10         = 291,
    KEY_F11         = 292,
    KEY_F12         = 293,

    /* Key state modifier keys */
    KEY_NUMLOCK     = 300,
    KEY_CAPSLOCK    = 301,
    KEY_SCROLLOCK   = 302,
    KEY_RSHIFT      = 303,
    KEY_LSHIFT      = 304,
    KEY_RCTRL       = 305,
    KEY_LCTRL       = 306,
    KEY_RALT        = 307,
    KEY_LALT        = 308,
    KEY_LWIN        = 311,
    KEY_RWIN        = 312,
    KEY_MODE        = 313,      /* "Alt Gr" key */
    KEY_COMPOSE     = 314,      /* Multi-key compose key */

    /* Miscellaneous function keys */
    KEY_PRINT       = 316,

    /* Joy pad */
    KEY_JOY1_BUTTON_A  = 400,
    KEY_JOY1_BUTTON_B  = 401,
    KEY_JOY1_BUTTON_1  = 402,
    KEY_JOY1_BUTTON_2  = 403,
    KEY_JOY1_UP        = 404,
    KEY_JOY1_DOWN      = 405,
    KEY_JOY1_LEFT      = 406,
    KEY_JOY1_RIGHT     = 407,
    KEY_JOY1_HOME      = 408,
    KEY_JOY1_PLUS      = 409,
    KEY_JOY1_MINUS     = 410,
    KEY_JOY1_BUTTON_X  = 411,
    KEY_JOY1_BUTTON_Y  = 412,
    KEY_JOY1_BUTTON_L  = 413,
    KEY_JOY1_BUTTON_R  = 414,
    KEY_JOY1_BUTTON_ZL = 415,
    KEY_JOY1_BUTTON_ZR = 416,

    KEY_JOY2_BUTTON_A  = 420,
    KEY_JOY2_BUTTON_B  = 421,
    KEY_JOY2_BUTTON_1  = 422,
    KEY_JOY2_BUTTON_2  = 423,
    KEY_JOY2_UP        = 424,
    KEY_JOY2_DOWN      = 425,
    KEY_JOY2_LEFT      = 426,
    KEY_JOY2_RIGHT     = 427,
    KEY_JOY2_HOME      = 428,
    KEY_JOY2_PLUS      = 429,
    KEY_JOY2_MINUS     = 430,
    KEY_JOY2_BUTTON_X  = 431,
    KEY_JOY2_BUTTON_Y  = 432,
    KEY_JOY2_BUTTON_L  = 433,
    KEY_JOY2_BUTTON_R  = 434,
    KEY_JOY2_BUTTON_ZL = 435,
    KEY_JOY2_BUTTON_ZR = 436,

    /* Add any other keys here */
    KEY_LAST
} KEY;

typedef struct _kbd_data KBDDATA, *KBDHANDLE;
typedef void (*KBD_CALLBACK)(KBDHANDLE, KEY, int);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void KBD_GetKeys(KBDHANDLE hndl, KBD_CALLBACK cb);
extern int KBD_GetKeyStatus(KBDHANDLE hndl, KEY key);
extern u32 KBD_GetPadButtons(int channel);
extern u32 KBD_GetPadButtonStatus(int channel);
extern void KBD_DeInit(KBDHANDLE hndl);
extern KBDHANDLE KBD_Init(void);
extern int KBD_IsConnected(KBDHANDLE hndl);
const char *KBD_GetKeyName(KEY key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

