
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
    char *prevsel = NULL;
    char *return_value = NULL;

    // On re-entry, go back one level if not on root level
    if( dir_level > 0 ) {
        char *p = current_dir + strlen(current_dir) - 1;
        while(*p != '/') p--;
        *p = '\0';
        prevsel = p + 1;
        dir_level--;
    }

    // Seletion loop
    bool quit = false;
    do {
        DirElement *selected_dir = NULL;

        // Load dirs database and init list
        return_value = InitialiseList(prevsel);
        if( return_value != NULL ) {
            break;
        }

        // Run GUI
        for(;;) {
            if( Run() ) {
                int sel = list->GetSelectedItem();

                // enter selected directory
                selected_dir = dirs.GetDir(sel);
                strcat(current_dir, "/");
                strcat(current_dir, selected_dir->GetDirectory());
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
                    // go back one level
                    char *p = current_dir + strlen(current_dir) - 1;
                    while(*p != '/') p--;
                    *p = '\0';
                    prevsel = p + 1;
                    dir_level--;
                    break;
                }
            }
        }
    }while( !quit );

    return return_value;
}

char* GuiDlgDirSelect::InitialiseList(char *prevsel)
{
    archSetCurrentDirectory(current_dir);
    dirs.Load(xmlfile);
    num_dirs = dirs.GetNumberOfDirs();
    if( num_dirs == 0 ) {
        return current_dir;
    }
    title_list = (const char**)realloc(title_list, num_dirs * sizeof(const char*));
    for(int i = 0; i < num_dirs; i++) {
        title_list[i] = dirs.GetDir(i)->GetName();
    }

    // When just gone back one level, find entry comming from
    int sel = -1;
    if( prevsel ) {
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

    strcpy(current_dir, startdir);
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

