#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "GuiStateSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"

extern "C" {
#include "ArchGlob.h"
#include "FileHistory.h"
#include "ZipHelper.h"
}

// Resources
#include "GuiImages.h"

void GuiStateSelect::CreateStateFileList(Properties *properties, char *directory)
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

    num_states = 0;
    if (glob) {
        if (glob->count > 0) {
            for (int i = 0; i < glob->count; i++) {
                struct stat s;
                if( stat(glob->pathVector[i], &s) >= 0 ) {
                    int j, k;
                    time_t t = s.st_mtime;
                    // get date/time string and strip off milliseconds
                    char *str = strdup(ctime(&t));
                    char *p = &str[strlen(str)-1];
                    while( *p != ' ' ) p--;
                    *p = '\0';
                    // to get sorted list, find position where to strore in list
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
                    filenames[k] = strdup(glob->pathVector[i]);
                    num_states++;
                }
            }
        }
        archGlobFree(glob);
    }
}

void GuiStateSelect::FreeStateFileList(void)
{
    for(int i = 0; i < num_states; i++) {
        free(timestrings[i]);
        timestrings[i] = NULL;
        free(filenames[i]);
        filenames[i] = NULL;
    }
}

void GuiStateSelect::UpdateScreenShot(char *file)
{
    sprScreenShot.SetImage(g_imgNoise);
    if( imgScreenShot != NULL ) {
        delete imgScreenShot;
        imgScreenShot = NULL;
    }
    if( file != NULL ) {
        int size;
        void* buffer = zipLoadFile(file, "screenshot.png", &size);

        if( buffer != NULL ) {
            imgScreenShot = new Image;
            if(imgScreenShot->LoadImage((const unsigned char*)buffer) != IMG_LOAD_ERROR_NONE) {
                delete imgScreenShot;
                imgScreenShot = NULL;
            }
            free(buffer);
        }

        Image *img = imgScreenShot;
        if( img == NULL ) {
            img = g_imgNoise;
        }

        sprScreenShot.SetImage(img);
        sprScreenShot.SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprScreenShot.SetRefPixelPosition(0, 0);
        sprScreenShot.SetStretchWidth(256.0f/img->GetWidth());
        sprScreenShot.SetStretchHeight(212.0f/img->GetHeight());
    }
}

void GuiStateSelect::OnSetSelected(int index, int selected)
{
    // Update screenshot
    if( selected >= 0 ) {
        UpdateScreenShot(filenames[index+selected]);
    }
}

char *GuiStateSelect::DoModal(Properties *properties, char *directory)
{
    #define SSEL_YPITCH       51
    #define SSEL_HEIGHT       256
    #define SSEL_MENU_SPACING 8
    #define SSEL_X_SPACING    12
    #define SSEL_LIST_WIDTH   300

    char *returnValue = NULL;

    // Load states
    CreateStateFileList(properties, directory);
    if( num_states == 0 ) {
        return NULL;
    }

    // Claim UI
    manager->Lock();

    // Container
    int posx = 14;
    int posy = 240-(SSEL_HEIGHT/2)-16+37;
    int sizex = 640-28;
    int sizey = SSEL_HEIGHT+32;
    GuiContainer container(posx, posy,
                           sizex, sizey, 160);
	manager->AddTop(container.GetLayer());
    sizex = container.GetWidth();
    sizey = container.GetHeight();

    // Selection
    ShowSelection((const char **)timestrings, num_states, 0, 30, SSEL_YPITCH,
                  posx+SSEL_X_SPACING,
                  posy+sizey/2-(NUM_STATE_ITEMS*SSEL_YPITCH)/2,
                  SSEL_MENU_SPACING, SSEL_LIST_WIDTH);

    // Screen shot
	sprScreenShot.SetPosition(posx+sizex-256-SSEL_X_SPACING-SSEL_MENU_SPACING,
	                          posy+sizey/2-106);
	manager->AddTop(&sprScreenShot);

    // Release UI
    manager->Unlock();

    // Menu loop
    int sel;
    do {
        sel = DoSelection();

        if( sel >= 0 ) {
            returnValue = filenames[sel];
            // confirmation
            char str[256];
            strcpy(str, "Do you want to load\n\"");
            strcat(str, timestrings[sel]);
            strcat(str, "\"");
            GuiMessageBox *msgbox = new GuiMessageBox(manager);
            bool ok = msgbox->Show(str, NULL, true, 192);
            msgbox->Remove();
            delete msgbox;
            if( ok ) {
                break;
            }
        }else{
            returnValue = NULL;
        }
    }while(sel >= 0);

    // Claim UI
    manager->Lock();

    // Remove UI elements
    UpdateScreenShot(NULL);
    RemoveSelection();
	manager->Remove(&sprScreenShot);
	manager->Remove(container.GetLayer());

    // Release UI
    manager->Unlock();

    // Free stuff
    FreeStateFileList();

    return returnValue;
}

GuiStateSelect::GuiStateSelect(GuiManager *man) : GuiSelectionList(man, NUM_STATE_ITEMS)
{
    manager = man;
    imgScreenShot = NULL;
}

GuiStateSelect::~GuiStateSelect()
{
    if( imgScreenShot != NULL ) {
        delete imgScreenShot;
    }
}

