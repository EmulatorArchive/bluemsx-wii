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

#include "GuiMenu.h"
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
#include "ArchSound.h"
#include "ArchNotifications.h"
#include "JoystickPort.h"
#include "WiiShortcuts.h"
#include "ArchThread.h"
#include "ogc_video.h"
#include "kbdlib.h"

void WiiTimerInit(void);
void WiiTimerDestroy(void);

void keyboardSetDirectory(char* directory);
void keyboardInit();
void keyboardSetFocus(int handle, int focus);
void keyboardUpdate();
int keyboardGetModifiers();
}

static Properties* properties;
static Video* video;
static Mixer* mixer;
static Shortcuts* shortcuts;

static int   bitDepth = 16;
static int   zoom = 1;
static int   displayPitch = TEX_WIDTH * 2;

static int g_doQuit = 0;

extern KBDHANDLE kbdHandle;

#define WIDTH  320
#define HEIGHT 240

#define EVENT_UPDATE_DISPLAY 2
#define EVENT_UPDATE_WINDOW  3

int updateEmuDisplay(int updateAll)
{
    FrameBuffer* frameBuffer;
    char* dpyData  = (char *)texturemem;

    frameBuffer = frameBufferFlipViewFrame(properties->emulation.syncMethod == P_EMU_SYNCTOVBLANKASYNC);
    if (frameBuffer == NULL) {
        frameBuffer = frameBufferGetWhiteNoiseFrame();
    }

    VIDEO_WaitVSync();

    videoRender(video, frameBuffer, bitDepth, zoom,
                dpyData, 0, displayPitch, -1);

    ogc_video__update();

    return 0;
}

int  archUpdateEmuDisplay(int syncMode)
{
    return 1;
}

void archUpdateWindow()
{
}

void setDefaultPaths(const char* rootDir)
{
    char buffer[512];

    propertiesSetDirectory(rootDir, rootDir);

    sprintf(buffer, "%s/Audio Capture", rootDir);
    archCreateDirectory(buffer);
    actionSetAudioCaptureSetDirectory(buffer, "");

    sprintf(buffer, "%s/QuickSave", rootDir);
    archCreateDirectory(buffer);
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

    sprintf(buffer, "%s/Shortcut Profiles", rootDir);
    archCreateDirectory(buffer);
    shortcutsSetDirectory(buffer);
}

static void displayThread(void)
{
    while(!g_doQuit) {
        updateEmuDisplay(1);
    }
}

