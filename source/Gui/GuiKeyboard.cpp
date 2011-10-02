
#include "GuiKeyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>

#include "../IoDevice/led.h"
#include "../Wii/WiiInput.h"
#include "../input/InputEvent.h"
#include "../GuiBase/GuiRootContainer.h"
#include "../GuiBase/GuiContainer.h"
#include "../GuiBase/GuiSprite.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

// Colors               R     G     B
#define COLOR_HOVER   0x40, 0x40, 0x40
#define COLOR_PRESSED 0x00, 0x00, 0xff

typedef struct {
  int key;
  int x;
  int y;
  int sx;
  int sy;
  bool toggle;
  GuiSprite **spr;
} KEY_RECT;

static GuiSprite *spr_shift_l;
static GuiSprite *spr_shift_r;
static GuiSprite *spr_ctrl;
static GuiSprite *spr_code;
static GuiSprite *spr_graph;
static GuiSprite *spr_caps;

static KEY_RECT key_rects[] = {
    { EC_ESC,     2,    3,   61,  29, false, NULL },
    { EC_F1,      64,   3,   70,  29, false, NULL },
    { EC_F2,      135,  3,   70,  29, false, NULL },
    { EC_F3,      206,  3,   70,  29, false, NULL },
    { EC_F4,      277,  3,   70,  29, false, NULL },
    { EC_F5,      348,  3,   70,  29, false, NULL },
    { EC_SELECT,  419,  3,   70,  29, false, NULL },
    { EC_STOP,    490,  3,   70,  29, false, NULL },
    { EC_CLS,     561,  3,   70,  29, false, NULL },
    { EC_INS,     632,  3,   70,  29, false, NULL },
    { EC_DEL,     703,  3,   71,  29, false, NULL },
    { EC_RBRACK,  2,    33,  49,  51, false, NULL },
    { EC_1,       52,   33,  53,  51, false, NULL },
    { EC_2,       106,  33,  53,  51, false, NULL },
    { EC_3,       160,  33,  53,  51, false, NULL },
    { EC_4,       214,  33,  53,  51, false, NULL },
    { EC_5,       268,  33,  53,  51, false, NULL },
    { EC_6,       322,  33,  53,  51, false, NULL },
    { EC_7,       376,  33,  53,  51, false, NULL },
    { EC_8,       429,  33,  53,  51, false, NULL },
    { EC_9,       483,  33,  53,  51, false, NULL },
    { EC_0,       537,  33,  53,  51, false, NULL },
    { EC_NEG,     591,  33,  53,  51, false, NULL },
    { EC_CIRCFLX, 645,  33,  53,  51, false, NULL },
    { EC_BKSPACE, 699,  33,  75,  51, false, NULL },
    { EC_TAB,     2,    85,  75,  52, false, NULL },
    { EC_Q,       78,   85,  53,  52, false, NULL },
    { EC_W,       132,  85,  52,  52, false, NULL },
    { EC_E,       186,  85,  52,  52, false, NULL },
    { EC_R,       239,  85,  53,  52, false, NULL },
    { EC_T,       293,  85,  53,  52, false, NULL },
    { EC_Y,       347,  85,  53,  52, false, NULL },
    { EC_U,       401,  85,  53,  52, false, NULL },
    { EC_I,       455,  85,  53,  52, false, NULL },
    { EC_O,       509,  85,  52,  52, false, NULL },
    { EC_P,       563,  85,  52,  52, false, NULL },
    { EC_AT,      616,  85,  53,  52, false, NULL },
    { EC_LBRACK,  670,  85,  53,  52, false, NULL },
    { EC_BKSLASH, 724,  85,  50,  52, false, NULL },
    { EC_CTRL,    2,   138,  89,  51, true,  &spr_ctrl },
    { EC_A,       92,  138,  53,  51, false, NULL },
    { EC_S,       146, 138,  52,  51, false, NULL },
    { EC_D,       200, 138,  52,  51, false, NULL },
    { EC_F,       253, 138,  53,  51, false, NULL },
    { EC_G,       307, 138,  53,  51, false, NULL },
    { EC_H,       361, 138,  53,  51, false, NULL },
    { EC_J,       415, 138,  53,  51, false, NULL },
    { EC_K,       469, 138,  53,  51, false, NULL },
    { EC_L,       523, 138,  52,  51, false, NULL },
    { EC_SEMICOL, 577, 138,  52,  51, false, NULL },
    { EC_COLON,   630, 138,  53,  51, false, NULL },
    { EC_RETURN,  684, 138,  90,  51, false, NULL },
    { EC_LSHIFT,  2,   190, 116,  52, true,  &spr_shift_l },
    { EC_Z,       119, 190,  52,  52, false, NULL },
    { EC_X,       173, 190,  52,  52, false, NULL },
    { EC_C,       227, 190,  52,  52, false, NULL },
    { EC_V,       280, 190,  53,  52, false, NULL },
    { EC_B,       334, 190,  53,  52, false, NULL },
    { EC_N,       388, 190,  53,  52, false, NULL },
    { EC_M,       442, 190,  53,  52, false, NULL },
    { EC_COMMA,   496, 190,  53,  52, false, NULL },
    { EC_PERIOD,  550, 190,  52,  52, false, NULL },
    { EC_DIV,     604, 190,  52,  52, false, NULL },
    { EC_RSHIFT,  657, 190, 117,  52, true,  &spr_shift_r },
    { EC_CAPS,    2,   243, 103,  58, false, &spr_caps },
    { EC_GRAPH,   106, 243,  65,  58, true,  &spr_graph },
    { EC_SPACE,   173, 243, 376,  58, false, NULL },
    { EC_CODE,    550, 243,  65,  58, false, &spr_code },
    { EC_UP,      669, 243,  51,  28, false, NULL },
    { EC_LEFT,    616, 272,  52,  29, false, NULL },
    { EC_DOWN,    669, 272,  51,  29, false, NULL },
    { EC_RIGHT,   722, 272,  52,  29, false, NULL }
};

