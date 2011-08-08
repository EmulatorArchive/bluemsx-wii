
#include "GuiDlgDirSelect.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../Arch/ArchFile.h"

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmSelectionList.h"
#include "../GuiDialogs/GuiDlgMessageBox.h"

#define DSEL_YPITCH 50
#define DIRSEL_FADE_FRAMES 10

char *GuiDlgDirSelect::DoModal(void)
{
    bool go_back_level = false;
    char prev_dir[MAX_DIRMANAGER_PATH] = "";
    char *return_value = NULL;

    // On re-entry, go back one level if not on root level
    if( dir_level > 0 ) {
        go_back_level = true;
    }

    // Seletion loop
    bool quit = false;
    do {
        DirElement *selected_dir = NULL;

        // Go back one level if requested for
        if( go_back_level ) {
            char *p = directory.CreatePath("");
            for( char *s = p; s != NULL; s = strstr(p, DIR_SLASH_S) ) {
              p = ++s;
            }
            strcpy(prev_dir, p);
            directory.ChangeDirectory("..");
            dir_level--;
            go_back_level = false;
        }

        // Load dirs database and init list
        return_value = InitialiseList(prev_dir);
        if( return_value != NULL ) {
            break;
        }

        // Run GUI
        for(;;) {
            if( Run() ) {
                int sel = list->GetSelectedItem();

                // enter selected directory
                selected_dir = dirs.GetDir(sel);
                directory.ChangeDirectory(selected_dir->GetDirectory());
                dir_level++;
                break;
            }else{
                if( dir_level == 0 ) {
                    // on root level, leave after confirmation
                    bool ok = GuiDlgMessageBox::ShowModal(this, "wantquit",
                                                          MSGT_YESNO, NULL, 192,
                                                          new GuiEffectFade(DIRSEL_FADE_FRAMES,0,true),
                                                          new GuiEffectFade(DIRSEL_FADE_FRAMES,0,true),
                                                          "Do you want to quit?") == MSGBTN_YES;
                    if( ok ) {
                        quit = true;
                        break;
                    }
                }else{
                    go_back_level = true;
                    break;
                }
            }
        }
    }while( !quit );

    return return_value;
}

char* GuiDlgDirSelect::InitialiseList(char *prevsel)
{
    dirs.Load(directory.CreatePath(xmlfile));
    num_dirs = dirs.GetNumberOfDirs();
    if( num_dirs == 0 ) {
        return directory.CreatePath("");
    }
    title_list = (const char**)realloc(title_list, num_dirs * sizeof(const char*));
    for(int i = 0; i < num_dirs; i++) {
        title_list[i] = dirs.GetDir(i)->GetName();
    }

    // When just gone back one level, find entry comming from
    int sel = -1;
    if( prevsel && strlen(prevsel) > 0 ) {
        for( int i = 0; i < num_dirs; i++ ) {
            if( strcmp(prevsel, dirs.GetDir(i)->GetDirectory()) == 0 ) {
                sel = i;
            }
        }
        prevsel = NULL;
    }
    
    // Selection
    GXColor white = {255, 255, 255, 255};
    list->InitSelection(new GuiElmListLineDefault(this, "defaultline", white, 32, false),
                        (void**)title_list, num_dirs, sel, DSEL_YPITCH,
                        320-180+8, 24+20, 24, 2*180-16);

    return NULL;
}

GuiDlgDirSelect::GuiDlgDirSelect(GuiContainer* parent, const char* name, const char* startdir, const char* filename)
                :GuiDialog(parent, name)
{
    title_list = NULL;
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BLUE, 320-180, 24, 2*180, 440-48);
    list = new GuiElmSelectionList(this, "list", NUM_DIR_ITEMS);

    directory.ChangeDirectory(startdir);
    strcpy(xmlfile, filename);
    dir_level = 0;

    (void)InitialiseList(NULL);

    AddTop(frame);
    AddTop(list);
}

GuiDlgDirSelect::~GuiDlgDirSelect()
{
    if( title_list ) {
        free(title_list);
    }
    RemoveAndDelete(frame);
    RemoveAndDelete(list);
}

