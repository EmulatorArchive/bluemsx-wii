/*
 * GuiBase - Input devices
 */

#include "InputDevices.h"
#include "GameWindow.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#ifdef WII
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include "../iodevice/led.h"
#include "../wiiusbkeyboard/keyboard.h"
#include "../wiiusbkeyboard/wsksymdef.h"
#else
#include <hge/hge.h>
extern HGE *g_hge;
#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifdef WII
#define GetTickCount() ticks_to_millisecs(gettime())
#endif

#define TIME_BEFORE_REPEATING 500
#define TIME_BETWEEN_REPEAT   200

#define KEYIDX_NEW 0
#define KEYIDX_OLD 1

typedef struct {
    BTN key;
    int code_a;
#ifdef WII
    int code_b;
#endif
} BTNCODE;

const char *InputDevices::m_keynames[BTN_LAST];
const char *InputDevices::m_keynames2[BTN_LAST];

#ifdef WII

typedef struct {
    int pad;
    int wpad;
} PAD2WPAD;

static PADCODE wpad_default[] =
{
    {BTN_JOY1_WIIMOTE_A,     BTN_JOY2_WIIMOTE_A,     WPAD_BUTTON_A      },
    {BTN_JOY1_WIIMOTE_B,     BTN_JOY2_WIIMOTE_B,     WPAD_BUTTON_B      },
    {BTN_JOY1_WIIMOTE_1,     BTN_JOY2_WIIMOTE_1,     WPAD_BUTTON_1      },
    {BTN_JOY1_WIIMOTE_2,     BTN_JOY2_WIIMOTE_2,     WPAD_BUTTON_2      },
    {BTN_JOY1_WIIMOTE_HOME,  BTN_JOY2_WIIMOTE_HOME,  WPAD_BUTTON_HOME   },
    {BTN_JOY1_WIIMOTE_PLUS,  BTN_JOY2_WIIMOTE_PLUS,  WPAD_BUTTON_PLUS   },
    {BTN_JOY1_WIIMOTE_MINUS, BTN_JOY2_WIIMOTE_MINUS, WPAD_BUTTON_MINUS  },
    {BTN_NONE, BTN_NONE, 0}
};

static PADCODE wpad_horizontal[] =
{
    {BTN_JOY1_UP,            BTN_JOY2_UP,            WPAD_BUTTON_RIGHT  },
    {BTN_JOY1_DOWN,          BTN_JOY2_DOWN,          WPAD_BUTTON_LEFT   },
    {BTN_JOY1_LEFT,          BTN_JOY2_LEFT,          WPAD_BUTTON_UP     },
    {BTN_JOY1_RIGHT,         BTN_JOY2_RIGHT,         WPAD_BUTTON_DOWN   },
    {BTN_NONE, BTN_NONE, 0}
};

static PADCODE wpad_vertical[] =
{
    {BTN_JOY1_UP,            BTN_JOY2_UP,            WPAD_BUTTON_UP     },
    {BTN_JOY1_DOWN,          BTN_JOY2_DOWN,          WPAD_BUTTON_DOWN   },
    {BTN_JOY1_LEFT,          BTN_JOY2_LEFT,          WPAD_BUTTON_LEFT   },
    {BTN_JOY1_RIGHT,         BTN_JOY2_RIGHT,         WPAD_BUTTON_RIGHT  },
    {BTN_NONE, BTN_NONE, 0}
};

static PADCODE wpad_classic[] =
{
    {BTN_JOY1_UP,            BTN_JOY2_UP,            WPAD_CLASSIC_BUTTON_UP     },
    {BTN_JOY1_DOWN,          BTN_JOY2_DOWN,          WPAD_CLASSIC_BUTTON_DOWN   },
    {BTN_JOY1_LEFT,          BTN_JOY2_LEFT,          WPAD_CLASSIC_BUTTON_LEFT   },
    {BTN_JOY1_RIGHT,         BTN_JOY2_RIGHT,         WPAD_CLASSIC_BUTTON_RIGHT  },
    {BTN_JOY1_CLASSIC_A,     BTN_JOY2_CLASSIC_A,     WPAD_CLASSIC_BUTTON_A      },
    {BTN_JOY1_CLASSIC_B,     BTN_JOY2_CLASSIC_B,     WPAD_CLASSIC_BUTTON_B      },
    {BTN_JOY1_CLASSIC_X,     BTN_JOY2_CLASSIC_X,     WPAD_CLASSIC_BUTTON_X      },
    {BTN_JOY1_CLASSIC_Y,     BTN_JOY2_CLASSIC_Y,     WPAD_CLASSIC_BUTTON_Y      },
    {BTN_JOY1_CLASSIC_L,     BTN_JOY2_CLASSIC_L,     WPAD_CLASSIC_BUTTON_FULL_L },
    {BTN_JOY1_CLASSIC_R,     BTN_JOY2_CLASSIC_R,     WPAD_CLASSIC_BUTTON_FULL_R },
    {BTN_JOY1_CLASSIC_ZL,    BTN_JOY2_CLASSIC_ZL,    WPAD_CLASSIC_BUTTON_ZL     },
    {BTN_JOY1_CLASSIC_ZR,    BTN_JOY2_CLASSIC_ZR,    WPAD_CLASSIC_BUTTON_ZR     },
    {BTN_JOY1_CLASSIC_MINUS, BTN_JOY2_CLASSIC_MINUS, WPAD_CLASSIC_BUTTON_MINUS  },
    {BTN_JOY1_CLASSIC_PLUS,  BTN_JOY2_CLASSIC_PLUS,  WPAD_CLASSIC_BUTTON_PLUS   },
    {BTN_JOY1_CLASSIC_HOME,  BTN_JOY2_CLASSIC_HOME,  WPAD_CLASSIC_BUTTON_HOME   },
    {BTN_NONE, BTN_NONE, 0}
};

