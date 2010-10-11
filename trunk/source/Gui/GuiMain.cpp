/*****************************************************************************
** $Source: GuiMain.cpp $
**
** $Revision: 0.0 $
**
** $Date: 2010/09/15 00:00:00 $
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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WII
#include <sys/stat.h>
#include <wiiuse/wpad.h>
#endif

#include "GuiBackground.h"
#include "GuiDirSelect.h"
#include "GuiGameSelect.h"
#include "GuiStateSelect.h"
#include "GuiMenu.h"
#include "GuiMessageBox.h"
#include "GuiKeyboard.h"

#include "../Emulator/CommandLine.h"
#include "../Emulator/Properties.h"
#include "../Arch/ArchFile.h"
#include "../VideoRender/VideoRender.h"
#include "../SoundChips/AudioMixer.h"
#include "../IoDevice/Casette.h"
#include "../IoDevice/PrinterIO.h"
#include "../IoDevice/UartIO.h"
#include "../IoDevice/MidiIO.h"
#include "../Board/Machine.h"
#include "../Board/Board.h"
#include "../Emulator/Emulator.h"
#include "../Emulator/FileHistory.h"
#include "../Emulator/Actions.h"
#include "../Language/Language.h"
#include "../Emulator/LaunchFile.h"
#include "../Arch/ArchEvent.h"
#include "../Arch/ArchSound.h"
#include "../Arch/ArchNotifications.h"
#include "../Input/JoystickPort.h"
#include "../Wii/WiiShortcuts.h"
#include "../Arch/ArchThread.h"
#include "../Wii/WiiToolLoader.h"
#include "../Input/InputEvent.h"
#include "../Wii/WiiInput.h"

#include "GuiConsole.h"
#include "GuiContainer.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "../Wii/StorageSetup.h"

#define CONSOLE_DEBUG 0
#define FORCE_50HZ    0

#define TEX_WIDTH  (512+32)
#define TEX_HEIGHT 480

static Properties* properties;
static Video* video;
static Mixer* mixer;

static int   bitDepth = 16;
static int   zoom = 1;
static int   displayPitch = TEX_WIDTH * 2;

static GuiManager *manager = NULL;
static GuiBackground *background = NULL;
#if CONSOLE_DEBUG
static GuiConsole *console = NULL;
#endif
static GuiMessageBox *msgbox = NULL;
static GuiKeyboard *osk = NULL;

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
    assert(msgbox);
    msgbox->ShowPopup(currentDisk, g_imgFloppyDisk, 192);
}

void blueMsxInit(int resetProperties)
{
    int width;
    int height;

    setDefaultPaths(archGetCurrentDirectory());

    properties = propCreate(resetProperties, 0, P_KBD_EUROPEAN, 0, "");

    if (resetProperties == 2) {
        propDestroy(properties);
        return;
    }

    // Load tools
    toolLoadAll(properties->language, manager);

    video = videoCreate();

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

static bool RenderEmuImage(void *context)
{
    if( emulatorGetState() == EMU_RUNNING ) {
        DrawableImage *img = (DrawableImage *)context;
        void *dpyData = img->GetTextureBuffer();

        FrameBuffer* frameBuffer;
        frameBuffer = frameBufferFlipViewFrame(properties->emulation.syncMethod == P_EMU_SYNCTOVBLANKASYNC);
        if (frameBuffer == NULL) {
            frameBuffer = frameBufferGetWhiteNoiseFrame();
        }

        videoRender(video, frameBuffer, bitDepth, zoom,
                    dpyData, 0, displayPitch, -1);

        img->FlushBuffer();
    }
    frameBufferSync();

    return false;
}

#ifndef WII
static char root_dir[256];
char * GetMSXRootPath(void)
{
    return root_dir;
}
#endif

static void blueMsxRun(GameElement *game, char * game_dir, GuiMessageBox *_msgbox)
{
    int i;

    msgbox = _msgbox;

    // Loading message
    msgbox->Show("Loading...");

    // Set current directory to the MSX-root
    archSetCurrentDirectory(GetMSXRootPath());

    // Reset properties
    propInitDefaults(properties, 0, P_KBD_EUROPEAN, 0, "");

    // Apply some default video propeties
#if FORCE_50HZ
    properties->emulation.vdpSyncMode = P_VDP_SYNC50HZ;
#else
    if( manager->GetMode() == GW_VIDEO_MODE_NTSC_440 ) {
        properties->emulation.vdpSyncMode = P_VDP_SYNC60HZ;
    }else{
        properties->emulation.vdpSyncMode = P_VDP_SYNCAUTO;
    }
#endif
    properties->emulation.syncMethod = P_EMU_SYNCTOVBLANKASYNC;
    videoUpdateAll(video, properties);

    printf("Title        : '%s'\n", game->GetName());
    printf("Command line : '%s'\n", game->GetCommandLine());
    printf("Screen shot 1: '%s'\n", game->GetScreenShot(0));
    printf("Screen shot 2: '%s'\n", game->GetScreenShot(1));

    // Init keyboard and remap keys
    keyboardReset();
    manager->gwd.input.SetWpadOrientation(WPADO_HORIZONTAL);
    if( game->GetProperty(GEP_KEYBOARD_JOYSTICK) ) {
        /* Remap WiiMote 1 to keyboard */
        keyboardRemapKey(BTN_JOY1_WIIMOTE_A, EC_SPACE);
        keyboardRemapKey(BTN_JOY1_WIIMOTE_B, EC_NONE);
        keyboardRemapKey(BTN_JOY1_WIIMOTE_1, EC_NONE);
        keyboardRemapKey(BTN_JOY1_WIIMOTE_2, EC_SPACE);
        keyboardRemapKey(BTN_JOY1_CLASSIC_A, EC_SPACE);
        keyboardRemapKey(BTN_JOY1_CLASSIC_B, EC_NONE);
        keyboardRemapKey(BTN_JOY1_UP, EC_UP);
        keyboardRemapKey(BTN_JOY1_DOWN, EC_DOWN);
        keyboardRemapKey(BTN_JOY1_LEFT, EC_LEFT);
        keyboardRemapKey(BTN_JOY1_RIGHT, EC_RIGHT);
        /* Remap WiiMote 2 to joystick 1 */
        keyboardRemapKey(BTN_JOY2_WIIMOTE_A, EC_JOY1_BUTTON1);
        keyboardRemapKey(BTN_JOY2_WIIMOTE_B, EC_JOY1_BUTTON2);
        keyboardRemapKey(BTN_JOY2_WIIMOTE_1, EC_JOY1_BUTTON2);
        keyboardRemapKey(BTN_JOY2_WIIMOTE_2, EC_JOY1_BUTTON1);
        keyboardRemapKey(BTN_JOY2_CLASSIC_A, EC_SPACE);
        keyboardRemapKey(BTN_JOY2_CLASSIC_B, EC_NONE);
        keyboardRemapKey(BTN_JOY2_UP, EC_JOY1_UP);
        keyboardRemapKey(BTN_JOY2_DOWN, EC_JOY1_DOWN);
        keyboardRemapKey(BTN_JOY2_LEFT, EC_JOY1_LEFT);
        keyboardRemapKey(BTN_JOY2_RIGHT, EC_JOY1_RIGHT);
    }
    for(i = 0; i < BTN_LAST; i++) {
        int event = game->GetKeyMapping((BTN)i);
        if( event != EC_NONE ) {
            keyboardRemapKey((BTN)i, event);
        }
    }

    ToolInfo* ti = toolInfoFind("Trainer");
    if( ti ) {
        char path[256];
        if( game->GetCheatFile() ) {
            sprintf(path, "%s/Tools/Cheats/%s", GetMSXRootPath(), game->GetCheatFile());
            toolInfoAddArgument(ti, "CheatFile", path);
        } else {
            toolInfoAddArgument(ti, "CheatFile", NULL);
        }
    }

    // Start displaying emlator
    msgbox->Remove();
    manager->Lock();
    DrawableImage *emuImg = new DrawableImage;
    emuImg->CreateImage(TEX_WIDTH, TEX_HEIGHT, GX_TF_RGB565);
    manager->AddRenderCallback(RenderEmuImage, (void *)emuImg);
    Sprite *emuSpr = new Sprite;
    emuSpr->SetImage(emuImg->GetImage());
    emuSpr->SetStretchWidth(640.0f / (float)TEX_WIDTH);
    emuSpr->SetStretchHeight(1.0f);
    emuSpr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    emuSpr->SetRefPixelPosition(0, 0);
    emuSpr->SetPosition(0, ((int)manager->GetHeight()-480)/2);
    manager->AddTop(emuSpr, 90);
    manager->Unlock();

    // Start emulator
    i = emuTryStartWithArguments(properties, game->GetCommandLine(), game_dir);
    if (i < 0) {
        printf("Failed to parse command line\n");
        msgbox->Remove();
        return;
    }
    if (i == 0) {
        printf("Starting emulation\n");
        emulatorStart(NULL);
    }

    archThreadSleep(2000);
    background->Hide();

    // Create on-screen keyboard
    GuiKeyboard *osk = NULL;
    osk = new GuiKeyboard(manager);

    // Loop while the user hasn't quit
    GuiMenu *menu = new GuiMenu(manager, 5);
    const char *menu_items[] = {
      "Load state",
      "Save state",
      "Screenshot",
      "Cheats",
      "Quit"
    };
    int refresh = 0;
    bool pressed = true;
    GW_VIDEO_MODE prevVideo = manager->GetMode();
    bool doQuit = false;
    while(!doQuit) {
        if( prevVideo != GW_VIDEO_MODE_NTSC_440 ) {
            int newrfsh = boardGetRefreshRate();
            if( newrfsh != 0 && newrfsh != refresh ) {
                if( newrfsh==50 ) {
                    manager->SetMode(GW_VIDEO_MODE_PAL50_440);
                }else{
                    manager->SetMode(GW_VIDEO_MODE_PAL60_440);
                }
                emuSpr->SetPosition(0, ((int)manager->GetHeight()-480)/2);
                refresh = newrfsh;
            }
        }
        if( manager->gwd.input.GetButtonStatus(BTN_JOY1_WIIMOTE_HOME) ||
            manager->gwd.input.GetButtonStatus(BTN_JOY2_WIIMOTE_HOME) ||
            manager->gwd.input.GetButtonStatus(BTN_JOY1_CLASSIC_HOME) ||
            manager->gwd.input.GetButtonStatus(BTN_JOY2_CLASSIC_HOME) ||
            manager->gwd.input.GetButtonStatus(BTN_F12) ) {
            if( !pressed ) {
                emulatorSuspend();
                osk->SetEnabled(false);
                manager->gwd.input.SetWpadOrientation(WPADO_VERTICAL);
                bool leave_menu = false;
                do {
                    int selection;
                    SELRET action = menu->DoModal(&selection, menu_items, 5, 344);
                    switch( action ) {
                        case SELRET_SELECTED:
                            switch( selection ) {
                                GuiStateSelect *statesel;
                                char *statefile;
                                case 0: /* Load state */
                                    statesel = new GuiStateSelect(manager);
                                    statefile = statesel->DoModal(properties, stateDir);
                                    if( statefile ) {
                                        msgbox->Show("Loading state...", NULL, MSGT_TEXT, 160);
                                        emulatorStop();
                                        emulatorStart(statefile);
                                        msgbox->Remove();
                                        leave_menu = true;
                                    }
                                    delete statesel;
                                    break;
                                case 1: /* Save state */
                                    msgbox->Show("Saving state...", NULL, MSGT_TEXT, 160);
                                    actionQuickSaveState();
                                    msgbox->Remove();
                                    msgbox->Show("State saved", NULL, MSGT_TEXT, 160);
                                    archThreadSleep(2000);
                                    msgbox->Remove();
                                    break;
                                case 2: /* Screenshot */
                                    char *p, fname1[256], fname2[256];
                                    strcpy(fname1, game_dir);
                                    strcat(fname1, "/Screenshots/");
                                    strcat(fname1, game->GetScreenShot(0));
                                    strcpy(fname2, game_dir);
                                    strcat(fname2, "/Screenshots/");
                                    strcat(fname2, game->GetScreenShot(1));
                                    if( !archFileExists(fname1) ) {
                                        p = fname1;
                                        /* file does not exist, make sure there is a Screenshots directory */
                                        char scrshotdir[256];
                                        strcpy(scrshotdir, game_dir);
                                        strcat(scrshotdir, "/Screenshots");
#ifdef WII
                                        struct stat s;
                                        if( stat(scrshotdir, &s) != 0 ) {
                                            mkdir(scrshotdir, 0x777);
                                        }
#else
                                        CreateDirectoryA(scrshotdir, NULL);
#endif
                                    }else
                                    if( !archFileExists(fname2) ) {
                                        p = fname2;
                                    }else{
                                        p = generateSaveFilename(properties, screenShotDir, "", ".png", 2);
                                    }
                                    msgbox->Show("Saving screenshot...", NULL, MSGT_TEXT, 160);
                                    (void)archScreenCaptureToFile(SC_NORMAL, p);
                                    msgbox->Remove();
                                    msgbox->Show("Screenshot saved", NULL, MSGT_TEXT, 160);
                                    archThreadSleep(2000);
                                    msgbox->Remove();
                                    break;
                                case 3: /* Cheats */
                                    actionToolsShowTrainer();
                                    break;
                                case 4: /* Quit */
                                    doQuit = true;
                                    leave_menu = true;
                                    break;
                            }
                            break;
                        case SELRET_KEY_B:
                        case SELRET_KEY_HOME:
                            leave_menu = true;
                            break;
                        default:
                            break;
                    }
                }while(!leave_menu);
                manager->gwd.input.GetButtonEvents(NULL, NULL); // flush
                manager->gwd.input.SetWpadOrientation(WPADO_HORIZONTAL);
                emulatorResume();
                osk->SetEnabled(!doQuit);
                pressed = true;
            }
        }else{
            pressed = false;
        }
        // wait a frame
#ifdef WII
        VIDEO_WaitVSync();
#else
        archThreadSleep(20);
#endif
    }
    emulatorStop();
    manager->gwd.input.SetWpadOrientation(WPADO_VERTICAL);
    delete menu;

    // Remove emulator+keyboard from display
    manager->Lock();
    manager->RemoveRenderCallback(RenderEmuImage, (void *)emuImg);
    manager->RemoveAndDelete(emuSpr, emuImg, 20);
    delete osk;
    manager->Unlock();

    manager->SetMode(prevVideo);
    background->Show();
}

