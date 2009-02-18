
#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

#include "InputEvent.h"
#include "GuiKeyboard.h"
#include "WiiInput.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

// Colors                 R            G          B
#define COLOR_HOVER   ((0x08 << 11)+(0x08 << 6)+(0x08))
#define COLOR_PRESSED ((0x00 << 11)+(0x00 << 6)+(0x1f))

typedef struct {
  int key;
  int x;
  int y;
  int sx;
  int sy;
  Sprite **spr;
} KEY_RECT;

static Sprite *spr_pressed;
static Sprite *spr_shift;
static Sprite *spr_ctrl;
static Sprite *spr_code;
static Sprite *spr_graph;

static KEY_RECT key_rects[] = {
    { EC_ESC,     2,    3,   61,  29, NULL },
    { EC_F1,      64,   3,   70,  29, NULL },
    { EC_F2,      135,  3,   70,  29, NULL },
    { EC_F3,      206,  3,   70,  29, NULL },
    { EC_F4,      277,  3,   70,  29, NULL },
    { EC_F5,      348,  3,   70,  29, NULL },
    { EC_SELECT,  419,  3,   70,  29, NULL },
    { EC_STOP,    490,  3,   70,  29, NULL },
    { EC_CLS,     561,  3,   70,  29, NULL },
    { EC_INS,     632,  3,   70,  29, NULL },
    { EC_DEL,     703,  3,   71,  29, NULL },
    { EC_RBRACK,  2,    33,  49,  51, NULL },
    { EC_1,       52,   33,  53,  51, NULL },
    { EC_2,       106,  33,  53,  51, NULL },
    { EC_3,       160,  33,  53,  51, NULL },
    { EC_4,       214,  33,  53,  51, NULL },
    { EC_5,       268,  33,  53,  51, NULL },
    { EC_6,       322,  33,  53,  51, NULL },
    { EC_7,       376,  33,  53,  51, NULL },
    { EC_8,       429,  33,  53,  51, NULL },
    { EC_9,       483,  33,  53,  51, NULL },
    { EC_0,       537,  33,  53,  51, NULL },
    { EC_NEG,     591,  33,  53,  51, NULL },
    { EC_CIRCFLX, 645,  33,  53,  51, NULL },
    { EC_BKSPACE, 699,  33,  75,  51, NULL },
    { EC_TAB,     2,    85,  75,  52, NULL },
    { EC_Q,       78,   85,  53,  52, NULL },
    { EC_W,       132,  85,  52,  52, NULL },
    { EC_E,       186,  85,  52,  52, NULL },
    { EC_R,       239,  85,  53,  52, NULL },
    { EC_T,       293,  85,  53,  52, NULL },
    { EC_Y,       347,  85,  53,  52, NULL },
    { EC_U,       401,  85,  53,  52, NULL },
    { EC_I,       455,  85,  53,  52, NULL },
    { EC_O,       509,  85,  52,  52, NULL },
    { EC_P,       563,  85,  52,  52, NULL },
    { EC_AT,      616,  85,  53,  52, NULL },
    { EC_LBRACK,  670,  85,  53,  52, NULL },
    { EC_BKSLASH, 724,  85,  50,  52, NULL },
    { EC_CTRL,    2,   138,  89,  51, &spr_ctrl },
    { EC_A,       92,  138,  53,  51, NULL },
    { EC_S,       146, 138,  52,  51, NULL },
    { EC_D,       200, 138,  52,  51, NULL },
    { EC_F,       253, 138,  53,  51, NULL },
    { EC_G,       307, 138,  53,  51, NULL },
    { EC_H,       361, 138,  53,  51, NULL },
    { EC_J,       415, 138,  53,  51, NULL },
    { EC_K,       469, 138,  53,  51, NULL },
    { EC_L,       523, 138,  52,  51, NULL },
    { EC_SEMICOL, 577, 138,  52,  51, NULL },
    { EC_COLON,   630, 138,  53,  51, NULL },
    { EC_RETURN,  684, 138,  90,  51, NULL },
    { EC_LSHIFT,  2,   190, 116,  52, &spr_shift },
    { EC_Z,       119, 190,  52,  52, NULL },
    { EC_X,       173, 190,  52,  52, NULL },
    { EC_C,       227, 190,  52,  52, NULL },
    { EC_V,       280, 190,  53,  52, NULL },
    { EC_B,       334, 190,  53,  52, NULL },
    { EC_N,       388, 190,  53,  52, NULL },
    { EC_M,       442, 190,  53,  52, NULL },
    { EC_COMMA,   496, 190,  53,  52, NULL },
    { EC_PERIOD,  550, 190,  52,  52, NULL },
    { EC_DIV,     604, 190,  52,  52, NULL },
    { EC_LSHIFT,  657, 190, 117,  52, &spr_shift },
    { EC_CAPS,    2,   243, 103,  58, NULL },
    { EC_GRAPH,   106, 243,  65,  58, &spr_graph },
    { EC_SPACE,   173, 243, 376,  58, NULL },
    { EC_CODE,    550, 243,  65,  58, &spr_code },
    { EC_UP,      669, 243,  51,  28, NULL },
    { EC_LEFT,    616, 272,  52,  29, NULL },
    { EC_DOWN,    669, 272,  51,  29, NULL },
    { EC_RIGHT,   722, 272,  52,  29, NULL }
};
#define NUM_KEYS (sizeof(key_rects) / sizeof(key_rects[0]))