static PADCODE wpad_nunchuk[] =
{
    {BTN_JOY1_NUNCHUCK_C,    BTN_JOY2_NUNCHUCK_C,     WPAD_NUNCHUK_BUTTON_C     },
    {BTN_JOY1_NUNCHUCK_Z,    BTN_JOY2_NUNCHUCK_Z,     WPAD_NUNCHUK_BUTTON_Z     },
    {BTN_NONE, BTN_NONE, 0}
};

static PADCODE keypad[] =
{
    {BTN_UP,     BTN_NONE,  WPAD_BUTTON_UP    },
    {BTN_DOWN,   BTN_NONE,  WPAD_BUTTON_DOWN  },
    {BTN_LEFT,   BTN_NONE,  WPAD_BUTTON_LEFT  },
    {BTN_RIGHT,  BTN_NONE,  WPAD_BUTTON_RIGHT },
    {BTN_RETURN, BTN_SPACE, WPAD_BUTTON_A     },
    {BTN_ESCAPE, BTN_NONE,  WPAD_BUTTON_B     },
    {BTN_F12,    BTN_NONE,  WPAD_BUTTON_HOME  },
    {BTN_NONE,   BTN_NONE,  0}
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
#endif

static BTNCODE syms[] =
{
#ifdef WII
    // Function keys
    {BTN_F1,            KS_F1,  KS_f1},
    {BTN_F2,            KS_F2,  KS_f2},
    {BTN_F3,            KS_F3,  KS_f3},
    {BTN_F4,            KS_F4,  KS_f4},
    {BTN_F5,            KS_F5,  KS_f5},
    {BTN_F6,            KS_F6,  KS_f6},
    {BTN_F7,            KS_F7,  KS_f7},
    {BTN_F8,            KS_F8,  KS_f8},
    {BTN_F9,            KS_F9,  KS_f9},
    {BTN_F10,           KS_F10, KS_f10},
    {BTN_F11,           KS_F11, KS_f11},
    {BTN_F12,           KS_F12, KS_f12},

    // ASCII mapped keysyms
    {BTN_1,             KS_1},
    {BTN_2,             KS_2},
    {BTN_3,             KS_3},
    {BTN_4,             KS_4},
    {BTN_5,             KS_5},
    {BTN_6,             KS_6},
    {BTN_7,             KS_7},
    {BTN_8,             KS_8},
    {BTN_9,             KS_9},
    {BTN_0,             KS_0},

    {BTN_EXCLAIM,       KS_exclam},
    {BTN_AT,            KS_at},
    {BTN_HASH,          KS_numbersign},
    {BTN_DOLLAR,        KS_dollar},
    {BTN_PERCENT,       KS_percent},
    {BTN_CARET,         KS_asciicircum},
    {BTN_AMPERSAND,     KS_ampersand},
    {BTN_ASTERISK,      KS_asterisk},
    {BTN_LEFTPAREN,     KS_parenleft},
    {BTN_RIGHTPAREN,    KS_parenright},

    {BTN_A,             KS_A,   KS_a},
    {BTN_B,             KS_B,   KS_b},
    {BTN_C,             KS_C,   KS_c},
    {BTN_D,             KS_D,   KS_d},
    {BTN_E,             KS_E,   KS_e},
    {BTN_F,             KS_F,   KS_f},
    {BTN_G,             KS_G,   KS_g},
    {BTN_H,             KS_H,   KS_h},
    {BTN_I,             KS_I,   KS_i},
    {BTN_J,             KS_J,   KS_j},
    {BTN_K,             KS_K,   KS_k},
    {BTN_L,             KS_L,   KS_l},
    {BTN_M,             KS_M,   KS_m},
    {BTN_N,             KS_N,   KS_n},
    {BTN_O,             KS_O,   KS_o},
    {BTN_P,             KS_P,   KS_p},
    {BTN_Q,             KS_Q,   KS_q},
    {BTN_R,             KS_R,   KS_r},
    {BTN_S,             KS_S,   KS_s},
    {BTN_T,             KS_T,   KS_t},
    {BTN_U,             KS_U,   KS_u},
    {BTN_V,             KS_V,   KS_v},
    {BTN_W,             KS_W,   KS_w},
    {BTN_X,             KS_X,   KS_x},
    {BTN_Y,             KS_Y,   KS_y},
    {BTN_Z,             KS_Z,   KS_z},

    {BTN_MINUS,         KS_minus},
    {BTN_EQUALS,        KS_equal},
    {BTN_BACKSLASH,     KS_backslash},
    {BTN_LEFTBRACKET,   KS_bracketleft},
    {BTN_RIGHTBRACKET,  KS_bracketright},
    {BTN_COLON,         KS_colon},
    {BTN_QUOTE,         KS_apostrophe},
    {BTN_BACKQUOTE,     KS_grave},
    {BTN_COMMA,         KS_comma},
    {BTN_PERIOD,        KS_period},
    {BTN_SLASH,         KS_slash},

    {BTN_UNDERSCORE,    KS_underscore},
    {BTN_PLUS,          KS_plus},
    {BTN_BAR,           KS_bar},
    {BTN_LEFTBRACE,     KS_braceleft},
    {BTN_RIGHTBRACE,    KS_braceright},
    {BTN_SEMICOLON,     KS_semicolon},
    {BTN_QUOTEDBL,      KS_quotedbl},
    {BTN_ASCIITILDE,    KS_asciitilde},
    {BTN_LESS,          KS_less},
    {BTN_GREATER,       KS_greater},
    {BTN_QUESTION,      KS_question},

    {BTN_BACKSPACE,     KS_BackSpace},
    {BTN_TAB,           KS_Tab},
    {BTN_RETURN,        KS_Return},
    {BTN_ESCAPE,        KS_Escape},
    {BTN_SPACE,         KS_space},

    // Numeric keypad
    {BTN_KP0,           KS_KP_0},
    {BTN_KP1,           KS_KP_1},
    {BTN_KP2,           KS_KP_2},
    {BTN_KP3,           KS_KP_3},
    {BTN_KP4,           KS_KP_4},
    {BTN_KP5,           KS_KP_5},
    {BTN_KP6,           KS_KP_6},
    {BTN_KP7,           KS_KP_7},
    {BTN_KP8,           KS_KP_8},
    {BTN_KP9,           KS_KP_9},
    {BTN_KP_PERIOD,     KS_KP_Decimal},
    {BTN_KP_DIVIDE,     KS_KP_Divide},
    {BTN_KP_MULTIPLY,   KS_KP_Multiply},
    {BTN_KP_MINUS,      KS_KP_Subtract},
    {BTN_KP_PLUS,       KS_KP_Add},
    {BTN_KP_ENTER,      KS_KP_Enter},

    /* Arrows + Home/End pad */
    {BTN_UP,            KS_Up,      KS_KP_Up},
    {BTN_DOWN,          KS_Down,    KS_KP_Down},
    {BTN_RIGHT,         KS_Right,   KS_KP_Right},
    {BTN_LEFT,          KS_Left,    KS_KP_Left},
    {BTN_INSERT,        KS_Insert,  KS_KP_Insert},
    {BTN_DELETE,        KS_Delete,  KS_KP_Delete},
    {BTN_HOME,          KS_Home,    KS_KP_Home},
    {BTN_END,           KS_End,     KS_KP_End},
    {BTN_PAGEUP,        KS_Prior,   KS_KP_Prior},
    {BTN_PAGEDOWN,      KS_Next,    KS_KP_Next},

    {BTN_NUMLOCK,       KS_Num_Lock},
    {BTN_CAPSLOCK,      KS_Caps_Lock},
    {BTN_SCROLLOCK,     KS_Hold_Screen},
    {BTN_RSHIFT,        KS_Shift_R},
    {BTN_LSHIFT,        KS_Shift_L},
    {BTN_RCTRL,         KS_Control_R},
    {BTN_LCTRL,         KS_Control_L},
    {BTN_RALT,          KS_Alt_R},
    {BTN_LALT,          KS_Alt_L},
    {BTN_COMPOSE,       KS_Multi_key},      /* Multi-key compose key */

    /* Miscellaneous function keys */
    {BTN_PRINT,         KS_Print_Screen},
#else
    // Function keys
    {BTN_F1,            HGEK_F1},
    {BTN_F2,            HGEK_F2},
    {BTN_F3,            HGEK_F3},
    {BTN_F4,            HGEK_F4},
    {BTN_F5,            HGEK_F5},
    {BTN_F6,            HGEK_F6},
    {BTN_F7,            HGEK_F7},
    {BTN_F8,            HGEK_F8},
    {BTN_F9,            HGEK_F9},
    {BTN_F10,           HGEK_F10},
    {BTN_F11,           HGEK_F11},
    {BTN_F12,           HGEK_F12},

    // ASCII mapped keysyms
    {BTN_1,             HGEK_1},
    {BTN_2,             HGEK_2},
    {BTN_3,             HGEK_3},
    {BTN_4,             HGEK_4},
    {BTN_5,             HGEK_5},
    {BTN_6,             HGEK_6},
    {BTN_7,             HGEK_7},
    {BTN_8,             HGEK_8},
    {BTN_9,             HGEK_9},
    {BTN_0,             HGEK_0},

    {BTN_A,             HGEK_A},
    {BTN_B,             HGEK_B},
    {BTN_C,             HGEK_C},
    {BTN_D,             HGEK_D},
    {BTN_E,             HGEK_E},
    {BTN_F,             HGEK_F},
    {BTN_G,             HGEK_G},
    {BTN_H,             HGEK_H},
    {BTN_I,             HGEK_I},
    {BTN_J,             HGEK_J},
    {BTN_K,             HGEK_K},
    {BTN_L,             HGEK_L},
    {BTN_M,             HGEK_M},
    {BTN_N,             HGEK_N},
    {BTN_O,             HGEK_O},
    {BTN_P,             HGEK_P},
    {BTN_Q,             HGEK_Q},
    {BTN_R,             HGEK_R},
    {BTN_S,             HGEK_S},
    {BTN_T,             HGEK_T},
    {BTN_U,             HGEK_U},
    {BTN_V,             HGEK_V},
    {BTN_W,             HGEK_W},
    {BTN_X,             HGEK_X},
    {BTN_Y,             HGEK_Y},
    {BTN_Z,             HGEK_Z},

    {BTN_MINUS,         HGEK_MINUS},
    {BTN_EQUALS,        HGEK_EQUALS},
    {BTN_BACKSLASH,     HGEK_BACKSLASH},
    {BTN_LEFTBRACKET,   HGEK_LBRACKET},
    {BTN_RIGHTBRACKET,  HGEK_RBRACKET},
    {BTN_BACKQUOTE,     HGEK_GRAVE},
    {BTN_COMMA,         HGEK_COMMA},
    {BTN_PERIOD,        HGEK_PERIOD},
    {BTN_SLASH,         HGEK_SLASH},

    {BTN_SEMICOLON,     HGEK_SEMICOLON},

    {BTN_BACKSPACE,     HGEK_BACKSPACE},
    {BTN_TAB,           HGEK_TAB},
    {BTN_RETURN,        HGEK_ENTER},
    {BTN_ESCAPE,        HGEK_ESCAPE},
    {BTN_SPACE,         HGEK_SPACE},

    // Numeric keypad
    {BTN_KP0,           HGEK_NUMPAD0},
    {BTN_KP1,           HGEK_NUMPAD1},
    {BTN_KP2,           HGEK_NUMPAD2},
    {BTN_KP3,           HGEK_NUMPAD3},
    {BTN_KP4,           HGEK_NUMPAD4},
    {BTN_KP5,           HGEK_NUMPAD5},
    {BTN_KP6,           HGEK_NUMPAD6},
    {BTN_KP7,           HGEK_NUMPAD7},
    {BTN_KP8,           HGEK_NUMPAD8},
    {BTN_KP9,           HGEK_NUMPAD9},

    /* Arrows + Home/End pad */
/*
    {BTN_UP,            HGEK_UP     },
    {BTN_DOWN,          HGEK_DOWN   },
    {BTN_RIGHT,         HGEK_RIGHT  },
    {BTN_LEFT,          HGEK_LEFT   },
*/
    {BTN_INSERT,        HGEK_INSERT },
    {BTN_DELETE,        HGEK_DELETE },
    {BTN_HOME,          HGEK_HOME   },
    {BTN_END,           HGEK_END    },
    {BTN_PAGEUP,        HGEK_PGUP   },
    {BTN_PAGEDOWN,      HGEK_PGDN   },

    {BTN_NUMLOCK,       HGEK_NUMLOCK},
    {BTN_CAPSLOCK,      HGEK_CAPSLOCK},
    {BTN_SCROLLOCK,     HGEK_SCROLLLOCK},
    {BTN_LSHIFT,        HGEK_SHIFT  },
    {BTN_LCTRL,         HGEK_CTRL   },
    {BTN_LALT,          HGEK_ALT    },

    {BTN_JOY1_WIIMOTE_A,     HGEK_Z },
    {BTN_JOY1_WIIMOTE_B,     HGEK_X },
    {BTN_JOY1_WIIMOTE_1,     HGEK_C },
    {BTN_JOY1_WIIMOTE_2,     HGEK_V },
    {BTN_JOY1_WIIMOTE_HOME,  HGEK_HOME },
    {BTN_JOY1_WIIMOTE_PLUS,  HGEK_PGUP },
    {BTN_JOY1_WIIMOTE_MINUS, HGEK_PGDN },
    {BTN_JOY1_UP,            HGEK_UP    },
    {BTN_JOY1_DOWN,          HGEK_DOWN  },
    {BTN_JOY1_LEFT,          HGEK_LEFT  },
    {BTN_JOY1_RIGHT,         HGEK_RIGHT },

    {BTN_JOY1_WIIMOTE_A,     HGEK_LBUTTON },
    {BTN_JOY1_WIIMOTE_B,     HGEK_RBUTTON },
#endif
    {BTN_NONE, 0 }
};

void InputDevices::FillKeyNames(void)
{
    /* Fill in the blanks in m_keynames */
    m_keynames[BTN_BACKSPACE] = "backspace";
    m_keynames[BTN_TAB]       = "tab";
    m_keynames[BTN_RETURN]    = "return";
    m_keynames[BTN_ESCAPE]    = "escape";
    m_keynames[BTN_SPACE]     = "space";
    m_keynames[BTN_EXCLAIM]   = "exclamation";
    m_keynames[BTN_QUOTEDBL]  = "doublequote";
    m_keynames[BTN_HASH]      = "hash";
    m_keynames[BTN_DOLLAR]    = "dollar";
    m_keynames[BTN_PERCENT]   = "percent";
    m_keynames[BTN_AMPERSAND] = "ampersand";
    m_keynames[BTN_QUOTE]     = "quote";
    m_keynames[BTN_LEFTPAREN] = "open_parenthesis";
    m_keynames[BTN_RIGHTPAREN]= "close_parenthesis";
    m_keynames[BTN_ASTERISK]  = "asterisk";
    m_keynames[BTN_PLUS]      = "plus";
    m_keynames[BTN_COMMA]     = "comma";
    m_keynames[BTN_MINUS]     = "minus";
    m_keynames[BTN_PERIOD]    = "period";
    m_keynames[BTN_SLASH]     = "slash";
    m_keynames[BTN_0]         = "key_0";
    m_keynames[BTN_1]         = "key_1";
    m_keynames[BTN_2]         = "key_2";
    m_keynames[BTN_3]         = "key_3";
    m_keynames[BTN_4]         = "key_4";
    m_keynames[BTN_5]         = "key_5";
    m_keynames[BTN_6]         = "key_6";
    m_keynames[BTN_7]         = "key_7";
    m_keynames[BTN_8]         = "key_8";
    m_keynames[BTN_9]         = "key_9";
    m_keynames[BTN_COLON]     = "colon";
    m_keynames[BTN_SEMICOLON] = "semicolon";
    m_keynames[BTN_LESS]      = "less";
    m_keynames[BTN_EQUALS]    = "equal";
    m_keynames[BTN_GREATER]   = "greater";
    m_keynames[BTN_QUESTION]  = "question";
    m_keynames[BTN_AT]        = "at";
    m_keynames[BTN_LEFTBRACKET] = "open_bracket";
    m_keynames[BTN_BACKSLASH] = "backslash";
    m_keynames[BTN_RIGHTBRACKET] = "close_bracket";
    m_keynames[BTN_CARET]     = "caret";
    m_keynames[BTN_UNDERSCORE] = "underscore";
    m_keynames[BTN_BACKQUOTE] = "backquote";
    m_keynames[BTN_A]         = "a";
    m_keynames[BTN_B]         = "b";
    m_keynames[BTN_C]         = "c";
    m_keynames[BTN_D]         = "d";
    m_keynames[BTN_E]         = "e";
    m_keynames[BTN_F]         = "f";
    m_keynames[BTN_G]         = "g";
    m_keynames[BTN_H]         = "h";
    m_keynames[BTN_I]         = "i";
    m_keynames[BTN_J]         = "j";
    m_keynames[BTN_K]         = "k";
    m_keynames[BTN_L]         = "l";
    m_keynames[BTN_M]         = "m";
    m_keynames[BTN_N]         = "n";
    m_keynames[BTN_O]         = "o";
    m_keynames[BTN_P]         = "p";
    m_keynames[BTN_Q]         = "q";
    m_keynames[BTN_R]         = "r";
    m_keynames[BTN_S]         = "s";
    m_keynames[BTN_T]         = "t";
    m_keynames[BTN_U]         = "u";
    m_keynames[BTN_V]         = "v";
    m_keynames[BTN_W]         = "w";
    m_keynames[BTN_X]         = "x";
    m_keynames[BTN_Y]         = "y";
    m_keynames[BTN_Z]         = "z";
    m_keynames[BTN_LEFTBRACE] = "open_brace",
    m_keynames[BTN_BAR]       = "vertical_bar",
    m_keynames[BTN_RIGHTBRACE] = "close_brace",
    m_keynames[BTN_ASCIITILDE] = "tilde",
    m_keynames[BTN_DELETE]    = "delete";

    m_keynames[BTN_KP0]       = "num_0";
    m_keynames[BTN_KP1]       = "num_1";
    m_keynames[BTN_KP2]       = "num_2";
    m_keynames[BTN_KP3]       = "num_3";
    m_keynames[BTN_KP4]       = "num_4";
    m_keynames[BTN_KP5]       = "num_5";
    m_keynames[BTN_KP6]       = "num_6";
    m_keynames[BTN_KP7]       = "num_7";
    m_keynames[BTN_KP8]       = "num_8";
    m_keynames[BTN_KP9]       = "num_9";
    m_keynames[BTN_KP_PERIOD] = "num_period";
    m_keynames[BTN_KP_DIVIDE] = "num_slash";
    m_keynames[BTN_KP_MULTIPLY] = "num_asterisk";
    m_keynames[BTN_KP_MINUS]  = "num_minus";
    m_keynames[BTN_KP_PLUS]   = "num_plus";
    m_keynames[BTN_KP_ENTER]  = "num_enter";
    m_keynames[BTN_KP_EQUALS] = "num_equal";

    m_keynames[BTN_UP]        = "up";
    m_keynames[BTN_DOWN]      = "down";
    m_keynames[BTN_RIGHT]     = "right";
    m_keynames[BTN_LEFT]      = "left";
    m_keynames[BTN_INSERT]    = "insert";
    m_keynames[BTN_HOME]      = "home";
    m_keynames[BTN_END]       = "end";
    m_keynames[BTN_PAGEUP]    = "page_up";
    m_keynames[BTN_PAGEDOWN]  = "page_down";

    m_keynames[BTN_F1]        = "f1";
    m_keynames[BTN_F2]        = "f2";
    m_keynames[BTN_F3]        = "f3";
    m_keynames[BTN_F4]        = "f4";
    m_keynames[BTN_F5]        = "f5";
    m_keynames[BTN_F6]        = "f6";
    m_keynames[BTN_F7]        = "f7";
    m_keynames[BTN_F8]        = "f8";
    m_keynames[BTN_F9]        = "f9";
    m_keynames[BTN_F10]       = "f10";
    m_keynames[BTN_F11]       = "f11";
    m_keynames[BTN_F12]       = "f12";

    m_keynames[BTN_NUMLOCK]   = "numlock";
    m_keynames[BTN_CAPSLOCK]  = "caps_lock";
    m_keynames[BTN_SCROLLOCK] = "scroll_lock";
    m_keynames[BTN_RSHIFT]    = "right_shift";
    m_keynames[BTN_LSHIFT]    = "left_shift";
    m_keynames[BTN_RCTRL]     = "right_ctrl";
    m_keynames[BTN_LCTRL]     = "left_ctrl";
    m_keynames[BTN_RALT]      = "right_alt";
    m_keynames[BTN_LALT]      = "left_alt";
    m_keynames[BTN_LWIN]      = "left_win";
    m_keynames[BTN_RWIN]      = "right_win";
    m_keynames[BTN_MODE]      = "alt_gr";
    m_keynames[BTN_COMPOSE]   = "compose";

    m_keynames[BTN_PRINT]     = "print_screen";

    /* common buttons */
    m_keynames[BTN_JOY1_UP]           = "up1";
    m_keynames[BTN_JOY1_DOWN]         = "down1";
    m_keynames[BTN_JOY1_LEFT]         = "left1";
    m_keynames[BTN_JOY1_RIGHT]        = "right1";
    /* wiimote */
    m_keynames[BTN_JOY1_WIIMOTE_A]    = "wiimoteA1";     m_keynames2[BTN_JOY1_WIIMOTE_A]    = "buttonA1";
    m_keynames[BTN_JOY1_WIIMOTE_B]    = "wiimoteB1";     m_keynames2[BTN_JOY1_WIIMOTE_B]    = "buttonB1";
    m_keynames[BTN_JOY1_WIIMOTE_1]    = "wiimote11";     m_keynames2[BTN_JOY1_WIIMOTE_1]    = "button11";
    m_keynames[BTN_JOY1_WIIMOTE_2]    = "wiimote21";     m_keynames2[BTN_JOY1_WIIMOTE_2]    = "button21";
    m_keynames[BTN_JOY1_WIIMOTE_HOME] = "wiimotehome1";  m_keynames2[BTN_JOY1_WIIMOTE_HOME] = "home1";
    m_keynames[BTN_JOY1_WIIMOTE_PLUS] = "wiimoteplus1";  m_keynames2[BTN_JOY1_WIIMOTE_PLUS] = "plus1";
    m_keynames[BTN_JOY1_WIIMOTE_MINUS]= "wiimoteminus1"; m_keynames2[BTN_JOY1_WIIMOTE_MINUS]= "minus1";
    /* nunchuck */
    m_keynames[BTN_JOY1_NUNCHUCK_C]   = "nunchuckC1";
    m_keynames[BTN_JOY1_NUNCHUCK_Z]   = "nunchuckZ1";
    /* classic */
    m_keynames[BTN_JOY1_CLASSIC_A]    = "classicA1";     m_keynames2[BTN_JOY1_CLASSIC_A]    = "buttonA1";
    m_keynames[BTN_JOY1_CLASSIC_B]    = "classicB1";     m_keynames2[BTN_JOY1_CLASSIC_B]    = "buttonB1";
    m_keynames[BTN_JOY1_CLASSIC_X]    = "classicX1";     m_keynames2[BTN_JOY1_CLASSIC_X]    = "buttonX1";
    m_keynames[BTN_JOY1_CLASSIC_Y]    = "classicY1";     m_keynames2[BTN_JOY1_CLASSIC_Y]    = "buttonY1";
    m_keynames[BTN_JOY1_CLASSIC_L]    = "classicL1";     m_keynames2[BTN_JOY1_CLASSIC_L]    = "buttonL1";
    m_keynames[BTN_JOY1_CLASSIC_R]    = "classicR1";     m_keynames2[BTN_JOY1_CLASSIC_R]    = "buttonR1";
    m_keynames[BTN_JOY1_CLASSIC_ZL]   = "classicZL1";    m_keynames2[BTN_JOY1_CLASSIC_ZL]   = "buttonZL1";
    m_keynames[BTN_JOY1_CLASSIC_ZR]   = "classicZR1";    m_keynames2[BTN_JOY1_CLASSIC_ZR]   = "buttonZR1";
    m_keynames[BTN_JOY1_CLASSIC_HOME] = "classichome1";  m_keynames2[BTN_JOY1_CLASSIC_HOME] = "home1";
    m_keynames[BTN_JOY1_CLASSIC_PLUS] = "classicplus1";  m_keynames2[BTN_JOY1_CLASSIC_PLUS] = "plus1";
    m_keynames[BTN_JOY1_CLASSIC_MINUS]= "classicminus1"; m_keynames2[BTN_JOY1_CLASSIC_MINUS]= "minus1";

    /* common buttons */
    m_keynames[BTN_JOY2_UP]           = "up2";
    m_keynames[BTN_JOY2_DOWN]         = "down2";
    m_keynames[BTN_JOY2_LEFT]         = "left2";
    m_keynames[BTN_JOY2_RIGHT]        = "right2";
    /* wiimote */
    m_keynames[BTN_JOY2_WIIMOTE_A]    = "wiimoteA2";     m_keynames2[BTN_JOY2_WIIMOTE_A]    = "buttonA2";
    m_keynames[BTN_JOY2_WIIMOTE_B]    = "wiimoteB2";     m_keynames2[BTN_JOY2_WIIMOTE_B]    = "buttonB2";
    m_keynames[BTN_JOY2_WIIMOTE_1]    = "wiimote12";     m_keynames2[BTN_JOY2_WIIMOTE_1]    = "button12";
    m_keynames[BTN_JOY2_WIIMOTE_2]    = "wiimote22";     m_keynames2[BTN_JOY2_WIIMOTE_2]    = "button22";
    m_keynames[BTN_JOY2_WIIMOTE_HOME] = "wiimotehome2";  m_keynames2[BTN_JOY2_WIIMOTE_HOME] = "home2";
    m_keynames[BTN_JOY2_WIIMOTE_PLUS] = "wiimoteplus2";  m_keynames2[BTN_JOY2_WIIMOTE_PLUS] = "plus2";
    m_keynames[BTN_JOY2_WIIMOTE_MINUS]= "wiimoteminus2"; m_keynames2[BTN_JOY2_WIIMOTE_MINUS]= "minus2";
    /* nunchuck */
    m_keynames[BTN_JOY2_NUNCHUCK_C]   = "nunchuckC2";
    m_keynames[BTN_JOY2_NUNCHUCK_Z]   = "nunchuckZ2";
    /* classic */
    m_keynames[BTN_JOY2_CLASSIC_A]    = "classicA2";     m_keynames2[BTN_JOY2_CLASSIC_A]    = "buttonA2";
    m_keynames[BTN_JOY2_CLASSIC_B]    = "classicB2";     m_keynames2[BTN_JOY2_CLASSIC_B]    = "buttonB2";
    m_keynames[BTN_JOY2_CLASSIC_X]    = "classicX2";     m_keynames2[BTN_JOY2_CLASSIC_X]    = "buttonX2";
    m_keynames[BTN_JOY2_CLASSIC_Y]    = "classicY2";     m_keynames2[BTN_JOY2_CLASSIC_Y]    = "buttonY2";
    m_keynames[BTN_JOY2_CLASSIC_L]    = "classicL2";     m_keynames2[BTN_JOY2_CLASSIC_L]    = "buttonL2";
    m_keynames[BTN_JOY2_CLASSIC_R]    = "classicR2";     m_keynames2[BTN_JOY2_CLASSIC_R]    = "buttonR2";
    m_keynames[BTN_JOY2_CLASSIC_ZL]   = "classicZL2";    m_keynames2[BTN_JOY2_CLASSIC_ZL]   = "buttonZL2";
    m_keynames[BTN_JOY2_CLASSIC_ZR]   = "classicZR2";    m_keynames2[BTN_JOY2_CLASSIC_ZR]   = "buttonZR2";
    m_keynames[BTN_JOY2_CLASSIC_HOME] = "classichome2";  m_keynames2[BTN_JOY2_CLASSIC_HOME] = "home2";
    m_keynames[BTN_JOY2_CLASSIC_PLUS] = "classicplus2";  m_keynames2[BTN_JOY2_CLASSIC_PLUS] = "plus2";
    m_keynames[BTN_JOY2_CLASSIC_MINUS]= "classicminus2"; m_keynames2[BTN_JOY2_CLASSIC_MINUS]= "minus2";
}

InputDevices::InputDevices()
{
    static int wpad_initialized = 0;

    m_wpad_orientation = WPADO_VERTICAL;

    memset(m_btnstatus, 0, sizeof(m_btnstatus));
    m_leds = 0;
    m_mouse_x = m_mouse_y = m_mouse_angle = 0;
    m_mouse_active = false;
#ifdef WII
    m_wpad[0] = m_wpad[1] = 0;
#endif

    FillKeyNames();

    if( !wpad_initialized ) {
#ifdef WII
        WPAD_Init();
#endif
        wpad_initialized = 1;
    }

#ifdef WII
    m_connected = KEYBOARD_Init(NULL);
#else
    m_connected = 1;
#endif
}

InputDevices::~InputDevices()
{
#ifdef WII
    KEYBOARD_Deinit();
#endif
}

bool InputDevices::GetWiiMoteIR(int *x, int *y, int *angle)
{
    *x = m_mouse_x;
    *y = m_mouse_y;
    *angle = m_mouse_angle;
    return m_mouse_active;
}

void InputDevices::SetWpadOrientation(WPADO orient)
{
    m_wpad_orientation = orient;
}

void InputDevices::ScanButtons(void)
{
    m_mutex.Lock();

#ifdef WII
    keyboard_event kbdEvent;

    // Scan PAD/WPAD
    WPAD_ScanPads();
    PAD_ScanPads();

    // Process WPAD IR
    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);
    if( !ir.state || !ir.smooth_valid ) {
        WPAD_IR(WPAD_CHAN_1, &ir);
    }
    if( !ir.state || !ir.smooth_valid ) {
        m_mouse_active = false;
    }else{
        m_mouse_x = (int)(((ir.sx - (500-200)) * GameWindow::GetWidth()) / 400);
        m_mouse_y = (int)(((ir.sy - (500-150)) * GameWindow::GetHeight()) / 300);
        m_mouse_angle = ir.angle;
        m_mouse_active = true;
    }

    // Process PAD/WPAD buttons
    for(int btn = BTN_JOY_FIRST; btn < BTN_LAST; btn++) {
        m_btnstatus[KEYIDX_NEW][btn] = false;
    }
    PADCODE *p = wpad_default;
    for(int i = 0; i < 2; i++) {
        m_wpad[0] = GetPadButtonStatus(WPAD_CHAN_0);
        m_wpad[1] = GetPadButtonStatus(WPAD_CHAN_1);
        while(p->key_a != BTN_NONE)  {
            if( (m_wpad[0] & p->code) != 0  ) {
                m_btnstatus[KEYIDX_NEW][p->key_a] = true;
            }
            if( (m_wpad[1] & p->code) != 0  ) {
                m_btnstatus[KEYIDX_NEW][p->key_b] = true;
            }
            p++;
        }
        if( m_wpad_orientation == WPADO_HORIZONTAL ) {
            p = wpad_horizontal;
        }else{
            p = wpad_vertical;
        }
    }

    // Handle USB keyboard
    while( KEYBOARD_GetEvent(&kbdEvent) ) {
        switch( kbdEvent.type ) {
            case KEYBOARD_PRESSED:
                for(int i = 0; syms[i].key != BTN_NONE; i++)  {
                    if( kbdEvent.symbol == syms[i].code_a || kbdEvent.symbol == syms[i].code_b ) {
                        m_btnstatus[KEYIDX_NEW][syms[i].key] = true;
                    }
                }
                break;
            case KEYBOARD_RELEASED:
                for(int i = 0; syms[i].key != BTN_NONE; i++)  {
                    if( kbdEvent.symbol == syms[i].code_a || kbdEvent.symbol == syms[i].code_b ) {
                        m_btnstatus[KEYIDX_NEW][syms[i].key] = true;
                    }
                }
                break;
            case KEYBOARD_DISCONNECTED:
                memset(m_btnstatus, 0, sizeof(m_btnstatus));
                m_connected--;
                break;
            case KEYBOARD_CONNECTED:
                m_connected++;
                break;
        }
    }

    // Handle leds
    int led = ledGetCapslock();
    if( m_leds != led ) {
        KEYBOARD_SetLed(KEYBOARD_LEDCAPS, led);
        m_leds = led;
    }
#else
    // Process mouse
    float x, y;
    g_hge->Input_GetMousePos(&x, &y);
    m_mouse_active = g_hge->Input_IsMouseOver();
    m_mouse_x = (int)x;
    m_mouse_y = (int)y;

    // Process key events
    hgeInputEvent event;
    while( g_hge->Input_GetEvent(&event) ) {
      switch( event.type ) {
        case INPUT_KEYDOWN:
          if( event.key == HGEK_F9 ) {
            GameWindow::ToggleFullScreen();
            break;
          }
        case INPUT_MBUTTONDOWN:
          for(int i = 0; syms[i].key != BTN_NONE; i++)  {
            if( syms[i].code_a == event.key ) {
              m_btnstatus[KEYIDX_NEW][syms[i].key] = true;
            }
          }
          break;
        case INPUT_KEYUP:
        case INPUT_MBUTTONUP:
          for(int i = 0; syms[i].key != BTN_NONE; i++)  {
            if( syms[i].code_a == event.key ) {
              m_btnstatus[KEYIDX_NEW][syms[i].key] = false;
            }
          }
          break;
      }
    }
#endif

    m_mutex.Unlock();
}