#ifdef WII
extern bool g_bSDMounted;
extern bool g_bUSBMounted;
#endif

void GuiMain(GuiManager *man)
{
    manager = man;

#ifndef WII
    GetCurrentDirectoryA(sizeof(root_dir), root_dir);
    strcat(root_dir, "\\..\\MSX");
#endif

    // Resources
    GuiFontInit();
    GuiImageInit();

    // Init console
#if CONSOLE_DEBUG
    console = new GuiConsole(manager, 12, 12, 640-24, 448-24);
    console->SetVisible(true);
#endif

    // Background
    background = new GuiBackground(manager);
    background->Show();

#ifdef WII
    // Init storage access
    if( !g_bSDMounted && !g_bUSBMounted ) {
        // Prepare messagebox
        GuiMessageBox *msgboxSdSetup = new GuiMessageBox(manager);
        msgboxSdSetup->Show("No Storage (USB/SD-Card) found!");
        archThreadSleep(3000);
        delete msgboxSdSetup;
    } else
    // Init storage access
    if( SetupStorage(manager, g_bSDMounted, g_bUSBMounted) ) {
#else
    {
#endif
        // Set current directory to the MSX-root
        archSetCurrentDirectory(GetMSXRootPath());

        // Please wait...
        GuiMessageBox *msgbox = NULL;
        msgbox = new GuiMessageBox(manager);
        msgbox->Show("Please wait...");
        // Init blueMSX emulator
        blueMsxInit(1);

        msgbox->Remove();

        char *game_dir = NULL;
        char sGamesPath[100];
        sprintf(sGamesPath, "%s/Games", GetMSXRootPath());
        GuiDirSelect *dirs = new GuiDirSelect(manager, sGamesPath, "dirlist.xml");

        for(;;) {
            // Browse directory
            game_dir = dirs->DoModal();
            if( game_dir == NULL ) {
                break;
            }
            // Game menu
            GameElement *game = NULL;
            GameElement *prev;
            for(;;) {
                GuiGameSelect *menu = new GuiGameSelect(manager, background);
                prev = game;
                if( menu->Load(game_dir, "gamelist.xml") ) {
                    game = menu->DoModal(prev);
                    if( prev != NULL ) {
                        delete prev;
                    }
                }else{
                    msgbox->Show("gamelist.xml not found!");
                    archThreadSleep(2000);
                    msgbox->Remove();
                }
                delete menu;

                if( game == NULL ) {
                    break;
                }else{
                    blueMsxRun(game, game_dir, msgbox);
                }
            }
        }
        delete dirs;

         printf("Clean-up\n");

        // Destroy message box
        delete msgbox;

        // Destroy emulator
        emulatorExit();
        toolUnLoadAll();
        videoDestroy(video);
        propDestroy(properties);
        archSoundDestroy();
        mixerDestroy(mixer);
    }
    // Destroy background and layer manager
    delete background;

#if CONSOLE_DEBUG
    // Destroy console
    delete console;
#endif
    // Make sure everything is faded out before destroying global resources
    archThreadSleep(500);
    // Free GUI resources
    GuiFontClose();
    GuiImageClose();
}

