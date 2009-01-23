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
#include "GuiMenu.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

void GuiMenu::InitTitleList(TextRender *fontArial, int fontsize,
                            int x, int y, int sx, int sy, int pitch)
{
    // Arrows
    sprArrowUp.SetImage(g_imgArrow);
    sprArrowUp.SetPosition(x, y + 6);
    sprArrowUp.SetStretchWidth(0.5f);
    sprArrowUp.SetStretchHeight(0.5f);
    sprArrowUp.SetRotation(180.0f/2);
    sprArrowUp.SetVisible(false);
    manager->Insert(&sprArrowUp, 2);

    sprArrowDown.SetImage(g_imgArrow);
    sprArrowDown.SetPosition(x, y+(num_item_rows-1)*pitch + 6);
    sprArrowDown.SetStretchWidth(0.5f);
    sprArrowDown.SetStretchHeight(0.5f);
    sprArrowDown.SetVisible(false);
    manager->Insert(&sprArrowDown, 2);

    // Fill titles
    for(int i = 0; i < num_item_rows; i++) {
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
    current_index = -1;
}

void GuiMenu::RemoveTitleList(void)
{
    manager->Remove(&sprArrowUp);
    manager->Remove(&sprArrowDown);
    for(int i = 0; i < num_item_rows; i++) {
        manager->Remove(&titleTxtSprite[i]);
    }
}

void GuiMenu::ClearTitleList(void)
{
    current_index = -1;
}

void GuiMenu::SetListIndex(int index)
{
    // Claim UI
    archSemaphoreWait(video_semaphore, -1);
    // Update dir info
    for(int i = 0; i < num_item_rows; i++) {
        if( i+index < num_items ) {
            visible_items[i] = item_list[i+index];
        }else{
            visible_items[i] = "";
        }
    }
    // Render text (slow)
    if( index == current_index+1 && current_index != -1 ) {
        DrawableImage *p = titleTxtImgPtr[0];
        for(int i = 0; i < num_item_rows-1; i++) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i+1];
        }
        titleTxtImgPtr[num_item_rows-1] = p;
        titleTxtImgPtr[num_item_rows-1]->RenderText(visible_items[num_item_rows-1]);
    }else
    if( index == current_index-1 ) {
        DrawableImage *p = titleTxtImgPtr[num_item_rows-1];
        for(int i = num_item_rows-1; i > 0; i--) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i-1];
        }
        titleTxtImgPtr[0] = p;
        titleTxtImgPtr[0]->RenderText(visible_items[0]);
    }else
    for(int i = 0; i < num_item_rows; i++) {
        titleTxtImgPtr[i]->RenderText(visible_items[i]);
    }
    current_index = index;
    // Update sprites
    for(int i = 0; i < num_item_rows; i++) {
        titleTxtSprite[i].SetImage(titleTxtImgPtr[i]->GetImage());
    }
    // Up button
    if( index > 0  ) {
        titleTxtSprite[0].SetVisible(false);
        sprArrowUp.SetVisible(true);
        upper_index = 1;
    }else{
        titleTxtSprite[0].SetVisible(true);
        sprArrowUp.SetVisible(false);
        upper_index = 0;
    }
    // Down button
    if( index+num_item_rows < num_items ) {
        titleTxtSprite[num_item_rows-1].SetVisible(false);
        sprArrowDown.SetVisible(true);
        lower_index = num_item_rows-2;
    }else{
        titleTxtSprite[num_item_rows-1].SetVisible(true);
        sprArrowDown.SetVisible(false);
        lower_index = num_item_rows-1;
    }
    // Release UI
    archSemaphoreSignal(video_semaphore);
}

void GuiMenu::SetSelected(int selected)
{
    if( selected >= 0 ) {
        Sprite *selectedsprite = &titleTxtSprite[selected];
        sprSelector.SetPosition(selectedsprite->GetX(),selectedsprite->GetY());
        sprSelector.SetVisible(true);
    }
}

int GuiMenu::DoModal(const char **items, int num, int width)
{
#if RUMBLE
    u64 time2rumble = 0;
    bool rumbeling = false;
#endif
    // Init items
    item_list = items;
    num_items = num;

    // Claim UI
    archSemaphoreWait(video_semaphore, -1);

    // Containers
    int height = num_item_rows*64+32;
    int posx = (320-(width >> 1)) & ~3;
    int posy = (240+37-(height >> 1)) & ~3;
    GuiContainer container(posx, posy, width, height, 192);
    manager->Insert(container.GetLayer(), 2);

    // Selector
    sprSelector.SetImage(g_imgSelector);
    sprSelector.SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprSelector.SetRefPixelPosition(16, 4);
    sprSelector.SetPosition(0, 0);
    sprSelector.SetStretchWidth((float)width / 286);
    sprSelector.SetStretchHeight(1.5f);
    sprSelector.SetVisible(false);
    manager->Insert(&sprSelector, 2);

    // Menu list
    InitTitleList(g_fontArial, 32,
                  posx+32, posy+24, width-32, 48, 64);

    // Cursor
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
    int index = 0;

    // Update title list
    ClearTitleList();
    SetListIndex(index);

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
        for(int i = 0; i < num_item_rows; i++) {
            if( strlen(visible_items[i]) &&
                sprCursor.CollidesWith(&titleTxtSprite[i]) ) {
                cursor_visible = true;
                selected = i;
                archSemaphoreSignal(video_semaphore);
                break;
            }
        }
        if( selected == current ) {
            // Scroll when mouse stays on the arrows for a while
            if( cursor_visible && ticks_to_millisecs(gettime()) > scroll_time ) {
                if( selected == 0 ) {
                    buttons |= WPAD_BUTTON_UP;
                }
                if( selected == num_item_rows-1 ) {
                    buttons |= WPAD_BUTTON_DOWN;
                }
                scroll_time = ticks_to_millisecs(gettime()) + REPEAT_TIME;
            }else{
                if( !(cursor_visible && (selected == 0 || selected == num_item_rows-1)) ) {
                    scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
                }
            }

            // WPAD keys
            if( (buttons & WPAD_BUTTON_LEFT) ||
                (buttons & WPAD_BUTTON_DOWN) ||
                (buttons & WPAD_CLASSIC_BUTTON_DOWN) ) {
                if( current < lower_index &&
                    strlen(visible_items[current+1]) ) {
                    selected++;
                }else{
                    if( index+current < num_items-1 ) {
                        index++;
                        SetListIndex(index);
                        SetSelected(selected);
                    }
                }
            }
            if( (buttons & WPAD_BUTTON_RIGHT) ||
            (buttons & WPAD_BUTTON_UP) ||
            (buttons & WPAD_CLASSIC_BUTTON_UP) ) {
                if( current > upper_index ) {
                    selected--;
                }else{
                    if( index > 0 ) {
                        index--;
                        SetListIndex(index);
                        SetSelected(selected);
                    }
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
            break;
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

GuiMenu::GuiMenu(LayerManager *layman, void *sem, int rows)
{
    manager = layman;
    video_semaphore = sem;
    num_item_rows = rows;
    visible_items = new const char*[rows];
    titleTxtSprite = new Sprite[rows];
    titleTxtImg = new DrawableImage[rows];
    titleTxtImgPtr = new DrawableImage*[rows];
}

GuiMenu::~GuiMenu()
{
    delete[] titleTxtImgPtr;
    delete[] titleTxtImg;
    delete[] titleTxtSprite;
    delete[] visible_items;
}

