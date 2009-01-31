#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <fat.h>
#include <wiiuse/wpad.h>

#include "kbdlib.h"
#include "GuiDirSelect.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

void GuiDirSelect::InitTitleList(
                            TextRender *fontArial, int fontsize,
                            int x, int y, int sx, int sy, int pitch)
{
    // Arrows
    sprArrowUp.SetImage(g_imgArrow);
    sprArrowUp.SetPosition(x, y + 6);
    sprArrowUp.SetStretchWidth(0.5f);
    sprArrowUp.SetStretchHeight(0.5f);
    sprArrowUp.SetRotation(180.0f/2);
    sprArrowUp.SetVisible(false);
    manager->AddTop(&sprArrowUp);

    sprArrowDown.SetImage(g_imgArrow);
    sprArrowDown.SetPosition(x, y+(NUM_DIR_ITEMS-1)*pitch + 6);
    sprArrowDown.SetStretchWidth(0.5f);
    sprArrowDown.SetStretchHeight(0.5f);
    sprArrowDown.SetVisible(false);
    manager->AddTop(&sprArrowDown);

    // Fill titles
    for(int i = 0; i < NUM_DIR_ITEMS; i++) {
        titleTxtImgPtr[i] = &titleTxtImg[i];
        titleTxtImg[i].CreateImage(sx, sy);
        titleTxtImg[i].SetFont(fontArial);
        titleTxtImg[i].SetColor((GXColor){255,255,255,255});
        titleTxtImg[i].SetSize(fontsize);
        titleTxtSprite[i].SetImage(titleTxtImg[i].GetImage());
        titleTxtSprite[i].SetPosition(x, y);
        manager->AddTop(&titleTxtSprite[i]);
        y += pitch;
    }
    current_index = -1;
}

void GuiDirSelect::RemoveTitleList(void)
{
    manager->Remove(&sprArrowUp);
    manager->Remove(&sprArrowDown);
    for(int i = 0; i < NUM_DIR_ITEMS; i++) {
        manager->Remove(&titleTxtSprite[i]);
    }
}

void GuiDirSelect::ClearTitleList(void)
{
    current_index = -1;
}

void GuiDirSelect::SetListIndex(int index)
{
    // Update dir info
    for(int i = 0; i < NUM_DIR_ITEMS; i++) {
        dirInfo[i] = dirs.GetDir(i+index);
        if( dirInfo[i] == NULL ) {
            dirInfo[i] = &emptyDir;
        }
    }
    // Render text (slow)
    if( index == current_index+1 && current_index != -1 ) {
        DrawableImage *p = titleTxtImgPtr[0];
        for(int i = 0; i < NUM_DIR_ITEMS-1; i++) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i+1];
        }
        titleTxtImgPtr[NUM_DIR_ITEMS-1] = p;
        titleTxtImgPtr[NUM_DIR_ITEMS-1]->RenderText(dirInfo[NUM_DIR_ITEMS-1]->GetName());
    }else
    if( index == current_index-1 ) {
        DrawableImage *p = titleTxtImgPtr[NUM_DIR_ITEMS-1];
        for(int i = NUM_DIR_ITEMS-1; i > 0; i--) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i-1];
        }
        titleTxtImgPtr[0] = p;
        titleTxtImgPtr[0]->RenderText(dirInfo[0]->GetName());
    }else
    for(int i = 0; i < NUM_DIR_ITEMS; i++) {
        titleTxtImgPtr[i]->RenderText(dirInfo[i]->GetName());
    }
    current_index = index;
    // Update sprites
    for(int i = 0; i < NUM_DIR_ITEMS; i++) {
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
    if( index+NUM_DIR_ITEMS < num_dirs ) {
        titleTxtSprite[NUM_DIR_ITEMS-1].SetVisible(false);
        sprArrowDown.SetVisible(true);
        lower_index = NUM_DIR_ITEMS-2;
    }else{
        titleTxtSprite[NUM_DIR_ITEMS-1].SetVisible(true);
        sprArrowDown.SetVisible(false);
        lower_index = NUM_DIR_ITEMS-1;
    }
}

void GuiDirSelect::SetSelected(int selected)
{
    if( selected >= 0 ) {
        Sprite *selectedsprite = &titleTxtSprite[selected];
        sprSelector.SetPosition(selectedsprite->GetX(),selectedsprite->GetY());
        sprSelector.SetVisible(true);
    }
}