void GuiKeyboard::RenderWrapper(void *arg)
{
    GuiKeyboard *me = (GuiKeyboard*)arg;
    me->Render();
}

// Function is called every frame
void GuiKeyboard::Render(void)
{
    static KEY_RECT *key_pressed = NULL;

    // WPAD + Infrared
    WPAD_ScanPads();
    u32 buttons = WPAD_ButtonsHeld(WPAD_CHAN_0) | WPAD_ButtonsHeld(WPAD_CHAN_1);
    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);
    if( !ir.state || !ir.smooth_valid ) {
        WPAD_IR(WPAD_CHAN_1, &ir);
    }
    if( ir.state && ir.smooth_valid ) {
        spr_cursor->SetPosition(ir.sx, ir.sy);
        spr_cursor->SetRotation(ir.angle/2);
        spr_cursor->SetVisible(true);
    }else{
        spr_cursor->SetVisible(false);
        spr_cursor->SetPosition(0, 0);
    }

    int x = (int)((ir.sx - xpos) / xscale);
    int y = (int)((ir.sy - ypos) / yscale);

    // Check if above key
    KEY_RECT *key_active = NULL;
    for(unsigned i = 0; i < NUM_KEYS; i++) {
        KEY_RECT *key = &key_rects[i];
        if( x >= key->x && x < key->x + key->sx &&
            y >= key->y && y < key->y + key->sy )
        {
            key_active = key;
            break;
        }
    }
    // Handle keypress
    Sprite *spr = NULL;
    if( key_active && (buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) ) {
        if( key_pressed && !key_pressed->spr && key_pressed != key_active ) {
            inputEventUnset(key_pressed->key);
        }
        if( key_active->spr ) {
            if( key_active != key_pressed ) {
                if( inputEventGetState(key_active->key) ) {
                    inputEventUnset(key_active->key);
                    (*key_active->spr)->SetVisible(false);
                }else{
                    spr = *key_active->spr;
                    inputEventSet(key_active->key);
                }
            }
        }else{
            spr = spr_pressed;
            inputEventSet(key_active->key);
        }
        key_pressed = key_active;
    }else{
        if( key_pressed && !key_pressed->spr ) {
            inputEventUnset(key_pressed->key);
        }
        if( key_active ) {
            spr = spr_hover;
        }
        key_pressed = NULL;
    }
    // Give key a color
    spr_pressed->SetVisible(false);
    spr_hover->SetVisible(false);
    if( spr ) {
        spr->SetStretchWidth(((float)key_active->sx / 4) * xscale);
        spr->SetStretchHeight(((float)key_active->sy / 4) * yscale);
        spr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        spr->SetRefPixelPosition(0,0);
        spr->SetPosition(xscale * key_active->x + xpos, yscale * key_active->y + ypos);
        spr->SetTransparency(128);
        spr->SetVisible(true);
    }
}

