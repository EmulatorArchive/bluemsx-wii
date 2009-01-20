#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include <fat.h>
#include <wiiuse/wpad.h>

extern "C" {
#include "archEvent.h"
};

#include "kbdlib.h"
#include "GuiHomeMenu.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

const char *GuiHomeMenu::items[] = {
  "Load state",
  "Save state",
  "Properties",
  "Quit"
};

void GuiHomeMenu::InitTitleList(TextRender *fontArial, int fontsize,
                                int x, int y, int sx, int sy, int pitch)
{
    // Fill titles
    for(int i = 0; i < NUM_MENU_ITEMS; i++) {
        titleTxtImgPtr[i] = &titleTxtImg[i];
        titleTxtImg[i].CreateImage(sx, sy);
        titleTxtImg[i].SetFont(fontArial);
        titleTxtImg[i].SetColor((GXColor){255,255,255,255});
        titleTxtImg[i].SetSize(fontsize);
        titleTxtSprite[i].SetImage(titleTxtImg[i].GetImage());
        titleTxtSprite[i].SetPosition(x, y);
        manager->Insert(&titleTxtSprite[i], 2);
        y += pitch;
    }
    // Render text
    for(int i = 0; i < NUM_MENU_ITEMS; i++) {
        titleTxtImgPtr[i]->RenderText(items[i]);
    }
}

void GuiHomeMenu::RemoveTitleList(void)
{
    // Fill titles
    for(int i = 0; i < NUM_MENU_ITEMS; i++) {
        manager->Remove(&titleTxtSprite[i]);
    }
}

void GuiHomeMenu::SetSelected(int selected)
{
    if( selected >= 0 ) {
        Sprite *selectedsprite = &titleTxtSprite[selected];
        sprSelector.SetPosition(selectedsprite->GetX(),selectedsprite->GetY());
        sprSelector.SetVisible(true);
    }
}

