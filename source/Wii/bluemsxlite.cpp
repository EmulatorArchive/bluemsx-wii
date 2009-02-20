/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Sdl/bluemsxlite.c,v $
**
** $Revision: 1.20 $
**
** $Date: 2006/06/24 08:35:29 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include <sys/stat.h>

#include "GuiDirSelect.h"
#include "GuiGameSelect.h"
#include "GuiMenu.h"
#include "GuiMessageBox.h"
#include "GuiKeyboard.h"

extern "C" {
#include "CommandLine.h"
#include "Properties.h"
#include "ArchFile.h"
#include "VideoRender.h"
#include "AudioMixer.h"
#include "Casette.h"
#include "PrinterIO.h"
#include "UartIO.h"
#include "MidiIO.h"
#include "Machine.h"
#include "Board.h"
#include "Emulator.h"
#include "FileHistory.h"
#include "Actions.h"
#include "Language.h"
#include "LaunchFile.h"
#include "ArchEvent.h"
#include "ArchGlob.h"
#include "ArchSound.h"
#include "ArchNotifications.h"
#include "JoystickPort.h"
#include "WiiShortcuts.h"
#include "ArchThread.h"
#include "ogc_video.h"
#include "WiiInput.h"

void WiiTimerInit(void);
void WiiTimerDestroy(void);
}

#include "GuiConsole.h"
#include "GuiContainer.h"
#include "GuiFonts.h"
#include "GuiImages.h"

#define CONSOLE_DEBUG          0
#define MALLOC_LOG_BLUEMSX_RUN 0
#define MALLOC_LOG_GUI         0

#define MSX_ROOT_DIR "fat:/MSX"

static Properties* properties;
static Video* video;
static Mixer* mixer;

static int   bitDepth = 16;
static int   zoom = 1;
static int   displayPitch = TEX_WIDTH * 2;

static bool  g_doQuit = false;

static GuiManager *manager = NULL;
static char *g_dpyData = NULL;
static GuiConsole *console = NULL;
static GuiMessageBox *msgbox = NULL;
static GuiKeyboard *osk = NULL;

extern KBDHANDLE kbdHandle;

#define WIDTH  320
#define HEIGHT 240

#define EVENT_UPDATE_DISPLAY 2
#define EVENT_UPDATE_WINDOW  3

extern "C" void archTrap(UInt8 value)
{
}

int  archUpdateEmuDisplay(int syncMode)
{
    return 1;
}

void archUpdateWindow()
{
}

static char stateDir[PROP_MAXPATH]  = "";
static char screenShotDir[PROP_MAXPATH]  = "";

void setDefaultPaths(const char* rootDir)
{
    char buffer[512];

    propertiesSetDirectory(rootDir, rootDir);

    sprintf(buffer, "%s/Audio Capture", rootDir);
    archCreateDirectory(buffer);
    actionSetAudioCaptureSetDirectory(buffer, "");

    sprintf(buffer, "%s/QuickSave", rootDir);
    archCreateDirectory(buffer);
    strcpy(stateDir, buffer);
    actionSetQuickSaveSetDirectory(buffer, "");

    sprintf(buffer, "%s/SRAM", rootDir);
    archCreateDirectory(buffer);
    boardSetDirectory(buffer);

    sprintf(buffer, "%s/Casinfo", rootDir);
    archCreateDirectory(buffer);
    tapeSetDirectory(buffer, "");

    sprintf(buffer, "%s/Databases", rootDir);
    archCreateDirectory(buffer);
    mediaDbLoad(buffer);

    sprintf(buffer, "%s/Keyboard Config", rootDir);
    archCreateDirectory(buffer);
    keyboardSetDirectory(buffer);

    sprintf(buffer, "%s/ScreenShots", rootDir);
    archCreateDirectory(buffer);
    strcpy(screenShotDir, buffer);
}

static char currentDisk[256];

