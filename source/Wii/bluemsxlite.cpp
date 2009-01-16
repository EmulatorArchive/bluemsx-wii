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

#include "GuiDirSelect.h"
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

#include "GuiConsole.h"
#include "GuiContainer.h"
#include "GuiFonts.h"
#include "GuiImages.h"

#define CONSOLE_DEBUG 1

#define MSX_ROOT_DIR "fat:/MSX"

static Properties* properties;
static Video* video;
static Mixer* mixer;
static Shortcuts* shortcuts;

static int   bitDepth = 16;
static int   zoom = 1;
static int   displayPitch = TEX_WIDTH * 2;

static int   g_doQuit = 0;

static LayerManager *manager = NULL;
static GameWindow gwd;
static char *g_dpyData = NULL;
static int g_yOffset = 0;
static void *g_vidSemaphore = NULL;
static GuiConsole *console = NULL;

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
        FrameBuffer* frameBuffer;
        archSemaphoreWait(g_vidSemaphore, -1);
        if( g_dpyData != NULL ) {
        
            frameBuffer = frameBufferFlipViewFrame(properties->emulation.syncMethod == P_EMU_SYNCTOVBLANKASYNC);
            if (frameBuffer == NULL) {
                frameBuffer = frameBufferGetWhiteNoiseFrame();
            }
            
            videoRender(video, frameBuffer, bitDepth, zoom,
                        g_dpyData, 0, displayPitch, -1);
            DCFlushRange(g_dpyData, TEX_WIDTH * TEX_HEIGHT * 4);
        }
        if( console != NULL && console->IsVisible() ) {
            console->Render();
        }
        manager->Draw(0, g_yOffset);
        archSemaphoreSignal(g_vidSemaphore);
        gwd.Flush();
    }
}

int main(int argc, char **argv)
{
    int resetProperties;
    int i;
    int width;
    int height;

    // Initialize GameWindow
	gwd.InitVideo();
	gwd.SetBackground((GXColor){ 0, 0, 0, 255 });

    // Init Wiimote
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);

    // Init SD-Card access
    fatInitDefault();

    // GUI init
    GuiFontInit();
    GuiImageInit();

    GameElement *game = NULL;
    char *game_dir = NULL;
    GuiDirSelect *dirs = new GuiDirSelect(&gwd, "fat:/MSX/Games", "dirlist.xml");
    for(;;) {
        // Browse directory
        game_dir = dirs->DoModal();
        if( game_dir == NULL ) {
            delete dirs;
            exit(0);
        }

        // Game menu
        GuiMenu *menu = new GuiMenu();
        game = menu->DoModal(&gwd, game_dir, "gamelist.xml");
        delete menu;
        if( game != NULL ) {
            break;
        }
    }
    game_dir = strdup(game_dir);
    delete dirs;

    // Set current directory back to the MSX-root
    archSetCurrentDirectory(MSX_ROOT_DIR);

    // Initialize manager
    manager = new LayerManager(3+2);

    DrawableImage *txtImg = new DrawableImage;
    txtImg->CreateImage(144, 60);
    txtImg->SetFont(g_fontArial);
    txtImg->SetSize(32);
    txtImg->SetColor((GXColor){255,255,255,255});
    txtImg->RenderText("Loading...");
    Sprite *txtSpr = new Sprite;
    txtSpr->SetImage(txtImg->GetImage());
    txtSpr->SetPosition(320-72, 240-30);
    manager->Append(txtSpr);

    // Container
    GuiContainer *grWinList = new GuiContainer(320-200, 240-80, 400, 160);
    manager->Append(grWinList->GetLayer());

    // Background
    Sprite *sprBackground = new Sprite;
    sprBackground->SetImage(g_imgBackground);
    sprBackground->SetPosition(0, 0);
    manager->Append(sprBackground);

    manager->Draw(0,0);
    gwd.Flush();

    console = new GuiConsole(manager, 12, 12, 640-24, 480-24);
#if CONSOLE_DEBUG
    console->SetVisible(true);
#endif

    g_vidSemaphore = archSemaphoreCreate(1);
    void *disp_thread = archThreadCreate(displayThread, THREAD_PRIO_NORMAL);

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
    //NOTE: Disable Moonsound right now since we allready have memory resource problems...
    //      ... and the Moonsound needs a lot of memory because of its wave table rom.
    boardSetMoonsoundEnable(0 /*properties->sound.chip.enableMoonsound*/);
    boardSetVideoAutodetect(properties->video.detectActiveMonitor);

    i = emuTryStartWithArguments(properties, game->GetCommandLine(), game_dir);
    free(game_dir);
    delete game;
    if (i < 0) {
        printf("Failed to parse command line\n");
        return 0;
    }

    if (i == 0) {
        printf("Starting emulation\n");
        emulatorStart(NULL);
    }

    printf("Waiting for quit event...\n");

    archSemaphoreWait(g_vidSemaphore, -1);
    manager->Remove(grWinList->GetLayer());
    manager->Remove(txtSpr);
    manager->Remove(sprBackground);
    delete sprBackground;
    g_yOffset = -37;
    console->SetPosition(12, 12+37);
    DrawableImage emuImg;
    emuImg.CreateImage(TEX_WIDTH, TEX_HEIGHT, GX_TF_RGB565);
    g_dpyData = (char *)emuImg.GetTextureBuffer();
    Sprite emuSpr;
    emuSpr.SetImage(emuImg.GetImage());
    emuSpr.SetStretchWidth(640.0f / (float)TEX_WIDTH);
    emuSpr.SetStretchHeight(548.0f / (float)480);
    emuSpr.SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    emuSpr.SetRefPixelPosition(0, 0);
    emuSpr.SetPosition(0, 0);
    manager->Append(&emuSpr);
    archSemaphoreSignal(g_vidSemaphore);

    // Turn off console by default
#if CONSOLE_DEBUG
    console->SetVisible(false);
#endif

    // Loop while the user hasn't quit
    for(;;) {
        if( KBD_GetKeyStatus(kbdHandle, KEY_JOY1_HOME) ) {
            break;
        }
        if( KBD_GetKeyStatus(kbdHandle, KEY_JOY1_MINUS) ) {
            if( console->IsVisible() ) {
                console->SetVisible(false);
            }
        }
        if( KBD_GetKeyStatus(kbdHandle, KEY_JOY1_PLUS) ) {
            if( !console->IsVisible() ) {
                console->SetVisible(true);
            }
        }
        archThreadSleep(100);
    }
    g_doQuit = 1;
    archThreadJoin(disp_thread, -1);
    archSemaphoreDestroy(g_vidSemaphore);

    fprintf(stderr, "Clean-up\n");

    delete manager;

    // GUI DeInit
    GuiFontClose();
    GuiImageClose();

    videoDestroy(video);
    propDestroy(properties);
    archSoundDestroy();
    mixerDestroy(mixer);

    fprintf(stderr, "Leaving...\n");

    return 0;
}