void GuiKeyboard::Show(void)
{
    if( is_showing ) {
        return;
    }

    keymap1 = keyboardGetMapping(KEY_JOY1_BUTTON_A);
    keymap2 = keyboardGetMapping(KEY_JOY2_BUTTON_A);
    keyboardRemapKey(KEY_JOY1_BUTTON_A, EC_NONE);
    keyboardRemapKey(KEY_JOY2_BUTTON_A, EC_NONE);

    manager->Lock();

    // Keyboard image
    spr_image = new Sprite;
    spr_image->SetImage(g_imgKeyboard);
    spr_image->SetStretchWidth(xscale);
    spr_image->SetStretchHeight(yscale);
    spr_image->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr_image->SetRefPixelPosition(0,0);
    spr_image->SetPosition(xpos, ypos);
    spr_image->SetTransparency(256-32);
    manager->AddTop(spr_image);

    // Selectors
    manager->AddTop(spr_hover);
    spr_hover->SetVisible(false);
    manager->AddTop(spr_pressed);
    spr_pressed->SetVisible(false);
    manager->AddTop(spr_shift);
    spr_shift->SetVisible(false);
    manager->AddTop(spr_ctrl);
    spr_ctrl->SetVisible(false);
    manager->AddTop(spr_graph);
    spr_graph->SetVisible(false);
    manager->AddTop(spr_code);
    spr_code->SetVisible(false);

    // Cursor
    spr_cursor = new Sprite;
    spr_cursor->SetImage(g_imgMousecursor);
    spr_cursor->SetPosition(400, 500);
    spr_cursor->SetVisible(false);
    manager->AddTop(spr_cursor);
    manager->FixLayers(1);

    manager->AddRenderCallback(RenderWrapper, (void*)this);

    is_showing = true;
    manager->Unlock();

}

void GuiKeyboard::Remove(void)
{
    if( !is_showing ) {
        return;
    }

    manager->RemoveRenderCallback(RenderWrapper, (void*)this);

    manager->Lock();
    manager->UnfixLayers(1);

    manager->Remove(spr_code);
    manager->Remove(spr_graph);
    manager->Remove(spr_ctrl);
    manager->Remove(spr_shift);
    manager->Remove(spr_pressed);
    manager->Remove(spr_hover);
    manager->Remove(spr_cursor);
    manager->Remove(spr_image);
    delete spr_cursor;
    delete spr_image;
    is_showing = false;
    manager->Unlock();

    keyboardRemapKey(KEY_JOY1_BUTTON_A, keymap1);
    keyboardRemapKey(KEY_JOY2_BUTTON_A, keymap2);
}

bool GuiKeyboard::IsShowing(void)
{
    return is_showing;
}

GuiKeyboard::GuiKeyboard(GuiManager *man)
{
    manager = man;
    is_showing = false;
    xpos = 10;
    ypos = 88+37;
    xscale = 0.8f;
    yscale = 1.0f;

    img_hover = new DrawableImage;
    img_hover->CreateImage(4, 4, GX_TF_RGB565);
    u16 *p1 = (u16*)img_hover->GetTextureBuffer();
    img_pressed = new DrawableImage;
    img_pressed->CreateImage(4, 4, GX_TF_RGB565);
    u16 *p2 = (u16*)img_pressed->GetTextureBuffer();
    for(int i = 0; i < 4*4; i++) {
        *p1++ = COLOR_HOVER;
        *p2++ = COLOR_PRESSED;
    }
    img_hover->FlushBuffer();
    img_pressed->FlushBuffer();
    spr_hover = new Sprite;
    spr_hover->SetImage(img_hover->GetImage());
    spr_pressed = new Sprite;
    spr_pressed->SetImage(img_pressed->GetImage());
    spr_shift = new Sprite;
    spr_shift->SetImage(img_pressed->GetImage());
    spr_ctrl = new Sprite;
    spr_ctrl->SetImage(img_pressed->GetImage());
    spr_graph = new Sprite;
    spr_graph->SetImage(img_pressed->GetImage());
    spr_code = new Sprite;
    spr_code->SetImage(img_pressed->GetImage());
}

GuiKeyboard::~GuiKeyboard()
{
    if( is_showing ) {
        Remove();
    }
    delete spr_code;
    delete spr_graph;
    delete spr_ctrl;
    delete spr_shift;
    delete spr_pressed;
    delete spr_hover;
    delete img_pressed;
    delete img_hover;
}