void archDiskQuickChangeNotify(int driveId, char* fileName, const char* fileInZipFile)
{
    printf("DISKCHANGE: %d, '%s', '%s'\n", driveId, fileName, fileInZipFile);
    if( fileInZipFile ) {
        strcpy(currentDisk, fileInZipFile);
    }else{
        strcpy(currentDisk, fileName);
    }
    // Show popup
    msgbox->ShowPopup(currentDisk, g_imgFloppyDisk, 192);
}

void blueMsxInit(int resetProperties)
{
    int width;
    int height;

    // Init timer
    WiiTimerInit();
    atexit(WiiTimerDestroy);

    setDefaultPaths(archGetCurrentDirectory());

    properties = propCreate(resetProperties, 0, P_KBD_EUROPEAN, 0, "");

    properties->emulation.syncMethod = P_EMU_SYNCTOVBLANKASYNC;

    if (resetProperties == 2) {
        propDestroy(properties);
        return;
    }

    video = videoCreate();

    videoUpdateAll(video, properties);

    if (properties->video.windowSize == P_VIDEO_SIZEFULLSCREEN) {
        zoom = properties->video.fullscreen.width / WIDTH;
        bitDepth = properties->video.fullscreen.bitDepth;
    }
    else {
        if (properties->video.windowSize == P_VIDEO_SIZEX1) {
            zoom = 1;
        }
        else {
            zoom = 2;
        }
        bitDepth = 16;
    }

    width  = zoom * WIDTH;
    height = zoom * HEIGHT;

    keyboardInit();

    mixer = mixerCreate();

    emulatorInit(properties, mixer);

    actionInit(video, properties, mixer);

    langInit();

    tapeSetReadOnly(properties->cassette.readOnly);

    langSetLanguage((EmuLanguageType)properties->language);

    joystickPortSetType(0, (JoystickPortType)properties->joy1.typeId);
    joystickPortSetType(1, (JoystickPortType)properties->joy2.typeId);
    printerIoSetType((PrinterType)properties->ports.Lpt.type, properties->ports.Lpt.fileName);
    printerIoSetType((PrinterType)properties->ports.Lpt.type, properties->ports.Lpt.fileName);
    uartIoSetType((UartType)properties->ports.Com.type, properties->ports.Com.fileName);
    midiIoSetMidiOutType((MidiType)properties->sound.MidiOut.type, properties->sound.MidiOut.fileName);
    midiIoSetMidiInType((MidiType)properties->sound.MidiIn.type, properties->sound.MidiIn.fileName);
    ykIoSetMidiInType((MidiType)properties->sound.YkIn.type, properties->sound.YkIn.fileName);

    emulatorRestartSound();

    for (int i = 0; i < MIXER_CHANNEL_TYPE_COUNT; i++) {
        mixerSetChannelTypeVolume(mixer, i, properties->sound.mixerChannel[i].volume);
        mixerSetChannelTypePan(mixer, i, properties->sound.mixerChannel[i].pan);
        mixerEnableChannelType(mixer, i, properties->sound.mixerChannel[i].enable);
    }

    mixerSetMasterVolume(mixer, properties->sound.masterVolume);
    mixerEnableMaster(mixer, properties->sound.masterEnable);

    videoUpdateAll(video, properties);

    mediaDbSetDefaultRomType(properties->cartridge.defaultType);

    for (int i = 0; i < PROP_MAX_CARTS; i++) {
        if (properties->media.carts[i].fileName[0]) insertCartridge(properties, i, properties->media.carts[i].fileName, properties->media.carts[i].fileNameInZip, properties->media.carts[i].type, -1);
        updateExtendedRomName(i, properties->media.carts[i].fileName, properties->media.carts[i].fileNameInZip);
    }

    for (int i = 0; i < PROP_MAX_DISKS; i++) {
        if (properties->media.disks[i].fileName[0]) insertDiskette(properties, i, properties->media.disks[i].fileName, properties->media.disks[i].fileNameInZip, -1);
        updateExtendedDiskName(i, properties->media.disks[i].fileName, properties->media.disks[i].fileNameInZip);
    }

    for (int i = 0; i < PROP_MAX_TAPES; i++) {
        if (properties->media.tapes[i].fileName[0]) insertCassette(properties, i, properties->media.tapes[i].fileName, properties->media.tapes[i].fileNameInZip, 0);
        updateExtendedCasName(i, properties->media.tapes[i].fileName, properties->media.tapes[i].fileNameInZip);
    }

    Machine* machine = machineCreate(properties->emulation.machineName);
    if (machine != NULL) {
        boardSetMachine(machine);
        machineDestroy(machine);
    }

    boardSetFdcTimingEnable(properties->emulation.enableFdcTiming);
    boardSetY8950Enable(properties->sound.chip.enableY8950);
    boardSetYm2413Enable(properties->sound.chip.enableYM2413);
    boardSetMoonsoundEnable(properties->sound.chip.enableMoonsound);
    boardSetVideoAutodetect(properties->video.detectActiveMonitor);
}

