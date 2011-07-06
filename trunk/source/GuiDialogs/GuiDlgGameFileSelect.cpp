#include "GuiDlgGameFileSelect.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../Arch/ArchFile.h"
#include "../Arch/ArchGlob.h"

#include "../GuiBase/GuiEffectFade.h"
#include "../GuiDialogs/GuiDlgMessageBox.h"
#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmSelectionList.h"

// Resources
#include "../Gui/GuiFonts.h"
#include "../Gui/GuiImages.h"

#define GFSEL_FADE_FRAMES     10

#define GFSEL_WIDTH           (640-28)
#define GFSEL_SELECTION_LINES 10
#define GFSEL_FONTSIZE        20
#define GFSEL_YPITCH          30
#define GFSEL_HEIGHT          300
#define GFSEL_MENU_SPACING    8
#define GFSEL_X_SPACING       12
#define GFSEL_LIST_WIDTH      (GFSEL_WIDTH-2*GFSEL_X_SPACING)

//-----------------------

typedef struct _fileitem {
    bool is_dir;
    char *txt_col_a;
    char *txt_col_b;
    char *txt_col_c;
} FILEITEM;

class GuiElmListLineFile : public GuiElmListLine {
public:
    GuiElmListLineFile(GuiContainer *parent, const char *name,
                       GXColor fontcol, int fontsz, bool cntr);
    virtual ~GuiElmListLineFile();

    virtual GuiElmListLine* Create(GuiContainer *parent);
    virtual void Initialize(void *itm);
    virtual COLL CollidesWith(GuiSprite* spr, bool compl);
private:
    GuiSprite *spr_col_a;
    GuiSprite *spr_col_b;
    GuiSprite *spr_col_c;
    GXColor fontcolor;
    int fontsize;
    bool center;
};

GuiElmListLineFile::GuiElmListLineFile(GuiContainer *parent, const char *name,
                                       GXColor fontcol, int fontsz, bool cntr)
                     : GuiElmListLine(parent, name)
{
    spr_col_a = NULL;
    spr_col_b = NULL;
    spr_col_c = NULL;
    fontcolor = fontcol;
    fontsize = fontsz;
    center = cntr;
}

GuiElmListLineFile::~GuiElmListLineFile()
{
    if( spr_col_a != NULL ) {
        RemoveAndDelete(spr_col_a);
        spr_col_a = NULL;
        RemoveAndDelete(spr_col_b);
        spr_col_b = NULL;
        RemoveAndDelete(spr_col_c);
        spr_col_c = NULL;
    }
}

void GuiElmListLineFile::Initialize(void *itm)
{
    FILEITEM *item;
    item = (FILEITEM *)itm;
    spr_col_a = new GuiSprite(this, "col_a");
    spr_col_a->CreateTextImage(g_fontArial, fontsize, GetWidth(), 0, center, fontcolor, item->txt_col_a);
    spr_col_a->SetPosition(0, fontsize/3);
    AddTop(spr_col_a);
    spr_col_b = new GuiSprite(this, "col_b");
    spr_col_b->CreateTextImage(g_fontArial, fontsize, GetWidth(), 0, false, fontcolor, item->txt_col_b);
    spr_col_b->SetPosition(GetWidth() - 200, fontsize/3);
    AddTop(spr_col_b);
    spr_col_c = new GuiSprite(this, "col_c");
    spr_col_c->CreateTextImage(g_fontArial, fontsize, GetWidth(), 0, false, fontcolor, item->txt_col_c);
    spr_col_c->SetPosition(GetWidth() - 100, fontsize/3);
    AddTop(spr_col_c);
}

GuiElmListLine* GuiElmListLineFile::Create(GuiContainer *parent)
{
    return new GuiElmListLineFile(parent, "GuiElmListLineFile",
                                  fontcolor, fontsize, center);
}

