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
#include "GuiGameSelect.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

void GuiGameSelect::InitTitleList(TextRender *fontArial, int fontsize,
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
	sprArrowDown.SetPosition(x, y+(NUM_LIST_ITEMS-1)*pitch + 6);
    sprArrowDown.SetStretchWidth(0.5f);
    sprArrowDown.SetStretchHeight(0.5f);
    sprArrowDown.SetVisible(false);
	manager->Insert(&sprArrowDown, 2);

    // Fill titles
    for(int i = 0; i < NUM_LIST_ITEMS; i++) {
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

void GuiGameSelect::RemoveTitleList(void)
{
    manager->Remove(&sprArrowUp);
    manager->Remove(&sprArrowDown);
    for(int i = 0; i < NUM_LIST_ITEMS; i++) {
        manager->Remove(&titleTxtSprite[i]);
    }
}

void GuiGameSelect::SetScreenShotImage(int index, Image *img)
{
    if( img == NULL ) {
        img = g_imgNoise;
    }
    Sprite *spr = &sprScreenShot[index];
    spr->SetImage(img);
    spr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr->SetRefPixelPosition(0, 0);
    spr->SetStretchWidth(252.0f/img->GetWidth());
    spr->SetStretchHeight(180.0f/img->GetHeight());
}

void GuiGameSelect::SetSelected(int index, int selected)
{
    // Update game info
    for(int i = 0; i < NUM_LIST_ITEMS; i++) {
        gameInfo[i] = games.GetGame(i+index);
        if( gameInfo[i] == NULL ) {
            gameInfo[i] = &emptyGame;
        }
    }
    // Render text (slow)
    if( index == current_index+1 && current_index != -1 ) {
        DrawableImage *p = titleTxtImgPtr[0];
        for(int i = 0; i < NUM_LIST_ITEMS-1; i++) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i+1];
        }
        titleTxtImgPtr[NUM_LIST_ITEMS-1] = p;
        titleTxtImgPtr[NUM_LIST_ITEMS-1]->RenderText(gameInfo[NUM_LIST_ITEMS-1]->GetName());
    }else
    if( index == current_index-1 ) {
        DrawableImage *p = titleTxtImgPtr[NUM_LIST_ITEMS-1];
        for(int i = NUM_LIST_ITEMS-1; i > 0; i--) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i-1];
        }
        titleTxtImgPtr[0] = p;
        titleTxtImgPtr[0]->RenderText(gameInfo[0]->GetName());
    }else
    for(int i = 0; i < NUM_LIST_ITEMS; i++) {
        titleTxtImgPtr[i]->RenderText(gameInfo[i]->GetName());
    }
    current_index = index;
    // Update sprites
    for(int i = 0; i < NUM_LIST_ITEMS; i++) {
        titleTxtSprite[i].SetImage(titleTxtImgPtr[i]->GetImage());
    }
    // Up button
    if( index > 0 ) {
        titleTxtSprite[0].SetVisible(false);
        sprArrowUp.SetVisible(true);
        upper_index = 1;
    }else{
        titleTxtSprite[0].SetVisible(true);
        sprArrowUp.SetVisible(false);
        upper_index = 0;
    }
    // Down button
    if( index+NUM_LIST_ITEMS < num_games ) {
        titleTxtSprite[NUM_LIST_ITEMS-1].SetVisible(false);
        sprArrowDown.SetVisible(true);
        lower_index = NUM_LIST_ITEMS-2;
    }else{
        titleTxtSprite[NUM_LIST_ITEMS-1].SetVisible(true);
        sprArrowDown.SetVisible(false);
        lower_index = NUM_LIST_ITEMS-1;
    }
    // Update screenshots
    if( selected >= 0 ) {
        Sprite *selectedsprite = &titleTxtSprite[selected];
        sprSelector.SetPosition(selectedsprite->GetX(),selectedsprite->GetY());
        sprSelector.SetVisible(true);
        SetScreenShotImage(0, gameInfo[selected]->GetImage(0));
        SetScreenShotImage(1, gameInfo[selected]->GetImage(1));
    }
    // Free images of games that are not on the screen
    for(int i = 0; i < games.GetNumberOfGames(); i++) {
        GameElement *game = games.GetGame(i);
        if( game != NULL && (i < index || i >= (index + NUM_LIST_ITEMS)) ) {
            game->FreeImage(0);
            game->FreeImage(1);
        }
    }
}

