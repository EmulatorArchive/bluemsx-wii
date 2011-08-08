/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Win32/Win32ToolLoader.c,v $
**
** $Revision: 1.26 $
**
** $Date: 2009-07-01 05:00:23 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik, Tomas Karlsson
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "Win32ToolLoader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Tools/BlueMSXToolInterface.h"
#include "../Debugger/Debugger.h"
#include "../Emulator/Actions.h"
#include "../Emulator/AppConfig.h"
#include "version.h"

/* include each tool (instead of dynamic interface) */
#include "../Tools/Trainer/Trainer.h"

#ifndef NO_TOOL_SUPPORT

#define MAX_TOOLS 16
#ifndef MAX_PATH
#define MAX_PATH  256
#endif

struct ToolInfo {
    char description[32];
    BlueDebugger* debugger;

    struct {
        NotifyFn  show;
        NotifyFn  destroy;
        NotifyFn  onEmulatorStart;
        NotifyFn  onEmulatorStop;
        NotifyFn  onEmulatorPause;
        NotifyFn  onEmulatorResume;
        NotifyFn  onEmulatorReset;
        TraceFn   onEmulatorTrace;
        SetBpFn   onEmulatorSetBp;
        SetLgFn   onEmulatorSetLg;
        ArgFn     onAddArgument;
        GetNameFn getName;
    } callbacks;
};

static ToolInfo* toolList[MAX_TOOLS];
static int       toolListCount = 0;
static volatile int isUpdating = 0;
static char toolDir[MAX_PATH] = "";

static void onEmulatorStart(ToolInfo* toolInfo) {
    if (!isUpdating && toolInfo->callbacks.onEmulatorStart != NULL) {
        toolInfo->callbacks.onEmulatorStart();
    }
}

static void onEmulatorStop(ToolInfo* toolInfo) {
    if (!isUpdating && toolInfo->callbacks.onEmulatorStop != NULL) {
        toolInfo->callbacks.onEmulatorStop();
    }
}

static void onEmulatorPause(ToolInfo* toolInfo) {
    if (!isUpdating && toolInfo->callbacks.onEmulatorPause != NULL) {
        toolInfo->callbacks.onEmulatorPause();
    }
}

static void onEmulatorResume(ToolInfo* toolInfo) {
    if (!isUpdating && toolInfo->callbacks.onEmulatorResume != NULL) {
        toolInfo->callbacks.onEmulatorResume();
    }
}

static void onEmulatorReset(ToolInfo* toolInfo) {
    if (!isUpdating && toolInfo->callbacks.onEmulatorReset != NULL) {
        toolInfo->callbacks.onEmulatorReset();
    }
}

static void onEmulatorTrace(ToolInfo* toolInfo, const char* str) {
    if (toolInfo->callbacks.onEmulatorTrace != NULL) {
        toolInfo->callbacks.onEmulatorTrace(str);
    }
}

static void onEmulatorSetBp(ToolInfo* toolInfo, UInt16 slot, UInt16 page, UInt16 address) {
    if (toolInfo->callbacks.onEmulatorSetBp != NULL) {
        toolInfo->callbacks.onEmulatorSetBp(slot, page, address);
    }
}

Snapshot* toolSnapshotCreate() {
    if (dbgGetState() == DBG_RUNNING) {
        Snapshot* snapshot;
        isUpdating = 1;
        actionEmuTogglePause();
        snapshot = (Snapshot*)dbgSnapshotCreate(NULL);
        actionEmuTogglePause();
        isUpdating = 0;
        return snapshot;
    }
    return (Snapshot*)dbgSnapshotCreate(NULL);
}

void toolSnapshotDestroy(Snapshot* s) {
    dbgSnapshotDestroy((DbgSnapshot*)s);
}

EmulatorState toolGetState() {
    return dbgGetState();
}

int toolSnapshotGetDeviceCount(Snapshot* s)
{
    return dbgSnapshotGetDeviceCount((DbgSnapshot*)s);
}

Device* toolSnapshotGetDevice(Snapshot* s, int d)
{
    return (Device*)dbgSnapshotGetDevice((DbgSnapshot*)s, d);
}

int toolDeviceGetMemoryBlockCount(Device* d)
{
    return dbgDeviceGetMemoryBlockCount((DbgDevice*)d);
}