void InputDevices::GetButtonEvents(BTN_CALLBACK cb, void *context)
{
    int run_callbacks[BTN_LAST];

    m_mutex.Lock();

    // compare with previous and call for each difference
    if( cb ) {
        memset(run_callbacks, 0, sizeof(run_callbacks));
        for(int i = 0; i < BTN_LAST; i++) {
            if( m_btnstatus[KEYIDX_OLD][i] != m_btnstatus[KEYIDX_NEW][i] ) {
                run_callbacks[i] = m_btnstatus[KEYIDX_NEW][i]? 2:1;
            }
        }
    }

    // copy to old
    memcpy(m_btnstatus[KEYIDX_OLD], m_btnstatus[KEYIDX_NEW], sizeof(m_btnstatus[0]));

    m_mutex.Unlock();

    // run callbacks
    if( cb ) {
        for(int i = 0; i < BTN_LAST; i++) {
            if( run_callbacks[i] ) {
                cb(context, (BTN)i, run_callbacks[i] - 1);
            }
        }
    }
}

int InputDevices::GetButtonStatus(BTN key)
{
    assert( key < BTN_LAST );
    return m_btnstatus[KEYIDX_NEW][key];
}

int InputDevices::IsUsbKeyboardConnected(void)
{
    return m_connected;
}

