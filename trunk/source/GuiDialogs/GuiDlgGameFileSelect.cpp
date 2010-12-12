#include "GuiDlgGameFileSelect.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiDialogs/GuiDlgMessageBox.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmSelectionList.h"

#include "../Arch/ArchGlob.h"
//#include "../Utils/ZipHelper.h"

// Resources
#include "../Gui/GuiImages.h"

#define GFSEL_FADE_FRAMES     10

#define GFSEL_SELECTION_LINES 10
#define GFSEL_FONTSIZE        20
#define GFSEL_YPITCH          30
#define GFSEL_HEIGHT          300
#define GFSEL_MENU_SPACING    8
#define GFSEL_X_SPACING       12
#define GFSEL_LIST_WIDTH      300

void GuiDlgGameFileSelect::CreateFileList(char *directory)
{
    char searchpath[256];
    ArchGlob* glob;

    // Free old list if exists
    FreeFileList();

    // Construct search path
    sprintf(searchpath, "%s/*", directory);

    // Process directories
    glob = archGlob(searchpath, ARCH_GLOB_DIRS);
    if (glob) {
        for (int i = 0; i < glob->count; i++) {
            char path[256];
            strcpy(path, glob->pathVector[i]);
            struct stat s;
            if( stat(path, &s) >= 0 ) {
                int j, k;
                // Find filename
                char *p, *filename = path;
                while( (p = strstr(filename, "/")) || (p = strstr(filename, "\\")) ) {
                    filename = p+1;
                }
                // Its a directory
                strcat(filename, "/");
                // to get sorted list, find position where to strore in list
                for(k = 0; k < num_files && stricmp(filename, filenames[k]) > 0; k++);
                // shift all entries behind
                for(j = num_files; j > k; j--) {
                    filenames[j] = filenames[j-1];
                }
                // store in list
                filenames[k] = strdup(filename);
                num_files++;
            }
        }
        archGlobFree(glob);
    }

    // Process files
    int first_index = num_files;
    glob = archGlob(searchpath, ARCH_GLOB_FILES);
    if (glob) {
        for (int i = 0; i < glob->count; i++) {
            char path[256];
            strcpy(path, glob->pathVector[i]);
            struct stat s;
            if( stat(path, &s) >= 0 ) {
                int j, k;
                // Find filename
                char *p, *filename = path;
                while( (p = strstr(filename, "/")) || (p = strstr(filename, "\\")) ) {
                    filename = p+1;
                }
                // to get sorted list, find position where to strore in list
                for(k = first_index; k < num_files && stricmp(filename, filenames[k]) > 0; k++);
                // shift all entries behind
                for(j = num_files; j > k; j--) {
                    filenames[j] = filenames[j-1];
                }
                // store in list
                filenames[k] = strdup(filename);
                num_files++;
            }
        }
        archGlobFree(glob);
    }

}

void GuiDlgGameFileSelect::FreeFileList(void)
{
    for(int i = 0; i < num_files; i++) {
        free(filenames[i]);
        filenames[i] = NULL;
    }
    num_files = 0;
}

int GuiDlgGameFileSelect::Create(char *directory)
{
    // Load files
    CreateFileList(directory);
    if( num_files == 0 ) {
        return 0;
    }

    // Container
    posx = 14;
    posy = GetHeight()/2-(GFSEL_HEIGHT/2)-16;
    sizex = 640-28;
    sizey = GFSEL_HEIGHT+32;
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_GRAY, posx, posy, sizex, sizey, 240);
    AddTop(frame);

    // Selection
    GXColor black = {0, 0, 0, 255};
    list->InitSelection((const char **)filenames, num_files, 0, GFSEL_FONTSIZE, black, GFSEL_YPITCH,
                        posx+GFSEL_X_SPACING,
                        posy+sizey/2-(GFSEL_SELECTION_LINES*GFSEL_YPITCH)/2,
                        GFSEL_MENU_SPACING, GFSEL_LIST_WIDTH, false);
    AddTop(list);

    return num_files;
}

char* GuiDlgGameFileSelect::DoModal(void)
{
    char *returnValue = NULL;

    // Menu loop
    int sel;
    do {
        // Run GUI
        sel = -1;
        if( Run() ) {
            sel = list->GetSelectedItem();
            returnValue = filenames[sel];
            // confirmation
            bool ok = GuiDlgMessageBox::ShowModal(this, "wantadd",
                                                  MSGT_YESNO, NULL, 192,
                                                  new GuiEffectFade(GFSEL_FADE_FRAMES), new GuiEffectFade(GFSEL_FADE_FRAMES),
                                                  "Do you want to add\n\"%s\"", filenames[sel]) == MSGBTN_YES;
            if( ok ) {
                break;
            }
        }else{
            returnValue = NULL;
        }
    }while(sel >= 0);

    return returnValue;
}

GuiDlgGameFileSelect::GuiDlgGameFileSelect(GuiContainer *parent, const char *name)
                    : GuiDialog(parent, name)
{
    frame = NULL;
    list = new GuiElmSelectionList(this, name, GFSEL_SELECTION_LINES);
    num_files = 0;
}

GuiDlgGameFileSelect::~GuiDlgGameFileSelect()
{
    // Remove UI elements
    if( list != NULL ) {
        RemoveAndDelete(list);
    }
    RemoveAndDelete(frame);
    // Free stuff
    FreeFileList();
}