static Sprite *sprBackground;

typedef struct {
    int number_of_saves;
    char **filenames;
    char **timestrings;
} SAVE_STATES;

SAVE_STATES *GetStateFileList(Properties* properties, char* directory, char* prefix, char* extension, int digits)
{
    static SAVE_STATES states;
    static char *filenames[256];
    static char *timestrings[256];
    ArchGlob* glob;
    static char filename[512];
    char baseName[128];
    int i, k;
    int numMod = 1;
    char filenameFormat[32] = "%s/%s%s_";
    char destfileFormat[32];

    for (i = 0; i < digits; i++) {
        strcat(filenameFormat, "?");
        numMod *= 10;
    }
    strcat(filenameFormat, "%s");
    sprintf(destfileFormat, "%%s/%%s%%s_%%0%di%%s", digits);

    createSaveFileBaseName(baseName, properties, 0);

    sprintf(filename, filenameFormat, directory, prefix, baseName, extension);

    glob = archGlob(filename, ARCH_GLOB_FILES);

    if (glob) {
        k = 0;
        if (glob->count > 0) {
            for (i = 0; i < glob->count; i++) {
                struct stat s;
                if( stat(glob->pathVector[i], &s) >= 0 ) {
                    timestrings[k] = strdup(ctime(&s.st_mtime));
                    filenames[k++] = strdup(glob->pathVector[i]);
                }
            }
        }
        states.timestrings = timestrings;
        states.filenames = filenames;
        states.number_of_saves = k;
        archGlobFree(glob);
    }else{
        states.number_of_saves = 0;
    }

    return &states;
}

void FreeStateFileList(SAVE_STATES *states)
{
    for(int i = 0; i < states->number_of_saves; i++) {
        free(states->timestrings[i]);
        states->timestrings[i] = NULL;
        free(states->filenames[i]);
        states->filenames[i] = NULL;
    }
}

void RenderEmuImage(void *arg)
{
    (void)arg;
    if( emulatorGetState() == EMU_RUNNING ) {
        FrameBuffer* frameBuffer;
        frameBuffer = frameBufferFlipViewFrame(properties->emulation.syncMethod == P_EMU_SYNCTOVBLANKASYNC);
        if (frameBuffer == NULL) {
            frameBuffer = frameBufferGetWhiteNoiseFrame();
        }

        videoRender(video, frameBuffer, bitDepth, zoom,
                    g_dpyData, 0, displayPitch, -1);
        DCFlushRange(g_dpyData, TEX_WIDTH * TEX_HEIGHT * 4);
    }
}

void actionToggleOnScreenKbd(void)
{
    if( osk->IsShowing() ) {
        osk->Remove();
    }else{
        osk->Show();
    }
}

