/*****************************************************************************
** $Source: sdsetup.c,v $
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

#include "SdSetup.h"

#define USE_EMBEDDED_SDCARD_IMAGE 1

#include <fat.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#if USE_EMBEDDED_SDCARD_IMAGE
#include "sdcard.inc"
#include "gamepack.inc"
#endif
#include "ZipFromMem.h"
#include "ZipHelper.h"

extern "C" {
#include "ArchThread.h"
}
#include "GuiMessageBox.h"


static GuiMessageBox *msgboxSdSetup = NULL;

void SetupSDProgressCallback(int total, int current)
{
    static int prev_percent = -1;
    int percent;

    percent = (current * 100 + (total / 2)) / total;
    if( percent != prev_percent ) {
        msgboxSdSetup->SetText("Installing (%d%%) ...", percent);
        prev_percent = percent;
    }
}

bool SetupInstallZip(GuiManager *manager, void *zipptr, unsigned int zipsize,
                     const char *directory, const char *message)
{
    // Prepare messagebox
    msgboxSdSetup = new GuiMessageBox(manager);

    bool ok = msgboxSdSetup->Show(message, NULL, MSGT_YESNO, 192) == BTN_YES;
    if( ok ) {
        bool failed = false;
        msgboxSdSetup->Show("Installing (0%%) ...    ");

        MemZip *zip = MemZipOpenResource(zipptr, zipsize);
        if( zip ) {
            chdir(directory);
            if( !zipExtract(zip->unzip, 1, NULL, SetupSDProgressCallback) ) {
                printf("failed to extract zip resource\n");
                failed = true;
            }
            MemZipClose(zip);
        }else{
            msgboxSdSetup->Show("Failed to install!");
            archThreadSleep(3000);
            ok = false; // leave
        }
    }
    delete msgboxSdSetup;
    return ok;
}

bool SetupSDCard(GuiManager *manager)
{
    bool ok = true;

#if USE_EMBEDDED_SDCARD_IMAGE
    // Check if 'Database' and 'Machine' directories exists
    struct stat s;
    if( stat(MSX_ROOT_DIR"/Databases", &s) != 0 ||
        stat(MSX_ROOT_DIR"/Machines", &s) != 0 ) {
        // Does not exist yet, install
        ok = SetupInstallZip(manager, sdcard, sizeof(sdcard), SD_ROOT_DIR,
                             "SD-Card is not setup yet,\n"
                             "Do you want to do it now?");
    }
    if( ok ) {
        // Check if 'Games' directory exist
        if( stat(MSX_ROOT_DIR"/Games", &s) != 0 ) {
            // Does not exist yet, install
            ok = SetupInstallZip(manager, gamepack, sizeof(gamepack), MSX_ROOT_DIR,
                                 "No gamepack is installed yet,\n"
                                 "Install the basic pack now?");
        }
    }
#endif

    return ok;
}