#define NUM_KEYS (sizeof(key_rects) / sizeof(key_rects[0]))

GuiKeyboard::GuiKeyboard(GuiContainer *parent, const char *name) :
             GuiElement(parent, name),
             effectDefault(20, 0, true, 0.0f, false, GetWidth()/2)
{
    is_showing = false;
    is_enabled = true;
    is_hidden = true;
    xscale = 0.8f;
    yscale = 1.0f;
    xsize = (int)g_imgKeyboard->GetWidth();
    ysize = (int)g_imgKeyboard->GetHeight();
    xpos = 10;
    ypos = (int)((GetHeight()-ysize)/2);

    // Cursor
    spr_cursor = new GuiSprite(this, "cursor");
    spr_cursor->SetImage(g_imgMousecursor);
    spr_cursor->SetRefPixelPosition(11, 4);
    spr_cursor->SetPosition(0, 0);

    AddRenderCallback(RenderWrapper, (void*)this);
}

GuiKeyboard::~GuiKeyboard()
{
    RemoveRenderCallback(RenderWrapper, (void*)this);
    RemoveAndDelete(spr_cursor);
    if( is_showing ) {
        Hide();
    }
}

bool GuiKeyboard::RenderWrapper(void *arg)
{
    GuiKeyboard *me = (GuiKeyboard*)arg;
    me->Render();
    return false;
}

// Function is called every frame
void GuiKeyboard::Render(void)
{
    static KEY_RECT *key_pressed = NULL;

    // Infrared
    int x, y, angle;
    bool validpos = GetRootContainer()->gwd.input.GetWiiMoteIR(&x, &y, &angle);
    if( validpos && is_enabled && !is_hidden ) {
        spr_cursor->SetPosition(x, y);
        spr_cursor->SetRotation((float)angle);
        spr_cursor->SetVisible(true);
    }else{
        spr_cursor->SetVisible(false);
        spr_cursor->SetPosition(0, 0);
    }

    x = (int)((x - xpos) / xscale);
    y = (int)((y - ypos) / yscale);

    if( validpos && is_enabled &&
        x >= -40 && x <= xsize+40 &&
        y >= -70 && y <= ysize+60 ) {
        if( !is_showing && !is_hidden ) {
            Show();
        }
    }else{
        is_hidden = false;
        if( is_showing ) {
            Hide();
        }
    }
    if( !validpos || !is_showing ) {
        return;
    }

    spr_hover->SetVisible(false);
    spr_pressed->SetVisible(false);

    // Loop all keys
    for(unsigned i = 0; i < NUM_KEYS; i++) {
        KEY_RECT *key = &key_rects[i];
        GuiSprite *spr = NULL;
        if( key->spr ) {
            spr = *key->spr;
        }
        // Check if above key
        if( x >= key->x && x < key->x + key->sx &&
            y >= key->y && y < key->y + key->sy )
        {
            // update hover sprite
            spr_hover->SetZoomX(((float)key->sx / 4) * xscale);
            spr_hover->SetZoomY(((float)key->sy / 4) * yscale);
            spr_hover->SetRefPixelPosition(0,0);
            spr_hover->SetPosition((int)(xscale * key->x + xpos), (int)(yscale * key->y + ypos));
            spr_hover->SetAlpha(0.5f);
            spr_hover->SetVisible(true);
            // unset previous pressed key
            if( key_pressed && key != key_pressed && !key_pressed->toggle ) {
                inputEventUnset(key_pressed->key);
            }
            // set/unset/toggle current key
            if( GetRootContainer()->gwd.input.GetButtonStatus(BTN_JOY1_WIIMOTE_A) ||
                GetRootContainer()->gwd.input.GetButtonStatus(BTN_JOY2_WIIMOTE_A) ) {
                if( key->toggle ) {
                    if( key != key_pressed ) {
                        if( inputEventGetState(key->key) ) {
                            inputEventUnset(key->key);
                        }else{
                            inputEventSet(key->key);
                        }
                    }
                }else{
                    inputEventSet(key->key);
                }
                if( spr == NULL ) {
                    spr = spr_pressed;
                }
                key_pressed = key;
            }else{
                if( key_pressed && !key_pressed->toggle ) {
                    inputEventUnset(key_pressed->key);
                }
                key_pressed = NULL;
            }
        }
        // set/unset pressed key
        if( spr ) {
            spr->SetZoomX(((float)key->sx / 4) * xscale);
            spr->SetZoomY(((float)key->sy / 4) * yscale);
            spr->SetRefPixelPosition(0,0);
            spr->SetPosition((int)(xscale * key->x + xpos), (int)(yscale * key->y + ypos));
            spr->SetAlpha(0.5f);
            if( key->key == EC_CAPS ) {
                spr->SetVisible(!!ledGetCapslock());
            }else{
                spr->SetVisible(!!inputEventGetState(key->key));
            }
        }
    }
}