char *GuiDirSelect::DoModal(void)
{
    char *return_value = NULL;
#if RUMBLE
    u64 time2rumble = 0;
    bool rumbeling = false;
#endif
    // Claim UI
    manager->Lock();

    // Containers
    GuiContainer containerDirList(320-140-32, 32, 2*140+64, NUM_DIR_ITEMS*64+32);
    manager->AddTop(containerDirList.GetLayer());

    // Selector
    sprSelector.SetImage(g_imgSelector);
    sprSelector.SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprSelector.SetRefPixelPosition(16, 4);
    sprSelector.SetPosition(0, 0);
    sprSelector.SetStretchWidth(1.2f);
    sprSelector.SetStretchHeight(1.5f);
    sprSelector.SetVisible(false);
    manager->AddTop(&sprSelector);

    // Dir list
    InitTitleList(g_fontArial, 32,
                  320-140, 32+24, 2*140, 48, 64);

    // Cursor
    Sprite sprCursor;
    sprCursor.SetImage(g_imgMousecursor);
    sprCursor.SetPosition(400, 500);
    sprCursor.SetVisible(false);
    manager->AddTop(&sprCursor);

    // Release UI
    manager->Unlock();

    // On re-entry, go back one level if not on root level
    char *prevsel = NULL;
    if( dir_level > 0 ) {
        char *p = current_dir + strlen(current_dir) - 1;
        while(*p != '/') p--;
        *p = '\0';
        prevsel = p + 1;
        dir_level--;
    }

    // Seletion loop
    for(;;) {
        int index = 0;
        int selected = 0;
        int current = -1;
        DirElement *selected_dir = NULL;

        // Load dirs database
        chdir(current_dir);
        dirs.Load(xmlfile);
        num_dirs = dirs.GetNumberOfDirs();
        if( num_dirs == 0 ) {
            return_value = current_dir;
            break;
        }

        // When just gone back one level, find entry comming from
        if( prevsel ) {
            for( int i = 0; i < num_dirs; i++ ) {
                if( strcmp(prevsel, dirs.GetDir(i)->GetDirectory()) == 0 ) {
                    if( i < NUM_DIR_ITEMS-1 ) {
                        selected = i;
                    }else{
                        index = i-1;
                        selected = 1;
                        if( index + NUM_DIR_ITEMS - num_dirs > 0 ) {
                            selected += index + NUM_DIR_ITEMS - num_dirs;
                            index -= index + NUM_DIR_ITEMS - num_dirs;
                        }
                    }
                }
            }
        }

        // Update title list
        ClearTitleList();
        SetListIndex(index);

        // Menu loop
        u64 scroll_time = 0;
        for(;;) {
            WPAD_ScanPads();
            u32 buttons = KBD_GetPadButtons(WPAD_CHAN_0) | KBD_GetPadButtons(WPAD_CHAN_1);

            // Exit on 'home'
            if( buttons & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME) ) {
                dir_level = 0;
                break;
            }
            // Back one level on 'B'
            if( dir_level > 0 && (buttons & (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B | WPAD_BUTTON_1)) ) {
                break;
            }

            // Claim UI
            manager->Lock();

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
            for(int i = 0; i < NUM_DIR_ITEMS; i++) {
                if( dirInfo[i] != &emptyDir &&
                    sprCursor.CollidesWith(&titleTxtSprite[i]) ) {
                    cursor_visible = true;
                    selected = i;
                    manager->Unlock();
                    break;
                }
            }
            if( selected == current ) {
                // Scroll when mouse stays on the arrows for a while
                if( cursor_visible && ticks_to_millisecs(gettime()) > scroll_time ) {
                    if( selected == 0 ) {
                        buttons |= WPAD_BUTTON_UP;
                    }
                    if( selected == NUM_DIR_ITEMS-1 ) {
                        buttons |= WPAD_BUTTON_DOWN;
                    }
                    scroll_time = ticks_to_millisecs(gettime()) + REPEAT_TIME;
                }else{
                    if( !(cursor_visible && (selected == 0 || selected == NUM_DIR_ITEMS-1)) ) {
                        scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
                    }
                }

                // WPAD keys
                if( (buttons & WPAD_BUTTON_LEFT) ||
                    (buttons & WPAD_BUTTON_DOWN) ||
                    (buttons & WPAD_CLASSIC_BUTTON_DOWN) ) {
                    if( current < lower_index &&
                        dirInfo[current+1] != &emptyDir ) {
                        selected++;
                    }else{
                        if( index+current < num_dirs-1 ) {
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

            // Release UI
            manager->Unlock();

#if RUMBLE
            //stop rumble after 50ms
            if(ticks_to_millisecs(gettime())>time2rumble+50 && rumbeling){ WPAD_Rumble(0,0); }
            //let it rumble again after 250ms
            if(ticks_to_millisecs(gettime())>time2rumble+250 && rumbeling){ rumbeling = false; }
#endif
            if( (buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A | WPAD_BUTTON_2)) &&
                (selected >= 0) ) {
                selected_dir = dirInfo[selected];
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

        if( selected_dir != NULL ) {
            // enter directory
            strcat(current_dir, "/");
            strcat(current_dir, selected_dir->GetDirectory());
            dir_level++;
        }else{
            if( dir_level == 0 ) {
                // on root level, leave
                dirs.Clear();
                break;
            }else{
                // go back one level
                char *p = current_dir + strlen(current_dir) - 1;
                while(*p != '/') p--;
                *p = '\0';
                prevsel = p + 1;
                dir_level--;
            }
        }
    }
    // Claim UI
    manager->Lock();

    manager->Remove(&sprCursor);
    RemoveTitleList();
    manager->Remove(&sprSelector);
    manager->Remove(containerDirList.GetLayer());

    // Release UI
    manager->Unlock();

    return return_value;
}

GuiDirSelect::GuiDirSelect(GuiManager *man, const char *startdir, const char *filename)
{
    manager = man;
    current_dir = (char *)malloc(1024);
    strcpy(current_dir, startdir);
    xmlfile = strdup(filename);
    dir_level = 0;
    emptyDir.SetName("");
    emptyDir.SetDirectory("");
}

GuiDirSelect::~GuiDirSelect()
{
    free(xmlfile);
    free(current_dir);
}