int GuiHomeMenu::DoModal(void)
{
#if RUMBLE
    u64 time2rumble = 0;
    bool rumbeling = false;
#endif
    // Claim UI
    archSemaphoreWait(video_semaphore, -1);

    // Containers
    int width = 2*140+64;
    int height = NUM_MENU_ITEMS*64+32;
    int posx = (320-(width >> 1)) & ~3;
    int posy = (240+37-(height >> 1)) & ~3;
    GuiContainer container(posx, posy, width, height, 192);
    manager->Insert(container.GetLayer(), 2);

    // Selector
    sprSelector.SetImage(g_imgSelector);
    sprSelector.SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprSelector.SetRefPixelPosition(16, 4);
    sprSelector.SetPosition(0, 0);
    sprSelector.SetStretchWidth(1.2f);
    sprSelector.SetStretchHeight(1.5f);
    sprSelector.SetVisible(false);
    manager->Insert(&sprSelector, 2);

    // Menu list
    InitTitleList(g_fontArial, 32,
                  posx+32, posy+24, width-64, 48, 64);

    // Cursor (top layer)
    Sprite sprCursor;
    sprCursor.SetImage(g_imgMousecursor);
    sprCursor.SetPosition(400, 500);
    sprCursor.SetVisible(false);
    manager->Insert(&sprCursor, 2);

    // Start displaying
    archSemaphoreSignal(video_semaphore);

    // Start menu
    int selected = 0;
    int current = -1;

    // Menu loop
    u64 scroll_time = 0;
    (void)KBD_GetPadButtons(WPAD_CHAN_0); // flush first
    (void)KBD_GetPadButtons(WPAD_CHAN_1);
    for(;;) {
        WPAD_ScanPads();
        u32 buttons = KBD_GetPadButtons(WPAD_CHAN_0) | KBD_GetPadButtons(WPAD_CHAN_1);

        // Exit on 'home' or 'B'
        if( buttons & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME |
                       WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B | WPAD_BUTTON_1) ) {
            selected = -1;
            break;
        }

        // Claim UI
        archSemaphoreWait(video_semaphore, -1);

        // Infrared
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);
        if( !ir.state || !ir.smooth_valid ) {
            WPAD_IR(WPAD_CHAN_1, &ir);
        }
        if( ir.state && ir.smooth_valid ) {
            sprCursor.SetPosition(ir.sx, ir.sy);
            sprCursor.SetRotation(ir.angle/2);
            sprCursor.SetVisible(true);
        }else{
            sprCursor.SetVisible(false);
            sprCursor.SetPosition(0, 0);
        }

        // Check mouse cursor colisions
        int cursor_visible = false;
        for(int i = 0; i < NUM_MENU_ITEMS; i++) {
            if( sprCursor.CollidesWith(&titleTxtSprite[i]) ) {
                cursor_visible = true;
                selected = i;
                break;
            }
        }
        if( selected == current ) {
            // Scroll when mouse stays on the arrows for a while
            if( cursor_visible && ticks_to_millisecs(gettime()) > scroll_time ) {
                if( selected == 0 ) {
                    buttons |= WPAD_BUTTON_UP;
                }
                if( selected == NUM_MENU_ITEMS-1 ) {
                    buttons |= WPAD_BUTTON_DOWN;
                }
                scroll_time = ticks_to_millisecs(gettime()) + REPEAT_TIME;
            }else{
                if( !(cursor_visible && (selected == 0 || selected == NUM_MENU_ITEMS-1)) ) {
                    scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
                }
            }

            // WPAD keys
            if( (buttons & WPAD_BUTTON_LEFT) ||
                (buttons & WPAD_BUTTON_DOWN) ||
                (buttons & WPAD_CLASSIC_BUTTON_DOWN) ) {
                if( current < NUM_MENU_ITEMS ) {
                    selected++;
                }
            }
            if( (buttons & WPAD_BUTTON_RIGHT) ||
                (buttons & WPAD_BUTTON_UP) ||
                (buttons & WPAD_CLASSIC_BUTTON_UP) ) {
                if( current > 0 ) {
                    selected--;
                }
            }
        }
        if( selected != current ) {
            SetSelected(selected);
#if RUMBLE
            if( selected >= 0 && !rumbeling ) {
                time2rumble = ticks_to_millisecs(gettime());
                WPAD_Rumble(0,1);
                rumbeling = true;
            }
#endif
            current = selected;
            scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
        }

#if RUMBLE
        //stop rumble after 50ms
        if(ticks_to_millisecs(gettime())>time2rumble+50 && rumbeling){ WPAD_Rumble(0,0); }
        //let it rumble again after 250ms
        if(ticks_to_millisecs(gettime())>time2rumble+250 && rumbeling){ rumbeling = false; }
#endif

        // Release UI
        archSemaphoreSignal(video_semaphore);

        if( (buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A | WPAD_BUTTON_2)) &&
            (selected >= 0) ) {
            bool quit = false;
            switch( selected ) {
                case 0: /* Load state */
                    printf("Menu: Load state\n");
                    quit = true;
                    break;
                case 1: /* Save state */
                    printf("Menu: Save state\n");
                    quit = true;
                    break;
                case 2: /* Properties */
                    printf("Menu: Properties\n");
                    break;
                case 3: /* Quit */
                    printf("Menu: Quit\n");
                    quit = true;
                    break;
            }
            if( quit ) {
                break;
            }
        }

        // wait a frame
		VIDEO_WaitVSync();
    }
#if RUMBLE
    if( rumbeling ) {
        WPAD_Rumble(0,0);
    }
#endif
    manager->Remove(&sprCursor);
    RemoveTitleList();
    manager->Remove(&sprSelector);
    manager->Remove(container.GetLayer());
    return selected;
}

GuiHomeMenu::GuiHomeMenu(LayerManager *layman, void *sem)
{
    manager = layman;
    video_semaphore = sem;
}

GuiHomeMenu::~GuiHomeMenu()
{
}