bool GuiGameSelect::DoModal(const char *dir, const char *filename, GameElement *game)
{
    GameElement *returnValue = NULL;
#if RUMBLE
	u64 time2rumble = 0;
	bool rumbeling = false;
#endif
    int index = 0;
	int selected = 0;
	int current = -1;

    // Load games database
    chdir(dir);
    games.Load(filename);
    num_games = games.GetNumberOfGames();
    if( num_games == 0 ) {
        return false;
    }

    // Claim UI
    archSemaphoreWait(video_semaphore, -1);

    // Containers
    GuiContainer grWinList(32-8, 24, 288, 420+12);
	manager->Insert(grWinList.GetLayer(), 2);
    GuiContainer grWinTitle(344-8, 24, 264+12, 204);
	manager->Insert(grWinTitle.GetLayer(), 2);
    GuiContainer grWinPlay(344-8, 240+12, 264+12, 204);
	manager->Insert(grWinPlay.GetLayer(), 2);

    // Screen shots (240x186)
	sprScreenShot[0].SetPosition(344+12-8, 24+12);
	sprScreenShot[1].SetPosition(344+12-8, 240+12+12);
	manager->Insert(&sprScreenShot[0], 2);
	manager->Insert(&sprScreenShot[1], 2);

    // Selector
	sprSelector.SetImage(g_imgSelector);
    sprSelector.SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprSelector.SetRefPixelPosition(4, 0);
	sprSelector.SetPosition(0, 0);
    sprSelector.SetVisible(false);
	manager->Insert(&sprSelector, 2);

    // Title list
    InitTitleList(g_fontArial, 24,
                  36, 32, 264+12, 36, 34);
    SetSelected(0, 0);

    // Cursor
    Sprite sprCursor;
	sprCursor.SetImage(g_imgMousecursor);
	sprCursor.SetPosition(400, 500);
    sprCursor.SetVisible(false);
	manager->Insert(&sprCursor, 2);

    // Release UI
    archSemaphoreSignal(video_semaphore);

    // Menu loop
    u64 scroll_time = 0;
	for(;;){
		WPAD_ScanPads();
        u32 buttons = KBD_GetPadButtons(WPAD_CHAN_0) | KBD_GetPadButtons(WPAD_CHAN_1);

        // Break-out on 'home' or 'B'
		if( buttons & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME |
                       WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B | WPAD_BUTTON_1) ) {
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
        for(int i = 0; i < NUM_LIST_ITEMS; i++) {
            if( gameInfo[i] != &emptyGame &&
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
                if( selected == NUM_LIST_ITEMS-1 ) {
                    buttons |= WPAD_BUTTON_DOWN;
                }
                scroll_time = ticks_to_millisecs(gettime()) + REPEAT_TIME;
            }else{
                if( !(cursor_visible && (selected == 0 || selected == NUM_LIST_ITEMS-1)) ) {
                    scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
                }
            }

            // WPAD keys
            if( (buttons & WPAD_BUTTON_LEFT) ||
                (buttons & WPAD_BUTTON_DOWN) ||
                (buttons & WPAD_CLASSIC_BUTTON_DOWN) ) {
                if( current < lower_index &&
                    gameInfo[current+1] != &emptyGame ) {
                    selected++;
                }else{
                    if( index+current < num_games-1 ) {
                        index++;
                        SetSelected(index, selected);
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
                        SetSelected(index, selected);
                    }
                }
            }
		}
		if( selected != current ) {
            SetSelected(index, selected);
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

        // Release UI
        archSemaphoreSignal(video_semaphore);

#if RUMBLE
		//stop rumble after 50ms
		if(ticks_to_millisecs(gettime())>time2rumble+50 && rumbeling){ WPAD_Rumble(0,0); }
		//let it rumble again after 250ms
		if(ticks_to_millisecs(gettime())>time2rumble+250 && rumbeling){ rumbeling = false; }
#endif
        if( (buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A | WPAD_BUTTON_2)) &&
            (selected >= 0) ) {
            returnValue = gameInfo[selected];
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

    // Claim UI
    archSemaphoreWait(video_semaphore, -1);

    manager->Remove(&sprCursor);
    RemoveTitleList();
    manager->Remove(&sprSelector);
	manager->Remove(&sprScreenShot[0]);
	manager->Remove(&sprScreenShot[1]);
	manager->Remove(grWinList.GetLayer());
	manager->Remove(grWinTitle.GetLayer());
	manager->Remove(grWinPlay.GetLayer());

    // Release UI
    archSemaphoreSignal(video_semaphore);

    if( returnValue != NULL ) {
        game->SetName(returnValue->GetName());
        game->SetCommandLine(returnValue->GetCommandLine());
        game->SetScreenShot(0, returnValue->GetScreenShot(0));
        game->SetScreenShot(1, returnValue->GetScreenShot(1));
        games.Clear();
        return true;
    }else{
        games.Clear();
        return false;
    }
}

GuiGameSelect::GuiGameSelect(LayerManager *layman, void *sem)
{
    manager = layman;
    video_semaphore = sem;
    emptyGame.SetName("");
    emptyGame.SetCommandLine("");
}

GuiGameSelect::~GuiGameSelect()
{
}

