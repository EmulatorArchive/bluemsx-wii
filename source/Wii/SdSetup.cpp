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

bool SetupSDCard(GuiManager *manager)
{
    bool ok = true;

    // Prepare messagebox
    msgboxSdSetup = new GuiMessageBox(manager);

    // Init SD-Card access
    if( !fatInitDefault() ) {
        msgboxSdSetup->Show("SD-Card error!");
        archThreadSleep(3000);
        delete msgboxSdSetup;
        return false;
    }
#if USE_EMBEDDED_SDCARD_IMAGE
    // Check if 'MSX' directory exists
    struct stat s;
    if( stat(MSX_ROOT_DIR, &s) == 0 ) {
        delete msgboxSdSetup; // allready exists, done
        return true;
    }

    ok = msgboxSdSetup->Show("SD-Card is not setup yet,\n"
                             "Do you want to do it now?", NULL, true, 192);
    if( ok ) {
        bool failed = false;
        msgboxSdSetup->Show("Installing (0%%) ...    ");

        zlib_filefunc_def filefunc;
        fill_fopen_memfunc(&filefunc, sizeof(sdcard));
        unzFile uf = unzOpen2((const char *)sdcard, &filefunc);
        if( uf ) {
            chdir(SD_ROOT_DIR);
            if( !zipExtract(uf, 1, NULL, SetupSDProgressCallback) ) {
                printf("failed to extract zip resource\n");
                failed = true;
            }
            unzClose(uf);
        }else{
            printf("failed to open zip resource\n");
            failed = true;
        }
        free_fopen_memfunc(&filefunc);

        if( stat(MSX_ROOT_DIR, &s) != 0 ) {
            printf("root dir still not exist\n");
            failed = true;
        }
        if( failed ) {
            msgboxSdSetup->Show("Failed to install!");
            archThreadSleep(3000);
            ok = false; // leave
        }
    }
#endif
    delete msgboxSdSetup;
    return ok;
}