MemoryBlock* toolDeviceGetMemoryBlock(Device* d, int m)
{
    return (MemoryBlock*)dbgDeviceGetMemoryBlock((DbgDevice*)d, m);
}

int toolDeviceGetRegisterBankCount(Device* d)
{
    return dbgDeviceGetRegisterBankCount((DbgDevice*)d);
}

RegisterBank* toolDeviceGetRegisterBank(Device* d, int r)
{
    return (RegisterBank*)dbgDeviceGetRegisterBank((DbgDevice*)d, r);
}

int toolDeviceGetCallstackCount(Device* d)
{
    return dbgDeviceGetCallstackCount((DbgDevice*)d);
}

Callstack* toolDeviceGetCallstack(Device* d, int c)
{
    return (Callstack*)dbgDeviceGetCallstack((DbgDevice*)d, c);
}

int toolDeviceGetIoPortsCount(Device* d)
{
    return dbgDeviceGetIoPortsCount((DbgDevice*)d);
}

IoPorts* toolDeviceGetIoPorts(Device* d, int p)
{
    return (IoPorts*)dbgDeviceGetIoPorts((DbgDevice*)d, p);
}

void toolRun()
{
    dbgRun();
}

void toolStop()
{
    dbgStop();
}

void toolPause()
{
    dbgPause();
}

void toolStep()
{
    dbgStep();
}

void toolSetBreakpoint(UInt16 address)
{
    dbgSetBreakpoint(address);
}

void toolEnableVramAccessCheck(int enable)
{
    dbgEnableVramAccessCheck(enable);
}

void toolClearBreakpoint(UInt16 address)
{
    dbgClearBreakpoint(address);
}

int toolWriteMemory(MemoryBlock* memoryBlock, void* data, int startAddr, int size)
{
    return dbgDeviceWriteMemory((DbgMemoryBlock*)memoryBlock, data, startAddr, size);
}

int toolWriteRegister(RegisterBank* regBank, int regIndex, UInt32 value)
{
    return dbgDeviceWriteRegister((DbgRegisterBank*)regBank, regIndex, value);
}

int toolDeviceWriteIoPort(IoPorts* ioPorts, int portIndex, UInt32 value)
{
    return dbgDeviceWriteIoPort((DbgIoPorts*)ioPorts, portIndex, value);
}

char* toolGetPath()
{
    return toolDir;
}

void toolGetEmulatorVersion(int* major, int* minor, int* buildNumber)
{
    *major = 1;
    *minor = 0;
    *buildNumber = 120;
}

static Interface toolInterface = {
    toolSnapshotCreate,
    toolSnapshotDestroy,
    toolGetState,
    toolSnapshotGetDeviceCount,
    toolSnapshotGetDevice,
    toolDeviceGetMemoryBlockCount,
    toolDeviceGetMemoryBlock,
    toolWriteMemory,
    toolDeviceGetRegisterBankCount,
    toolDeviceGetRegisterBank,
    toolWriteRegister,
    toolDeviceGetCallstackCount,
    toolDeviceGetCallstack,
    toolDeviceGetIoPortsCount,
    toolDeviceGetIoPorts,
    toolDeviceWriteIoPort,
    toolRun,
    toolStop,
    toolPause,
    toolStep,
    toolSetBreakpoint,
    toolClearBreakpoint,
    toolGetPath,
    toolGetEmulatorVersion,
    toolEnableVramAccessCheck,
};

