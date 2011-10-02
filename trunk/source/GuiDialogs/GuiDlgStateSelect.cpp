#include "GuiDlgStateSelect.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../Arch/ArchGlob.h"
#include "../Emulator/FileHistory.h"
#include "../Utils/ZipHelper.h"

#include "GuiDlgMessageBox.h"
#include "../Gui/GuiImages.h"
#include "../GuiBase/GuiEffectFade.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmSelectionList.h"


#define SSEL_FADE_FRAMES  10
#define SSEL_FADE_DELAY   8

#define SSEL_YPITCH       51
#define SSEL_HEIGHT       256
#define SSEL_MENU_SPACING 8
#define SSEL_X_SPACING    12
#define SSEL_LIST_WIDTH   280

void GuiDlgStateSelect::CreateStateFileList(Properties *properties, char *directory)
{
    ArchGlob* glob;
    static char filename[512];
    char baseName[128];
    int numMod = 1;
    char filenameFormat[32] = "%s/%s%s_";
    char destfileFormat[32];
    int digits = 2;
    const char *prefix = "";
    const char *extension = ".sta";

    for (int i = 0; i < digits; i++) {
        strcat(filenameFormat, "?");
        numMod *= 10;
    }
    strcat(filenameFormat, "%s");
    sprintf(destfileFormat, "%%s/%%s%%s_%%0%di%%s", digits);

    createSaveFileBaseName(baseName, properties, 0);

    sprintf(filename, filenameFormat, directory, prefix, baseName, extension);

    glob = archGlob(filename, ARCH_GLOB_FILES);

    // Free old list if exists
    FreeStateFileList();

    if (glob) {
        if (glob->count > 0) {
            for (int i = 0; i < glob->count; i++) {
                struct stat s;
                if( stat(glob->pathVector[i], &s) >= 0 ) {
                    int j, k;
                    time_t t = s.st_mtime;
                    // libogc bug: year = 1929
                    struct tm * timeinfo;
                    timeinfo = localtime(&t);
#ifdef UNDER_CE
                    char *str = (char*)malloc(80);
                    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d",
                                 timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
                                 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
                    timeinfo->tm_year += 80;
                    t = mktime(timeinfo);
                    // get date/time string and strip off milliseconds
                    char *str = _strdup(ctime(&t));
#endif
                    char *p = &str[strlen(str)-1];
                    while( *p != ' ' ) p--;
                    *p = '\0';
                    // to get sorted list, find position where to store in list
                    for(k = 0; k < num_states && filetimes[k] > t; k++);
                    // shift all entries behind
                    for(j = num_states; j > k; j--) {
                        filetimes[j] = filetimes[j-1];
                        timestrings[j] = timestrings[j-1];
                        filenames[j] = filenames[j-1];
                    }
                    // store in list
                    filetimes[k] = t;
                    timestrings[k] = str;
                    filenames[k] = _strdup(glob->pathVector[i]);
                    num_states++;
                }
            }
        }
        archGlobFree(glob);
    }
}

void GuiDlgStateSelect::FreeStateFileList(void)
{
    for(int i = 0; i < num_states; i++) {
        free(timestrings[i]);
        timestrings[i] = NULL;
        free(filenames[i]);
        filenames[i] = NULL;
    }
    num_states = 0;
}

void GuiDlgStateSelect::UpdateScreenShot(char *file)
{
    if( sprScreenShot != NULL ) {
        RemoveAndDelete(sprScreenShot, GuiEffectFade(SSEL_FADE_FRAMES, file? SSEL_FADE_DELAY:0));
        sprScreenShot = NULL;
    }
    if( file != NULL ) {
        int size;
        void* buffer = zipLoadFile(file, "screenshot.png", &size);

        sprScreenShot = new GuiSprite(this, "screenshot");
        if( buffer == NULL || !sprScreenShot->LoadImage((const unsigned char*)buffer) ) {
            sprScreenShot->SetImage(g_imgNoise);
        }
        if( buffer != NULL ) {
            free(buffer);
        }
        sprScreenShot->SetPosition(posx+sizex-283-SSEL_X_SPACING-2*SSEL_MENU_SPACING,
                                   posy+sizey/2-106);
        sprScreenShot->SetRefPixelPosition(0, 0);
        sprScreenShot->SetScaledWidth(283.0f);
        sprScreenShot->SetScaledHeight(212.0f);
        AddTop(sprScreenShot, GuiEffectFade(SSEL_FADE_FRAMES));
    }
}

void GuiDlgStateSelect::SetSelectedState(int index, int selected)
{
    // Update screenshot
    if( selected >= 0 ) {
        UpdateScreenShot(filenames[index+selected]);
    }
    last_index = index;
    last_selected = selected;
}

void GuiDlgStateSelect::OnUpdateScreen(void)
{
    int sel = list->GetSelectedItem();
    if( sel >= 0 && sel != last_selected ) {
        SetSelectedState(last_index, sel);
    }
}

char *GuiDlgStateSelect::DoModal(void)
{
    char *returnValue = NULL;

    if( num_states == 0 ) {
        return NULL;
    }

    // Menu loop
    int sel;
    do {
        // Run GUI
        sel = -1;
        if( Run() ) {
            sel = list->GetSelectedItem();
            returnValue = filenames[sel];
            // confirmation
            bool ok = GuiDlgMessageBox::ShowModal(this, "wantload",
                                                  MSGT_YESNO, NULL, 0.75f,
                                                  GuiEffectFade(10), GuiEffectFade(10),
                                                  "Do you want to load\n\"%s\"", timestrings[sel]) == MSGBTN_YES;
            if( ok ) {
                break;
            }
        }else{
            returnValue = NULL;
        }
    }while(sel >= 0);

    return returnValue;
}

GuiDlgStateSelect::GuiDlgStateSelect(GuiContainer *parent, const char *name, Properties *properties, char *directory)
                  :GuiDialog(parent, name)
{
    list = new GuiElmSelectionList(this, name, NUM_STATE_ITEMS);
    num_states = 0;
    sprScreenShot = NULL;
    last_selected = -1;
    last_index = 0;

    // Load states
    CreateStateFileList(properties, directory);

    // Frame
    posx = 14;
    posy = GetHeight()/2-(SSEL_HEIGHT/2)-16;
    sizex = 640-28;
    sizey = SSEL_HEIGHT+32;
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, posx, posy, sizex, sizey, 0.6f);
    AddTop(frame);

    // Selection
    GXColor white = {255, 255, 255, 255};
    list->InitSelection(new GuiElmListLineDefault(this, "defaultline", white, 26, SSEL_MENU_SPACING, false),
                        (void**)timestrings, num_states, 0, SSEL_YPITCH,
                        posx+SSEL_X_SPACING,
                        posy+sizey/2-(NUM_STATE_ITEMS*SSEL_YPITCH)/2,
                        SSEL_LIST_WIDTH);
    AddTop(list);
    list->SetFocus(true);
}

GuiDlgStateSelect::~GuiDlgStateSelect()
{
    // Remove UI elements
    if( sprScreenShot != NULL ) {
        RemoveAndDelete(sprScreenShot);
        sprScreenShot = NULL;
    }

    RemoveAndDelete(list);
    RemoveAndDelete(frame);

    // Free stuff
    FreeStateFileList();
}