COLL GuiElmListLineFile::CollidesWith(GuiSprite* spr, bool compl)
{
    assert( spr_col_a != NULL );
    return spr_col_a->CollidesWith(spr, compl);
}

//-----------------------

void GuiDlgGameFileSelect::CreateFileList(void)
{
    ArchGlob *glob_dirs, *glob_files;

    // Free old list if exists
    FreeFileList();

    // Find files/dirs
    glob_dirs = archGlob("*", ARCH_GLOB_DIRS);
    glob_files = archGlob("*", ARCH_GLOB_FILES);

    // Allocate item buffer
    items = (FILEITEM**)malloc((glob_dirs->count + glob_files->count + 1)*sizeof(FILEITEM*));

    // Add '..' if not int the root
    const char *curdir = archGetCurrentDirectory();
    if( strcmp(curdir, root_dir) != 0 ) {
        items[num_files] = new FILEITEM;
        items[num_files]->is_dir = true;
        items[num_files]->txt_col_a = strdup("..");
        items[num_files]->txt_col_b = strdup("<DIR>");
        items[num_files]->txt_col_c = strdup("");
        num_files++;
    }

    // Process directories
    int first_index = num_files;
    if (glob_dirs) {
        for (int i = 0; i < glob_dirs->count; i++) {
            char path[256];
            strcpy(path, glob_dirs->pathVector[i]);
            struct stat s;
            if( stat(path, &s) >= 0 ) {
                int j, k;
                // Get modification time
                struct tm *timeinfo;
                timeinfo = localtime(&s.st_mtime);
#ifndef WIN32
                timeinfo->tm_year += 80; // libogc bug: year = 1929
#endif
                char timestr[16];
                strftime(timestr, sizeof(timestr), "%Y/%m/%d", timeinfo);
                // Find filename
                char *p, *filename = path;
                while( (p = strstr(filename, "/")) || (p = strstr(filename, "\\")) ) {
                    filename = p+1;
                }
                // to get sorted list, find position where to strore in list
                for(k = first_index; k < num_files && stricmp(filename, items[k]->txt_col_a) > 0; k++);
                // shift all entries behind
                for(j = num_files; j > k; j--) {
                    items[j] = items[j-1];
                }
                // store in list
                items[k] = new FILEITEM;
                items[k]->is_dir = true;
                items[k]->txt_col_a = strdup(filename);
                items[k]->txt_col_b = strdup("<DIR>");
                items[k]->txt_col_c = strdup(timestr);
                num_files++;
            }
        }
        archGlobFree(glob_dirs);
    }

    // Process files
    first_index = num_files;
    if (glob_files) {
        for (int i = 0; i < glob_files->count; i++) {
            char path[256];
            strcpy(path, glob_files->pathVector[i]);
            struct stat s;
            if( stat(path, &s) >= 0 ) {
                int j, k;
                // Get modification time
                struct tm *timeinfo;
                timeinfo = localtime(&s.st_mtime);
#ifndef WIN32
                timeinfo->tm_year += 80; // libogc bug: year = 1929
#endif
                char timestr[16];
                strftime(timestr, sizeof(timestr), "%Y/%m/%d", timeinfo);
                // Get file size
                char sizestr[16];
                sprintf(sizestr, "%d", s.st_size);
                // Find filename
                char *p, *filename = path;
                while( (p = strstr(filename, "/")) || (p = strstr(filename, "\\")) ) {
                    filename = p+1;
                }
                // to get sorted list, find position where to strore in list
                for(k = first_index; k < num_files && stricmp(filename, items[k]->txt_col_a) > 0; k++);
                // shift all entries behind
                for(j = num_files; j > k; j--) {
                    items[j] = items[j-1];
                }
                // store in list
                items[k] = new FILEITEM;
                items[k]->is_dir = false;
                items[k]->txt_col_a = strdup(filename);
                items[k]->txt_col_b = strdup(sizestr);
                items[k]->txt_col_c = strdup(timestr);
                num_files++;
            }
        }
        archGlobFree(glob_files);
    }

}