void GuiKeyboard::Show(void)
{
    if( is_showing ) {
        return;
    }

    // Keyboard image
    spr_image = new GuiSprite(this, "keyboard");
    spr_image->SetImage(g_imgKeyboard);
    spr_image->SetZoomX(xscale);
    spr_image->SetZoomY(yscale);
    spr_image->SetRefPixelPosition(0,0);
    spr_image->SetPosition(xpos, ypos);
    spr_image->SetAlpha(0.85f);

    spr_hover = new GuiSprite(this, "hover");
    spr_hover->CreateDrawImage(4, 4, GX_TF_RGB565);
    spr_hover->FillSolidColor(COLOR_HOVER);
    spr_pressed = new GuiSprite(this, "pressed");
    spr_pressed->CreateDrawImage(4, 4, GX_TF_RGB565);
    spr_pressed->FillSolidColor(COLOR_PRESSED);
    spr_caps = new GuiSprite(this, "caps");
    spr_caps->SetImage(spr_pressed->GetImage());
    spr_shift_l = new GuiSprite(this, "shift_left");
    spr_shift_l->SetImage(spr_pressed->GetImage());
    spr_shift_r = new GuiSprite(this, "shift_right");
    spr_shift_r->SetImage(spr_pressed->GetImage());
    spr_ctrl = new GuiSprite(this, "ctrl");
    spr_ctrl->SetImage(spr_pressed->GetImage());
    spr_graph = new GuiSprite(this, "graph");
    spr_graph->SetImage(spr_pressed->GetImage());
    spr_code = new GuiSprite(this, "code");
    spr_code->SetImage(spr_pressed->GetImage());

    keymap1 = keyboardGetMapping(BTN_JOY1_WIIMOTE_A);
    keymap2 = keyboardGetMapping(BTN_JOY2_WIIMOTE_A);
    keyboardRemapKey(BTN_JOY1_WIIMOTE_A, EC_NONE);
    keyboardRemapKey(BTN_JOY2_WIIMOTE_A, EC_NONE);

    // Keyboard image
    AddTop(spr_image, effectDefault);

    // Selectors
    AddTop(spr_hover, effectDefault);
    spr_hover->SetVisible(false);
    AddTop(spr_caps, effectDefault);
    spr_caps->SetVisible(false);
    AddTop(spr_pressed, effectDefault);
    spr_pressed->SetVisible(false);
    AddTop(spr_shift_l, effectDefault);
    spr_shift_l->SetVisible(false);
    AddTop(spr_shift_r, effectDefault);
    spr_shift_r->SetVisible(false);
    AddTop(spr_ctrl, effectDefault);
    spr_ctrl->SetVisible(false);
    AddTop(spr_graph, effectDefault);
    spr_graph->SetVisible(false);
    AddTop(spr_code, effectDefault);
    spr_code->SetVisible(false);

    // Cursor
    spr_cursor->SetVisible(false);
    AddTopFixed(spr_cursor);

    is_showing = true;
}

void GuiKeyboard::Hide(void)
{
    if( !is_showing ) {
        return;
    }

    Remove(spr_cursor);
    RemoveAndDelete(spr_code, effectDefault);
    RemoveAndDelete(spr_graph, effectDefault);
    RemoveAndDelete(spr_ctrl, effectDefault);
    RemoveAndDelete(spr_shift_l, effectDefault);
    RemoveAndDelete(spr_shift_r, effectDefault);
    RemoveAndDelete(spr_caps, effectDefault);
    RemoveAndDelete(spr_pressed, effectDefault);
    RemoveAndDelete(spr_hover, effectDefault);
    RemoveAndDelete(spr_image, effectDefault);
    is_showing = false;

    keyboardRemapKey(BTN_JOY1_WIIMOTE_A, keymap1);
    keyboardRemapKey(BTN_JOY2_WIIMOTE_A, keymap2);
}

bool GuiKeyboard::IsShowing(void)
{
    return is_showing;
}

void GuiKeyboard::SetEnabled(bool enable)
{
    is_enabled = is_hidden = enable;
}

