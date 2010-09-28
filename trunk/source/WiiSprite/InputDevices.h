/*
 * libwiisprite - Input devices
 */

#ifndef LIBWIISPRITE_INPUTDEVICES
#define LIBWIISPRITE_INPUTDEVICES

#include <gccore.h>
#ifdef WII
#include <wiiuse/wpad.h>
#endif
#include "mutex.h"

typedef enum {
    /* The numbers maps to ascii where possible */
    BTN_NONE        = 0,
    BTN_BACKSPACE   = 8,
    BTN_TAB         = 9,
    BTN_RETURN      = 13,
    BTN_ESCAPE      = 27,
    BTN_SPACE       = 32,
    BTN_EXCLAIM     = 33,
    BTN_QUOTEDBL    = 34,
    BTN_HASH        = 35,
    BTN_DOLLAR      = 36,
    BTN_PERCENT     = 37,
    BTN_AMPERSAND   = 38,
    BTN_QUOTE       = 39,
    BTN_LEFTPAREN   = 40,
    BTN_RIGHTPAREN  = 41,
    BTN_ASTERISK    = 42,
    BTN_PLUS        = 43,
    BTN_COMMA       = 44,
    BTN_MINUS       = 45,
    BTN_PERIOD      = 46,
    BTN_SLASH       = 47,
    BTN_0           = 48,
    BTN_1           = 49,
    BTN_2           = 50,
    BTN_3           = 51,
    BTN_4           = 52,
    BTN_5           = 53,
    BTN_6           = 54,
    BTN_7           = 55,
    BTN_8           = 56,
    BTN_9           = 57,
    BTN_COLON       = 58,
    BTN_SEMICOLON   = 59,
    BTN_LESS        = 60,
    BTN_EQUALS      = 61,
    BTN_GREATER     = 62,
    BTN_QUESTION    = 63,
    BTN_AT          = 64,
    BTN_LEFTBRACKET = 91,
    BTN_BACKSLASH   = 92,
    BTN_RIGHTBRACKET= 93,
    BTN_CARET       = 94,
    BTN_UNDERSCORE  = 95,
    BTN_BACKQUOTE   = 96,
    BTN_A           = 97,
    BTN_B           = 98,
    BTN_C           = 99,
    BTN_D           = 100,
    BTN_E           = 101,
    BTN_F           = 102,
    BTN_G           = 103,
    BTN_H           = 104,
    BTN_I           = 105,
    BTN_J           = 106,
    BTN_K           = 107,
    BTN_L           = 108,
    BTN_M           = 109,
    BTN_N           = 110,
    BTN_O           = 111,
    BTN_P           = 112,
    BTN_Q           = 113,
    BTN_R           = 114,
    BTN_S           = 115,
    BTN_T           = 116,
    BTN_U           = 117,
    BTN_V           = 118,
    BTN_W           = 119,
    BTN_X           = 120,
    BTN_Y           = 121,
    BTN_Z           = 122,
    BTN_LEFTBRACE   = 123,
    BTN_BAR         = 124,
    BTN_RIGHTBRACE  = 125,
    BTN_ASCIITILDE  = 126,
    BTN_DELETE      = 127,
    /* End of ASCII mapped keysyms */

    /* Numeric keypad */
    BTN_KP0         = 256,
    BTN_KP1         = 257,
    BTN_KP2         = 258,
    BTN_KP3         = 259,
    BTN_KP4         = 260,
    BTN_KP5         = 261,
    BTN_KP6         = 262,
    BTN_KP7         = 263,
    BTN_KP8         = 264,
    BTN_KP9         = 265,
    BTN_KP_PERIOD   = 266,
    BTN_KP_DIVIDE   = 267,
    BTN_KP_MULTIPLY = 268,
    BTN_KP_MINUS    = 269,
    BTN_KP_PLUS     = 270,
    BTN_KP_ENTER    = 271,
    BTN_KP_EQUALS   = 272,

    /* Arrows + Home/End pad */
    BTN_UP          = 273,
    BTN_DOWN        = 274,
    BTN_RIGHT       = 275,
    BTN_LEFT        = 276,
    BTN_INSERT      = 277,
    BTN_HOME        = 278,
    BTN_END         = 279,
    BTN_PAGEUP      = 280,
    BTN_PAGEDOWN    = 281,

    /* Function keys */
    BTN_F1          = 282,
    BTN_F2          = 283,
    BTN_F3          = 284,
    BTN_F4          = 285,
    BTN_F5          = 286,
    BTN_F6          = 287,
    BTN_F7          = 288,
    BTN_F8          = 289,
    BTN_F9          = 290,
    BTN_F10         = 291,
    BTN_F11         = 292,
    BTN_F12         = 293,

    /* Key state modifier keys */
    BTN_NUMLOCK     = 300,
    BTN_CAPSLOCK    = 301,
    BTN_SCROLLOCK   = 302,
    BTN_RSHIFT      = 303,
    BTN_LSHIFT      = 304,
    BTN_RCTRL       = 305,
    BTN_LCTRL       = 306,
    BTN_RALT        = 307,
    BTN_LALT        = 308,
    BTN_LWIN        = 311,
    BTN_RWIN        = 312,
    BTN_MODE        = 313,      /* "Alt Gr" key */
    BTN_COMPOSE     = 314,      /* Multi-key compose key */

    /* Miscellaneous function keys */
    BTN_PRINT       = 316,

    /* Joy pad */
    BTN_JOY_FIRST           = 400,
    BTN_JOY1_UP             = BTN_JOY_FIRST,
    BTN_JOY1_DOWN           = 401,
    BTN_JOY1_LEFT           = 402,
    BTN_JOY1_RIGHT          = 403,
    BTN_JOY1_WIIMOTE_A      = 404,
    BTN_JOY1_WIIMOTE_B      = 405,
    BTN_JOY1_WIIMOTE_1      = 406,
    BTN_JOY1_WIIMOTE_2      = 407,
    BTN_JOY1_WIIMOTE_HOME   = 408,
    BTN_JOY1_WIIMOTE_PLUS   = 409,
    BTN_JOY1_WIIMOTE_MINUS  = 410,
    BTN_JOY1_NUNCHUCK_C     = 411,
    BTN_JOY1_NUNCHUCK_Z     = 412,
    BTN_JOY1_CLASSIC_A      = 413,
    BTN_JOY1_CLASSIC_B      = 414,
    BTN_JOY1_CLASSIC_X      = 415,
    BTN_JOY1_CLASSIC_Y      = 416,
    BTN_JOY1_CLASSIC_L      = 417,
    BTN_JOY1_CLASSIC_R      = 418,
    BTN_JOY1_CLASSIC_ZL     = 419,
    BTN_JOY1_CLASSIC_ZR     = 420,
    BTN_JOY1_CLASSIC_HOME   = 421,
    BTN_JOY1_CLASSIC_PLUS   = 422,
    BTN_JOY1_CLASSIC_MINUS  = 423,

    BTN_JOY2_UP             = 430,
    BTN_JOY2_DOWN           = 431,
    BTN_JOY2_LEFT           = 432,
    BTN_JOY2_RIGHT          = 433,
    BTN_JOY2_WIIMOTE_A      = 434,
    BTN_JOY2_WIIMOTE_B      = 435,
    BTN_JOY2_WIIMOTE_1      = 436,
    BTN_JOY2_WIIMOTE_2      = 437,
    BTN_JOY2_WIIMOTE_HOME   = 438,
    BTN_JOY2_WIIMOTE_PLUS   = 439,
    BTN_JOY2_WIIMOTE_MINUS  = 440,
    BTN_JOY2_NUNCHUCK_C     = 441,
    BTN_JOY2_NUNCHUCK_Z     = 442,
    BTN_JOY2_CLASSIC_A      = 443,
    BTN_JOY2_CLASSIC_B      = 444,
    BTN_JOY2_CLASSIC_X      = 445,
    BTN_JOY2_CLASSIC_Y      = 446,
    BTN_JOY2_CLASSIC_L      = 447,
    BTN_JOY2_CLASSIC_R      = 448,
    BTN_JOY2_CLASSIC_ZL     = 449,
    BTN_JOY2_CLASSIC_ZR     = 450,
    BTN_JOY2_CLASSIC_HOME   = 451,
    BTN_JOY2_CLASSIC_PLUS   = 452,
    BTN_JOY2_CLASSIC_MINUS  = 453,

    /* Add any other keys here */
    BTN_LAST
} BTN;

