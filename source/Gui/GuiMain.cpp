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
#include "GuiMain.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WII
#include <sys/stat.h>
#include <wiiuse/wpad.h>
#endif
#ifdef UNDER_CE
#include <Win32Wrappers.h>
#endif

#include "../GuiElements/GuiElmFrame.h"
#include "../GuiElements/GuiElmBackground.h"
#include "../GuiDialogs/GuiDlgDirSelect.h"
#include "../GuiDialogs/GuiDlgGameSelect.h"
#include "../GuiDialogs/GuiDlgStateSelect.h"
#include "../GuiDialogs/GuiDlgMenu.h"
#include "../GuiDialogs/GuiDlgMessageBox.h"

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
#include "../Wii/StorageSetup.h"

#include "DirectoryHelper.h"
#include "GuiKeyboard.h"
#include "GuiFonts.h"
#include "GuiImages.h"

#define FORCE_50HZ    0

#define TEX_WIDTH  (512+32)
#define TEX_HEIGHT 480

static Properties* properties;
static Video* video;
static Mixer* mixer;

static int   bitDepth = 16;
static int   zoom = 1;
static int   displayPitch = TEX_WIDTH * 2;

static GuiElmBackground *background = NULL;
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

    DirectoryHelper::SetRootDirectory(rootDir);

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
#ifdef WII
    printf("DISKCHANGE: %d, '%s', '%s'\n", driveId, fileName, fileInZipFile);
#endif
    if( fileInZipFile ) {
        strcpy(currentDisk, fileInZipFile);
    }else{
        strcpy(currentDisk, fileName);
    }
    // Show popup
    GuiDlgMessageBox::ShowPopup(GuiContainer::GetRootContainer(), "diskchange",
                                "image_floppy_disk", 0.75f, 500,
                                GuiEffectFade(10, 0, true),
                                GuiEffectFade(50, 10, true), currentDisk);
}

/*-------------------------*/

GuiMain::GuiMain() :
         GuiRootContainer(),
         effectDefault(10),
         effectZoom(10, 0, true)
{
    // Resources
    GuiImage *image;
    GuiRect rect;
    GuiFontInit();
    GuiImages::Init(this);
    GuiImages::GetImage("image_mousecursor", &image, &rect);
    SetPointerImage(image, rect);
}

GuiMain::~GuiMain()
{
    // Free GUI resources
    GuiFontClose(this);
    GuiImages::Close();
}


void GuiMain::blueMsxInit(int resetProperties)
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
    toolLoadAll(properties->language, this);

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

bool GuiMain::RenderEmuImage(void *context)
{
    if( emulatorGetState() == EMU_RUNNING ) {
        GuiSprite *spr = (GuiSprite *)context;
        void *dpyData = spr->GetTextureBuffer();

        FrameBuffer* frameBuffer;
        frameBuffer = frameBufferFlipViewFrame(properties->emulation.syncMethod == P_EMU_SYNCTOVBLANKASYNC);
        if (frameBuffer == NULL) {
            frameBuffer = frameBufferGetWhiteNoiseFrame();
        }

        videoRender(video, frameBuffer, bitDepth, zoom,
                    dpyData, 0, displayPitch, -1);

        spr->FlushBuffer();
    }
    frameBufferSync();

    return false;
}