void GuiDlgGameFileSelect::FreeFileList(void)
{
    if( items != NULL ) {
        for(int i = 0; i < num_files; i++) {
            free(items[i]->txt_col_a);
            items[i]->txt_col_a = NULL;
            free(items[i]->txt_col_b);
            items[i]->txt_col_b = NULL;
            free(items[i]->txt_col_c);
            items[i]->txt_col_c = NULL;
            delete items[i];
            items[i] = NULL;
        }
        free(items);
        items = NULL;
    }
    num_files = 0;
}

int GuiDlgGameFileSelect::Create(void)
{
    // Load files
    CreateFileList();
    if( num_files == 0 ) {
        return 0;
    }

    // Selection
    GXColor black = {0, 0, 0, 255};
    list->InitSelection(new GuiElmListLineFile(this, "fileline", black, GFSEL_FONTSIZE, false),
                        (void**)items, num_files, 0, GFSEL_YPITCH,
                        posx+GFSEL_X_SPACING,
                        posy+sizey/2-(GFSEL_SELECTION_LINES*GFSEL_YPITCH)/2,
                        GFSEL_MENU_SPACING, GFSEL_LIST_WIDTH);

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
            if( items[sel]->is_dir ) {
                // It's a directory, enter it
                archSetCurrentDirectory(items[sel]->txt_col_a);
                Create();
                continue;
            }else{
                // Combine path and filename
                char *file = items[sel]->txt_col_a;
                char *cur_dir = strdup(archGetCurrentDirectory());
                char *directory = cur_dir + strlen(root_dir);
                if( *directory == '/' ) directory++;
                char *relpath = (char*)malloc(strlen(directory)+strlen(file)+2);
                if( strlen(directory) > 0 ) {
                    strcpy(relpath, directory);
                    strcat(relpath, "/");
                    strcat(relpath, file);
                }else{
                    strcpy(relpath, file);
                }
                free(cur_dir);
                
                // Ask confirmation
                bool ok = GuiDlgMessageBox::ShowModal(this, "wantadd",
                                                      MSGT_YESNO, NULL, 192,
                                                      new GuiEffectFade(GFSEL_FADE_FRAMES), new GuiEffectFade(GFSEL_FADE_FRAMES),
                                                      "Do you want to add\n\"%s\"", file) == MSGBTN_YES;
                if( ok ) {
                    returnValue = relpath;
                    break;
                }
            }
        }else{
            returnValue = NULL;
        }
    }while(sel >= 0);

    return returnValue;
}

GuiDlgGameFileSelect::GuiDlgGameFileSelect(GuiContainer *parent, const char *name,
                                           const char *directory)
                    : GuiDialog(parent, name)
{
    items = NULL;

    // Prepare directory
    org_dir = strdup(archGetCurrentDirectory());
    archSetCurrentDirectory(directory);
    root_dir = strdup(archGetCurrentDirectory());

    // Frame
    posx = (640-GFSEL_WIDTH)/2;
    posy = GetHeight()/2-(GFSEL_HEIGHT/2)-16;
    sizex = GFSEL_WIDTH;
    sizey = GFSEL_HEIGHT+32;
    frame = new GuiElmFrame(this, "frame", FRAMETYPE_BRUSHED, posx, posy, sizex, sizey, 240);
    AddTop(frame);

    // List
    list = new GuiElmSelectionList(this, name, GFSEL_SELECTION_LINES);
    AddTop(list);
    num_files = 0;
}

GuiDlgGameFileSelect::~GuiDlgGameFileSelect()
{
    // Restore current directory
    archSetCurrentDirectory(org_dir);

    // Remove UI elements
    if( list != NULL ) {
        RemoveAndDelete(list);
    }
    RemoveAndDelete(frame);

    // Free stuff
    FreeFileList();
    delete(root_dir);
    delete(org_dir);
}