int InputDevices::CheckButtonName(BTN key, const char *kname)
{
    if( key < BTN_LAST &&
        ((m_keynames[key] && strcmp(kname, m_keynames[key])==0) ||
         (m_keynames2[key] && strcmp(kname, m_keynames2[key])==0)) ) {
        return 1;
    }
    return 0;
}

const char *InputDevices::GetButtonName(BTN key)
{
    const char *keyname;

    keyname = NULL;
    if ( key < BTN_LAST ) {
        keyname = m_keynames[key];
    }
    return keyname;
}


#ifdef WII
u32 InputDevices::GetJoystickDirection(joystick_t *js)
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

void InputDevices::ProcessWPadButtons(int channel, PADCODE *wpad)
{
    for(int i = 0; wpad[i].key_a != BTN_NONE; i++)  {
        if( (m_wpad[channel] & wpad[i].code) != 0  ) {
            m_btnstatus[KEYIDX_NEW][wpad[i].key_a] = true;
        }
    }
}

u32 InputDevices::GetPadButtonStatus(int channel)
{
    u32 extensions;
    WPADData data;
    u32 buttons, gcbuttons;
    PAD2WPAD *p2w;
    joystick_t padjoy;

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

    // Key translations for default WiiMote buttons
    ProcessWPadButtons(channel, wpad_default);

    // Check extensions
    WPAD_Probe(channel, &extensions);
    if( extensions == WPAD_EXP_NUNCHUK ) {
      // Nunchuk stick
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.nunchuk.js);
      // Nunchuck key translations
      ProcessWPadButtons(channel, wpad_nunchuk);
    } else if( extensions == WPAD_EXP_CLASSIC ) {
      // Both classic controller sticks
      WPAD_Expansion(channel, &data.exp);
      buttons |= GetJoystickDirection(&data.exp.classic.ljs);
      buttons |= GetJoystickDirection(&data.exp.classic.rjs);
      // Classic controller key translations
      ProcessWPadButtons(channel, wpad_classic);
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

u32 InputDevices::GetPadButtons(void)
{
    int i;
    static u32 prev_buttons = 0;
    static u64 repeat_time = 0;
    u32 buttons;

    buttons = m_wpad[0] | m_wpad[1];

    for( i = 0; keypad[i].code != 0; i++ ) {
        if( GetButtonStatus(keypad[i].key_a) ||
            GetButtonStatus(keypad[i].key_b) ) {
            buttons |= keypad[i].code;
        }
    }

    if( buttons != prev_buttons ) {
        prev_buttons = buttons;
            repeat_time = GetTickCount() + TIME_BEFORE_REPEATING;
            return buttons;
        }
        if( buttons != 0 && GetTickCount() > repeat_time ) {
            repeat_time = GetTickCount() + TIME_BETWEEN_REPEAT;
        return buttons;
    }
    return 0;
}
#endif