void toolLoadAll(int languageId, void /*GuiRootContainer*/ *container)
{
    ToolInfo* toolInfo;

    char description[32] = "Unknown";
    CreateFn  create   = Trainer_Create;
    NotifyFn  destroy  = Trainer_Destroy;
    NotifyFn  show     = Trainer_Show;
    NotifyFn  onStart  = Trainer_NotifyEmulatorStart;
    NotifyFn  onStop   = Trainer_NotifyEmulatorStop;
    NotifyFn  onPause  = Trainer_NotifyEmulatorPause;
    NotifyFn  onResume = Trainer_NotifyEmulatorResume;
    NotifyFn  onReset  = Trainer_NotifyEmulatorReset;
    TraceFn   onTrace  = Trainer_EmulatorTrace;
    SetBpFn   onSetBp  = Trainer_EmulatorSetBreakpoint;
    SetLgFn   onSetLg  = Trainer_SetLanguage;
    ArgFn     onArg    = Trainer_AddArgument;
    GetNameFn onGetNm  = Trainer_GetName;

    if (create(&toolInterface, description, 31) == 0) {
        return;
    }

    toolInfo = malloc(sizeof(ToolInfo));
    toolInfo->callbacks.destroy           = destroy;
    toolInfo->callbacks.show              = show;
    toolInfo->callbacks.onEmulatorStart   = onStart;
    toolInfo->callbacks.onEmulatorStop    = onStop;
    toolInfo->callbacks.onEmulatorPause   = onPause;
    toolInfo->callbacks.onEmulatorResume  = onResume;
    toolInfo->callbacks.onEmulatorReset   = onReset;
    toolInfo->callbacks.onEmulatorTrace   = onTrace;
    toolInfo->callbacks.onEmulatorSetBp   = onSetBp;
    toolInfo->callbacks.onEmulatorSetLg   = onSetLg;
    toolInfo->callbacks.onAddArgument     = onArg;
    toolInfo->callbacks.getName           = onGetNm;
    toolInfo->debugger = debuggerCreate(onStart  ? onEmulatorStart  : NULL,
                                        onStop   ? onEmulatorStop   : NULL,
                                        onPause  ? onEmulatorPause  : NULL,
                                        onResume ? onEmulatorResume : NULL,
                                        onReset  ? onEmulatorReset  : NULL,
                                        onTrace  ? onEmulatorTrace  : NULL,
                                        onSetBp  ? onEmulatorSetBp  : NULL,
                                        toolInfo);
    strcpy(toolInfo->description, description);

    toolList[toolListCount++] = toolInfo;

    toolInfoSetLanguage(toolInfo, languageId);
    toolInfoAddArgument(toolInfo, "GuiContainer", container);
}

void toolUnLoadAll()
{
    int i;

    if (appConfigGetInt("toolsenable", 1) == 0) {
        return;
    }

    for (i = 0; i < toolListCount; i++) {
        if (toolList[i]->callbacks.destroy != NULL) {
            toolList[i]->callbacks.destroy();
        }
        debuggerDestroy(toolList[i]->debugger);
        free(toolList[i]);
    }
	toolListCount = 0;
}

int toolGetCount() {
    if (appConfigGetInt("toolsenable", 1) == 0) {
        return 0;
    }

    return toolListCount;
}

ToolInfo* toolInfoGet(int index)
{
    if (appConfigGetInt("toolsenable", 1) == 0) {
        return NULL;
    }

    if (index < 0 || index >= toolListCount) {
        return NULL;
    }

    return toolList[index];
}


ToolInfo* toolInfoFind(char* name)
{
    int i;

    if (appConfigGetInt("toolsenable", 1) == 0) {
        return NULL;
    }

    for (i = 0; i < toolListCount; i++) {
        if (strcmp(toolList[i]->description, name) == 0) {
            return toolList[i];
        }
    }
    return NULL;
}

const char* toolInfoGetName(ToolInfo* toolInfo)
{
    if (appConfigGetInt("toolsenable", 1) == 0) {
        return "";
    }

    if (toolInfo->callbacks.getName != NULL) {
        return toolInfo->callbacks.getName();
    }
    return toolInfo->description;
}

void toolInfoShowTool(ToolInfo* toolInfo)
{
    if (appConfigGetInt("toolsenable", 1) == 0) {
        return;
    }

    if (toolInfo->callbacks.show != NULL) {
        toolInfo->callbacks.show();
    }
}

void toolInfoSetLanguage(ToolInfo* toolInfo, int langId)
{
    if (appConfigGetInt("toolsenable", 1) == 0) {
        return;
    }

    if (toolInfo->callbacks.onEmulatorSetLg != NULL) {
        toolInfo->callbacks.onEmulatorSetLg(langId);
    }
}

void toolInfoAddArgument(ToolInfo* toolinfo, const char* type, void* arg)
{
    if (appConfigGetInt("toolsenable", 1) == 0) {
        return;
    }

    if (toolinfo->callbacks.onAddArgument != NULL) {
        toolinfo->callbacks.onAddArgument(type, arg);
    }
}

#endif