int main(int argc, char **argv)
{
    int resetProperties;
    int i;
    int width;
    int height;
    void* disp_thread;

	// Initialise the video system
    //ogc_video__init(0, 0, 0);

    // Init Wiimote
	WPAD_Init();

    // Init SD-Card access
    fatInitDefault();

    // Menu
    archSetCurrentDirectory("fat:/MSX/Games");
    GuiMenu *menu = new GuiMenu();
    GameElement *game = menu->DoModal("gamelist.xml");
    delete menu;
    if( game == NULL ) {
        exit(0);
    }
    archSetCurrentDirectory("fat:/MSX");

    // Init Console
    ogc_video__init(0, 0, 0);

    printf("Title        : '%s'\n", game->GetName());
    printf("Command line : '%s'\n", game->GetCommandLine());
    printf("Screen shot 1: '%s'\n", game->GetScreenShot(0));
    printf("Screen shot 2: '%s'\n", game->GetScreenShot(1));

    // Init timer
    WiiTimerInit();
    atexit(WiiTimerDestroy);

    // Set main thread priority
    LWP_SetThreadPriority(LWP_GetSelf(), 100);

    printf("Setting/creating default paths\n");

    setDefaultPaths(archGetCurrentDirectory());

    printf("Check and reset arguments\n");

    resetProperties = emuCheckResetArgument(game->GetCommandLine());

    printf("Create properties\n");

    properties = propCreate(resetProperties, 0, P_KBD_EUROPEAN, 0, "");

    properties->emulation.syncMethod = P_EMU_SYNCTOVBLANKASYNC;

    if (resetProperties == 2) {
        propDestroy(properties);
        return 0;
    }

    printf("Initialize video renderer\n");
    video = videoCreate();

    videoUpdateAll(video, properties);

    printf("Create video window\n");

    if (properties->video.windowSize == P_VIDEO_SIZEFULLSCREEN) {
        zoom = properties->video.fullscreen.width / WIDTH;
        bitDepth = properties->video.fullscreen.bitDepth;
        printf("Video mode: Full-screen, %dbit\n", bitDepth);
    }
    else {
        if (properties->video.windowSize == P_VIDEO_SIZEX1) {
            printf("Video mode: 1x, 16bit\n");
            zoom = 1;
        }
        else {
            printf("Video mode: 2x, 16bit\n");
            zoom = 2;
        }
        bitDepth = 16;
    }

    width  = zoom * WIDTH;
    height = zoom * HEIGHT;
    printf("Resolution: %dx%d\n", width, height);

    printf("Initialize keyboard\n");

    keyboardInit();

    printf("Initialize mixer\n");

    mixer = mixerCreate();

    printf("Initialize emulator\n");

    emulatorInit(properties, mixer);

    actionInit(video, properties, mixer);

    printf("Initialize languages\n");

    langInit();

    printf("Initialize tape\n");

    tapeSetReadOnly(properties->cassette.readOnly);

    printf("Setting language to %d\n", properties->language);

    langSetLanguage((EmuLanguageType)properties->language);

    printf("Setting types\n");

    joystickPortSetType(0, (JoystickPortType)properties->joy1.typeId);
    joystickPortSetType(1, (JoystickPortType)properties->joy2.typeId);
    printerIoSetType((PrinterType)properties->ports.Lpt.type, properties->ports.Lpt.fileName);
    printerIoSetType((PrinterType)properties->ports.Lpt.type, properties->ports.Lpt.fileName);
    uartIoSetType((UartType)properties->ports.Com.type, properties->ports.Com.fileName);
    midiIoSetMidiOutType((MidiType)properties->sound.MidiOut.type, properties->sound.MidiOut.fileName);
    midiIoSetMidiInType((MidiType)properties->sound.MidiIn.type, properties->sound.MidiIn.fileName);
    ykIoSetMidiInType((MidiType)properties->sound.YkIn.type, properties->sound.YkIn.fileName);

    printf("Starting sound\n");

    emulatorRestartSound();

    for (i = 0; i < MIXER_CHANNEL_TYPE_COUNT; i++) {
        mixerSetChannelTypeVolume(mixer, i, properties->sound.mixerChannel[i].volume);
        mixerSetChannelTypePan(mixer, i, properties->sound.mixerChannel[i].pan);
        mixerEnableChannelType(mixer, i, properties->sound.mixerChannel[i].enable);
    }

    mixerSetMasterVolume(mixer, properties->sound.masterVolume);
    mixerEnableMaster(mixer, properties->sound.masterEnable);

    printf("Update video\n");

    videoUpdateAll(video, properties);

    shortcuts = shortcutsCreate();

    mediaDbSetDefaultRomType(properties->cartridge.defaultType);

    printf("Loading ROMs\n");
    for (i = 0; i < PROP_MAX_CARTS; i++) {
        if (properties->media.carts[i].fileName[0]) insertCartridge(properties, i, properties->media.carts[i].fileName, properties->media.carts[i].fileNameInZip, properties->media.carts[i].type, -1);
        updateExtendedRomName(i, properties->media.carts[i].fileName, properties->media.carts[i].fileNameInZip);
    }

    printf("Loading Disks\n");
    for (i = 0; i < PROP_MAX_DISKS; i++) {
        if (properties->media.disks[i].fileName[0]) insertDiskette(properties, i, properties->media.disks[i].fileName, properties->media.disks[i].fileNameInZip, -1);
        updateExtendedDiskName(i, properties->media.disks[i].fileName, properties->media.disks[i].fileNameInZip);
    }

    printf("Loading Tapes\n");
    for (i = 0; i < PROP_MAX_TAPES; i++) {
        if (properties->media.tapes[i].fileName[0]) insertCassette(properties, i, properties->media.tapes[i].fileName, properties->media.tapes[i].fileNameInZip, 0);
        updateExtendedCasName(i, properties->media.tapes[i].fileName, properties->media.tapes[i].fileNameInZip);
    }

    printf("Create machine\n");
    {
        Machine* machine = machineCreate(properties->emulation.machineName);
        if (machine != NULL) {
            boardSetMachine(machine);
            machineDestroy(machine);
        }
    }
    boardSetFdcTimingEnable(properties->emulation.enableFdcTiming);
    boardSetY8950Enable(properties->sound.chip.enableY8950);
    boardSetYm2413Enable(properties->sound.chip.enableYM2413);
    boardSetMoonsoundEnable(properties->sound.chip.enableMoonsound);
    boardSetVideoAutodetect(properties->video.detectActiveMonitor);

    i = emuTryStartWithArguments(properties, game->GetCommandLine());
    if (i < 0) {
        printf("Failed to parse command line\n");
        return 0;
    }

    if (i == 0) {
        printf("Starting emulation\n");
        emulatorStart(NULL);
    }

    printf("Waiting for quit event...\n");

    disp_thread = archThreadCreate(displayThread, THREAD_PRIO_NORMAL);

    // Start displaying error messages top-left
    printf("\x1b[2;0H");

    // Loop while the user hasn't quit
    while(!g_doQuit) {
        if( KBD_GetKeyStatus(kbdHandle, KEY_JOY1_HOME) ) {
            g_doQuit = 1;
        }
        archThreadSleep(100);
    }

    fprintf(stderr, "Clean-up\n");

    videoDestroy(video);
    propDestroy(properties);
    archSoundDestroy();
    mixerDestroy(mixer);

    fprintf(stderr, "Leaving...\n");

    return 0;
}