typedef struct {
    BTN key_a;
    BTN key_b;
    int code;
} PADCODE;

typedef enum {
    WPADO_HORIZONTAL,
    WPADO_VERTICAL,
} WPADO;

typedef void (*BTN_CALLBACK)(void*, BTN, int);

//!Drawable objects should derive from this class. Layers can be managed by LayerManagers.
class InputDevices
{
public:
    InputDevices();
    virtual ~InputDevices();

    void ScanButtons(void);
    bool GetWiiMoteIR(int *x, int *y, int *angle);
    void SetWpadOrientation(WPADO orient);
    void GetButtonEvents(BTN_CALLBACK cb, void *context);
    int GetButtonStatus(BTN key);
    int IsUsbKeyboardConnected(void);
#ifdef WII
    u32 GetPadButtons(void);
#endif
    static int CheckButtonName(BTN key, const char *kname);
    static const char *GetButtonName(BTN key);
private:
    void FillKeyNames(void);
#ifdef WII
    u32 GetJoystickDirection(joystick_t *js);
    void ProcessWPadButtons(int channel, PADCODE *wpad);
    u32 GetPadButtonStatus(int channel);
#endif

    CMutex m_mutex;
    static const char *m_keynames[BTN_LAST];
    static const char *m_keynames2[BTN_LAST];
    WPADO m_wpad_orientation;
    int m_leds;
    bool m_btnstatus[2][BTN_LAST];
    int m_connected;
    int m_mouse_x, m_mouse_y, m_mouse_angle;
    bool m_mouse_active;
#ifdef WII
    u32 m_wpad[2];
#endif
};

#endif
