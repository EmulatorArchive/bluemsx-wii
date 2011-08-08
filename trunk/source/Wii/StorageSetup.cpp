/*****************************************************************************
** $Source: StorageSetup.c,v $
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/

#include "StorageSetup.h"

#define USE_EMBEDDED_SDCARD_IMAGE 0

#include <fat.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#if USE_EMBEDDED_SDCARD_IMAGE
#include "sdcard.inc"
#include "gamepack.inc"
#endif
#include "../Utils/ZipFromMem.h"
#include "../Utils/ZipHelper.h"

extern "C" {
#include "../Arch/ArchFile.h"
#include "../Arch/ArchThread.h"
}
#include "../GuiDialogs/GuiDlgMenu.h"
#include "../GuiDialogs/GuiDlgMessageBox.h"
#include "../GuiBase/GuiEffectFade.h"

static void SetupStorageProgressCallback(int total, int current)
{
    static int prev_percent = -1;
    int percent;

    percent = (current * 100 + (total / 2)) / total;
    if( percent != prev_percent ) {
        //msgboxSdSetup->SetText("Installing (%d%%) ...", percent);
        prev_percent = percent;
    }
}

bool SetupInstallZip(GuiContainer *container, void *zipptr, unsigned int zipsize,
                     const char *directory, const char *message)
{
    // Prepare messagebox
    bool ok = GuiDlgMessageBox::ShowModal(container, "wantinstall",
                                          MSGT_YESNO, NULL, 192, new GuiEffectFade(10),
                                          new GuiEffectFade(10), message) == MSGBTN_YES;
    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(container, "install");
    container->AddTop(msgbox, new GuiEffectFade(10));
    if( ok ) {
        bool failed = false;
        msgbox->Create(MSGT_TEXT, NULL, 128, "Installing (0%%) ...    ");

        MemZip *zip = MemZipOpenResource(zipptr, zipsize);
        if( zip ) {
            chdir(directory);
            if( !zipExtract(zip->unzip, 1, NULL, SetupStorageProgressCallback) ) {
                printf("failed to extract zip resource\n");
                failed = true;
            }
            MemZipClose(zip);
        }else{
            msgbox->Create(MSGT_TEXT, NULL, 128, "Failed to install!");
            archThreadSleep(3000);
            ok = false; // leave
        }
    }
    container->RemoveAndDelete(msgbox, new GuiEffectFade(10));
    return ok;
}

bool SetupStorage(GuiContainer *container, bool bSDMounted, bool bUSBMounted)
{
    static char *sStorageRoot;
    bool ok = true;

    // Check if there already is a MSX folder on a storage device
    // Prevalate USB, as it is a lot faster than SD (USB 2.0 supported since IOS58)
    bool bBlueMSXInstalled = false;
    struct stat s;
    if( bUSBMounted && stat(USB_ROOT_DIR MSX_DIR, &s) == 0 ) {
        archSetCurrentDirectory(USB_ROOT_DIR MSX_DIR);
        sStorageRoot = USB_ROOT_DIR;
        bBlueMSXInstalled = true;
    } else
    if( bSDMounted && stat(SD_ROOT_DIR MSX_DIR, &s) == 0 ) {
        archSetCurrentDirectory(SD_ROOT_DIR MSX_DIR);
        sStorageRoot = SD_ROOT_DIR;
        bBlueMSXInstalled = true;
    }

#if USE_EMBEDDED_SDCARD_IMAGE
    if( !bBlueMSXInstalled ) {
        if( bSDMounted && bUSBMounted ) {
            // Have the user decide which device to install to
            GuiDlgMenu *menu = new GuiDlgMenu(container, "menu", 2);
            container->AddTop(menu);
            const char *menu_items[] = {
              "Use USB Device",
              "Use SD-Card",
            };
            int action = menu->DoModal(menu_items, 2, 344);
            switch( action ) {
                case 0: /* USB Device */
                    archSetCurrentDirectory(USB_ROOT_DIR MSX_DIR);
                    sStorageRoot = USB_ROOT_DIR);
                    break;
                case 1: /* SD-Card */
                    archSetCurrentDirectory(SD_ROOT_DIR MSX_DIR);
                    sStorageRoot = SD_ROOT_DIR;
                    break;
                default:
                    break;
            }
            container->RemoveAndDelete(menu);
        } else if( bSDMounted ) {
            archSetCurrentDirectory(SD_ROOT_DIR MSX_DIR);
            sStorageRoot = SD_ROOT_DIR;
        } else if( bUSBMounted ) {
            archSetCurrentDirectory(USB_ROOT_DIR MSX_DIR);
            sStorageRoot = USB_ROOT_DIR;
        }
    }

    char sDatabasePath[30];
    char sMachinesPath[30];
    sprintf(sDatabasePath, "%s/Databases", archGetCurrentDirectory());
    sprintf(sMachinesPath, "%s/Machines", archGetCurrentDirectory());
    // Check if 'Database' and 'Machine' directories exists
    if( stat(sDatabasePath, &s) != 0 ||
        stat(sMachinesPath, &s) != 0 ) {

        // Does not exist yet, install
        ok = SetupInstallZip(container, sdcard, sizeof(sdcard), sStorageRoot,
                             "Storage is not setup yet,\n"
                             "Do you want to do it now?");
    }
    if( ok ) {
        char sGamesPath[20];
        sprintf(sGamesPath, "%s/Games", archGetCurrentDirectory());
        // Check if 'Games' directory exist
        if( stat(sGamesPath, &s) != 0 ) {
            // Does not exist yet, install
            ok = SetupInstallZip(container, gamepack, sizeof(gamepack), archGetCurrentDirectory(),
                                 "No gamepack is installed yet,\n"
                                 "Install the basic pack now?");
        }
    }
#endif

    return ok;
}