void GuiMain::blueMsxRun(GameElement *game, char * game_dir)
{
    int i;

    // Loading message
    GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(this, "loading");
    msgbox->Create(MSGT_TEXT, NULL, 0.5f, "Loading...");
    AddTop(msgbox, effectZoom);

    // Reset properties
    propInitDefaults(properties, 0, P_KBD_EUROPEAN, 0, "");

    // Apply some default video propeties
#if FORCE_50HZ
    properties->emulation.vdpSyncMode = P_VDP_SYNC50HZ;
#else
    if( GetMode() == GW_VIDEO_MODE_NTSC_440 ) {
        properties->emulation.vdpSyncMode = P_VDP_SYNC60HZ;
    }else{
        properties->emulation.vdpSyncMode = P_VDP_SYNCAUTO;
    }
#endif
    properties->emulation.syncMethod = P_EMU_SYNCTOVBLANKASYNC;
    videoUpdateAll(video, properties);

#ifdef WII
    printf("Title        : '%s'\n", game->GetName());
    printf("Command line : '%s'\n", game->GetCommandLine());
    printf("Screen shot 1: '%s'\n", game->GetScreenShot(0));
    printf("Screen shot 2: '%s'\n", game->GetScreenShot(1));
#endif

    // Init keyboard and remap keys
    keyboardReset();
    gwd.input.SetWpadOrientation(WPADO_HORIZONTAL);
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
            sprintf(path, "Tools/Cheats/%s", game->GetCheatFile());
            toolInfoAddArgument(ti, "CheatFile", path);
        } else {
            toolInfoAddArgument(ti, "CheatFile", NULL);
        }
    }

    // Start displaying emlator
    RemoveAndDelete(msgbox, effectDefault);
    GuiSprite *emuSpr = new GuiSprite(this, "emulator");
    emuSpr->CreateDrawImage(TEX_WIDTH, TEX_HEIGHT, GX_TF_RGB565);
    emuSpr->SetScaledWidth(640.0f);
    emuSpr->SetScaledHeight(480.0f);
    emuSpr->SetPosition(0, ((int)GetHeight()-480)/2);
    AddTop(emuSpr, GuiEffectFade(90));
    AddRenderCallback(RenderEmuImage, (void *)emuSpr);

    // Start emulator
    i = emuTryStartWithArguments(properties, game->GetCommandLine(), game_dir);
    if (i < 0) {
        printf("Failed to parse command line\n");
        return;
    }
    if (i == 0) {
        printf("Starting emulation\n");
        emulatorStart(NULL);
    }

    archThreadSleep(2000);
    Hide(background);

    // Create on-screen keyboard
    GuiKeyboard *osk = NULL;
    osk = new GuiKeyboard(this, "keyboard");
    AddTop(osk);

    // Loop while the user hasn't quit
    GuiDlgMenu *menu = new GuiDlgMenu(this, "menu", 5);
    const char *menu_items[] = {
      "Load state",
      "Save state",
      "Screenshot",
      "Cheats",
      "Quit"
    };
    int refresh = 0;
    bool pressed = true;
    GW_VIDEO_MODE prevVideo = this->GetMode();
    bool doQuit = false;
    while(!doQuit) {
        if( prevVideo != GW_VIDEO_MODE_NTSC_440 ) {
            int newrfsh = boardGetRefreshRate();
            if( newrfsh != 0 && newrfsh != refresh ) {
                if( newrfsh==50 ) {
                    SetMode(GW_VIDEO_MODE_PAL50_440);
                }else{
                    SetMode(GW_VIDEO_MODE_PAL60_440);
                }
                emuSpr->SetPosition(0, ((int)GetHeight()-480)/2);
                refresh = newrfsh;
            }
        }
        if( gwd.input.GetButtonStatus(BTN_JOY1_WIIMOTE_HOME) ||
            gwd.input.GetButtonStatus(BTN_JOY2_WIIMOTE_HOME) ||
            gwd.input.GetButtonStatus(BTN_JOY1_CLASSIC_HOME) ||
            gwd.input.GetButtonStatus(BTN_JOY2_CLASSIC_HOME) ||
            gwd.input.GetButtonStatus(BTN_F12) ) {
            if( !pressed ) {
                emulatorSuspend();
                osk->SetEnabled(false);
                gwd.input.SetWpadOrientation(WPADO_VERTICAL);
                bool leave_menu = false;
                do {
                    int selection;
                    menu->Initialize(menu_items, 5, 344);
                    AddTop(menu, effectZoom);
                    SELRET action = menu->DoModal(&selection);
                    Remove(menu, effectZoom);
                    switch( action ) {
                        case SELRET_SELECTED:
                            switch( selection ) {
                                GuiDlgStateSelect *statesel;
                                char *statefile;
                                case 0: /* Load state */
                                    statesel = new GuiDlgStateSelect(this, "statesel", properties, stateDir);
                                    AddTop(statesel, effectDefault);
                                    statefile = statesel->DoModal();
                                    RemoveAndDelete(statesel, effectDefault);
                                    if( statefile ) {
                                        GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(this, "loadstate");
                                        msgbox->Create(MSGT_TEXT, NULL, 0.6f, "Loading state...");
                                        AddTop(msgbox, effectZoom);
                                        emulatorStop();
                                        emulatorStart(statefile);
                                        RemoveAndDelete(msgbox, effectZoom);
                                        leave_menu = true;
                                    }
                                    break;
                                case 1: /* Save state */
                                    msgbox = new GuiDlgMessageBox(this, "savestate");
                                    msgbox->Create(MSGT_TEXT, NULL, 0.6f, "Saving state...");
                                    AddTop(msgbox, effectZoom);
                                    actionQuickSaveState();
                                    archThreadSleep(200);
                                    RemoveAndDelete(msgbox, effectZoom);
                                    GuiDlgMessageBox::ShowPopup(this, "statesaved", NULL, 0.6f, 2000,
                                                                effectZoom, effectZoom, "State saved");
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

                                    msgbox = new GuiDlgMessageBox(this, "savescreenshot");
                                    msgbox->Create(MSGT_TEXT, NULL, 0.6f, "Saving screenshot...");
                                    AddTop(msgbox, effectZoom);
                                    (void)archScreenCaptureToFile(SC_NORMAL, p);
                                    RemoveAndDelete(msgbox, effectZoom);

                                    GuiDlgMessageBox::ShowPopup(this, "screensaved", NULL, 0.6f, 2000,
                                                                effectZoom, effectZoom, "Screenshot saved");
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
                gwd.input.GetButtonEvents(NULL, NULL); // flush
                gwd.input.SetWpadOrientation(WPADO_HORIZONTAL);
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
    gwd.input.SetWpadOrientation(WPADO_VERTICAL);
    Delete(menu);

    // Remove emulator+keyboard from display
    RemoveRenderCallback(RenderEmuImage, (void *)emuSpr);
    RemoveAndDelete(emuSpr, GuiEffectFade(20));
    RemoveAndDelete(osk);

    SetMode(prevVideo);
    Show(background);
}

#ifdef WII
extern bool g_bSDMounted;
extern bool g_bUSBMounted;
#endif

void GuiMain::Main(void)
{
    // Background
    background = new GuiElmBackground(this, "background");
    AddTop(background, effectDefault);

#ifdef WII
    // Init storage access
    if( !g_bSDMounted && !g_bUSBMounted ) {
        // Prepare messagebox
        GuiDlgMessageBox::ShowPopup(this, "nostorage",
                                    NULL, 0.5f, 3000,
                                    effectZoom, effectZoom,
                                    "No Storage (USB/SD-Card) found!");
    } else
    // Init storage access
    if( SetupStorage(this, g_bSDMounted, g_bUSBMounted) ) {
#else
    {
#endif
        // Please wait...
        GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(this, "pleasewait");
        msgbox->Create(MSGT_TEXT, NULL, 0.5f, "Please wait...");
        AddTop(msgbox, GuiEffectFade(500, 0, true, 0.5f, false, 0.5f * GetWidth(), 1.0f * GetHeight()));
        // Init blueMSX emulator
        blueMsxInit(1);

        RemoveAndDelete(msgbox, GuiEffectFade(50, 0, true, 0.5f, false, 0, 0));

        char *game_dir = NULL;
        GuiDlgDirSelect *dirs = new GuiDlgDirSelect(this, "dirselect", "Games", "dirlist.xml");
        dirs->SetVisible(false);
        AddTop(dirs);
        for(;;) {
            Show(dirs, effectDefault);
            // Browse directory
            game_dir = dirs->Run();
            if( game_dir == NULL ) {
                RemoveAndDelete(dirs, effectDefault);
                dirs = NULL;
                break;
            }
            // Game menu
            GameElement *game = NULL;
            GameElement *prev;
            for(;;) {
                // Show the game selector
                GuiDlgGameSelect *menu = new GuiDlgGameSelect(this, "gameselect", background);
                prev = game;
                if( menu->Load(game_dir, "gamelist.xml", prev) ) {
                    Hide(dirs, effectDefault);
                    AddTop(menu, effectDefault);
                    game = menu->DoModal();
                    Remove(menu, effectDefault);
                    if( prev != NULL ) {
                        delete prev;
                    }
                }else{
                    GuiDlgMessageBox::ShowPopup(this, "nogamelist",
                                                NULL, 0.5f, 2000,
                                                effectZoom, effectZoom,
                                                "gamelist.xml not found!");
                }
                Delete(menu);

                if( game == NULL ) {
                    break;
                }else{
                    blueMsxRun(game, game_dir);
                }
            }

        }

#ifdef WII
        printf("Clean-up\n");
#endif
        // Destroy emulator
        emulatorExit();
        toolUnLoadAll();
        videoDestroy(video);
        propDestroy(properties);
        archSoundDestroy();
        mixerDestroy(mixer);
    }
    // Make sure everything is faded out before destroying global resources
    while( IsBusy() ) {
        archThreadSleep(20);
    }
    // Destroy background
    RemoveAndDelete(background);
}