static void blueMsxRun(GameElement *game, char *game_dir)
{
    int i;
#if MALLOC_LOG_BLUEMSX_RUN
    allocLogSetMarker();
#endif

    // Set current directory to the MSX-root
    archSetCurrentDirectory(MSX_ROOT_DIR);

    // Loading message
    msgbox->Show("Loading...");

    g_doQuit = false;

    printf("Title        : '%s'\n", game->GetName());
    printf("Command line : '%s'\n", game->GetCommandLine());
    printf("Screen shot 1: '%s'\n", game->GetScreenShot(0));
    printf("Screen shot 2: '%s'\n", game->GetScreenShot(1));

    // Init keyboard and remap keys
    keyboardReset();
    for(i = 0; i < KEY_LAST; i++) {
        int event = game->GetKeyMapping((KEY)i);
        if( event != -1 ) {
            keyboardRemapKey((KEY)i, event);
        }
    }

    // Create on-screen keyboard
    osk = new GuiKeyboard(manager);

    // Start emulator
    i = emuTryStartWithArguments(properties, game->GetCommandLine(), game_dir);
    if (i < 0) {
        printf("Failed to parse command line\n");
        delete osk;
        msgbox->Remove();
        return;
    }
    if (i == 0) {
        printf("Starting emulation\n");
        emulatorStart(NULL);
    }
    printf("Waiting for quit event...\n");
    //allocLogPrint();

    msgbox->Remove();
    archThreadSleep(100); // Don't know why, but it solve things...
    manager->Lock();
    manager->Remove(sprBackground);
    manager->SetYOffset(-37);
    console->SetPosition(12, 12+37);
    DrawableImage *emuImg = new DrawableImage;
    emuImg->CreateImage(TEX_WIDTH, TEX_HEIGHT, GX_TF_RGB565);
    g_dpyData = (char *)emuImg->GetTextureBuffer();
    manager->AddRenderCallback(RenderEmuImage, NULL);
    Sprite *emuSpr = new Sprite;
    emuSpr->SetImage(emuImg->GetImage());
    emuSpr->SetStretchWidth(640.0f / (float)TEX_WIDTH);
    emuSpr->SetStretchHeight(548.0f / (float)480);
    emuSpr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    emuSpr->SetRefPixelPosition(0, 0);
    emuSpr->SetPosition(0, 0);
    manager->AddTop(emuSpr);
    manager->Unlock();

    // Loop while the user hasn't quit
    GuiMenu *menu = new GuiMenu(manager, 4);
    const char *menu_items[] = {
      "Load state",
      "Save state",
      "Screenshot",
      "Keyboard",
      "Quit"
    };
    bool pressed = true;
    while(!g_doQuit) {
        if( KBD_GetKeyStatus(kbdHandle, KEY_JOY1_HOME) ) {
            if( !pressed ) {
                emulatorSuspend();
                bool leave_menu = false;
                do {
                    int selection = menu->DoModal(menu_items, 5, 344);
                    switch( selection ) {
                        SAVE_STATES *states;
                        case 0: /* Load state */
                            states = GetStateFileList(properties, stateDir, "", ".sta", 2);
                            if( states->number_of_saves > 0 ) {
                                int statenum = menu->DoModal((const char**)states->timestrings, states->number_of_saves, 360);
                                if( statenum != -1 ) {
                                    emulatorStop();
                                    emulatorStart(states->filenames[statenum]);
                                    VIDEO_WaitVSync();
                                    VIDEO_WaitVSync();
                                    VIDEO_WaitVSync();
                                    VIDEO_WaitVSync();
                                    emulatorSuspend();
                                }
                            }
                            FreeStateFileList(states);
                            break;
                        case 1: /* Save state */
                            actionQuickSaveState();
                            emulatorSuspend();
                            break;
                        case 2: /* Screenshot */
                            char fname1[256], fname2[256];
                            strcpy(fname1, game_dir);
                            strcat(fname1, "/");
                            strcat(fname1, game->GetScreenShot(0));
                            strcpy(fname2, game_dir);
                            strcat(fname2, "/");
                            strcat(fname2, game->GetScreenShot(1));
                            if( !archFileExists(fname1) ) {
                                (void)archScreenCaptureToFile(SC_NORMAL, fname1);
                            }else
                            if( !archFileExists(fname2) ) {
                                (void)archScreenCaptureToFile(SC_NORMAL, fname2);
                            }else{
                                (void)archScreenCaptureToFile(SC_NORMAL,
                                      generateSaveFilename(properties, screenShotDir, "", ".png", 2));
                            }
                            break;
                        case 3: /* Keyboard */
                            actionToggleOnScreenKbd();
                            break;
                        case 4: /* Quit */
                            g_doQuit = true;
                            leave_menu = true;
                            break;
                        case -1: /* leaved menu */
                            leave_menu = true;
                            break;
                    }
                }while(!leave_menu);
                emulatorResume();
                pressed = true;
            }
        }else{
            pressed = false;
        }
        // wait a frame
		VIDEO_WaitVSync();
    }
    delete menu;

    emulatorStop();

    // Remove emulator+keyboard from display
    manager->Lock();
    manager->RemoveRenderCallback(RenderEmuImage, NULL);
    manager->Remove(emuSpr);
    delete emuImg;
    delete emuSpr;
    delete osk;
    console->SetPosition(12, 12);
    manager->SetYOffset(0);
    manager->AddTop(sprBackground);
    manager->Unlock();

#if MALLOC_LOG_BLUEMSX_RUN
    allocLogPrint();
#endif
}

int main(int argc, char **argv)
{
    allocLogStart();

    // Set main thread priority
    LWP_SetThreadPriority(LWP_GetSelf(), 100);

    // Init Wiimote
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);

    // Init SD-Card access
    fatInitDefault();

    // Set current directory to the MSX-root
    archSetCurrentDirectory(MSX_ROOT_DIR);

    // GUI init
    manager = new GuiManager();
    GuiFontInit();
    GuiImageInit();

    // Init console
    console = new GuiConsole(manager, 12, 12, 640-24, 480-24);
#if CONSOLE_DEBUG
    console->SetVisible(true);
#endif

    // Background
    sprBackground = new Sprite;
    sprBackground->SetImage(g_imgBackground);
    sprBackground->SetStretchWidth(4.0f);
    sprBackground->SetStretchHeight(4.0f);
    sprBackground->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprBackground->SetRefPixelPosition(0, 0);
    sprBackground->SetPosition(0, 0);
    manager->AddTop(sprBackground);

    // Please wait...
    msgbox = new GuiMessageBox(manager);
    msgbox->Show("Please wait...");

    // Init blueMSX emulator
    blueMsxInit(1);

    msgbox->Remove();

    char *game_dir = NULL;
    GuiDirSelect *dirs = new GuiDirSelect(manager, "fat:/MSX/Games", "dirlist.xml");

#if MALLOC_LOG_GUI
    allocLogSetMarker();
#endif

    for(;;) {
        // Browse directory
        game_dir = dirs->DoModal();
        if( game_dir == NULL ) {
            delete dirs;
            exit(0);
        }
#if MALLOC_LOG_GUI
        allocLogPrint();
#endif
        // Game menu
        GameElement *game = NULL;
        GameElement *prev;
        for(;;) {
            GuiGameSelect *menu = new GuiGameSelect(manager);
            prev = game;
            game = menu->DoModal(game_dir, "gamelist.xml", prev);
            if( prev != NULL ) {
                delete prev;
            }
            delete menu;

            if( game == NULL ) {
                break;
            }else{
                blueMsxRun(game, game_dir);
            }
        }
#if MALLOC_LOG_GUI
        allocLogPrint();
#endif
    }
    delete dirs;

    printf("Clean-up\n");

    // Destroy emulator
    emulatorExit();
    videoDestroy(video);
    propDestroy(properties);
    archSoundDestroy();
    mixerDestroy(mixer);

    // Destroy background and layer manager
    manager->Remove(sprBackground);
    delete sprBackground;
    delete manager;

    // Free GUI resources
    GuiFontClose();
    GuiImageClose();

    printf("Leaving...\n");

    allocLogStop();

    return 0;
}

