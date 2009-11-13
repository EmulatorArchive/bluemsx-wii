/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Win32/BlueMSXToolInterface.h,v $
**
** $Revision: 1.21 $
**
** $Date: 2009-07-01 05:00:23 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
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
#ifndef BLUEMSX_TOOL_INTERFACE_H
#define BLUEMSX_TOOL_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BLUEMSX_TYPES
#define BLUEMSX_TYPES

#ifdef __GNUC__
#define __int64 long long
#endif

typedef unsigned char    UInt8;
typedef unsigned short   UInt16;
typedef unsigned long    UInt32;
typedef unsigned __int64 UInt64;
typedef signed   char    Int8;
typedef signed   short   Int16;
typedef signed   long    Int32;

// Define color stuff
#if PIXEL_WIDTH==32
typedef UInt32 Pixel;
#elif PIXEL_WIDTH==8
typedef UInt8 Pixel;
#else
typedef UInt16 Pixel;
#endif


#endif

typedef enum { EMULATOR_STOPPED, EMULATOR_PAUSED, EMULATOR_RUNNING } EmulatorState;

typedef enum {
    LID_ENGLISH    = 0,
    LID_SWEDISH    = 1,
    LID_JAPANESE   = 2,
    LID_PORTUGUESE = 3,
    LID_FRENCH     = 4,
    LID_DUTCH      = 5,
    LID_SPANISH    = 6,
    LID_ITALIAN    = 7,
    LID_FINNISH    = 8,
    LID_KOREAN     = 9,
    LID_GERMAN     = 10,
    LID_POLISH     = 11
} LanguageId;

typedef struct Snapshot Snapshot;
typedef struct Device   Device;

typedef enum {
    DEVTYPE_UNKNOWN,
    DEVTYPE_CPU,
    DEVTYPE_CART,
    DEVTYPE_BIOS,
    DEVTYPE_RAM,
    DEVTYPE_AUDIO,
    DEVTYPE_VIDEO,
    DEVTYPE_IOPORT
} DeviceType;

struct Device {
    char name[64];
    DeviceType type;
    int  deviceId;
};


typedef struct {
    int    deviceId;
    char   name[32];
    int    writeProtected;
    UInt32 startAddress;
    UInt32 size;
    UInt8  memory[1];
} MemoryBlock;

typedef struct {
    int    deviceId;
    char   name[32];
    UInt32 count;
    struct Register {
        char  name[7];
        UInt8 width;
        UInt32 value;
    } reg[1];
} RegisterBank;

typedef struct {
    int    deviceId;
    char   name[32];
    UInt32 size;
    UInt32 callstack[1];
} Callstack;

typedef enum {
    IO_PORT_NONE      = 0,
    IO_PORT_READ      = 1,
    IO_PORT_WRITE     = 2,
    IO_PORT_READWRITE = 3
} IoPortDirection;

typedef struct {
    int    deviceId;
    char   name[32];
    UInt32 count;
    struct IoPort {
        UInt16 port;
        UInt8  direction;
        UInt8  value;
    } port[1];
} IoPorts;


/////////////////////////////////
/// Tool interface

typedef Snapshot*     (*ToolSnapshotCreate)();
typedef void          (*ToolSnapshotDestroy)(Snapshot* Snapshot);
typedef EmulatorState (*ToolGetEmulatorState)();
typedef int           (*ToolSnapshotGetDeviceCount)(Snapshot* Snapshot);
typedef Device*       (*ToolSnapshotGetDevice)(Snapshot* Snapshot, int deviceIndex);

typedef int           (*ToolDeviceGetMemoryBlockCount)(Device* device);
typedef MemoryBlock*  (*ToolDeviceGetMemoryBlock)(Device* device, int memBlockIndex);
typedef int           (*ToolDeviceWriteMemory)(MemoryBlock* memoryBlock, void* data, int startAddr, int size);
typedef int           (*ToolDeviceGetRegisterBankCount)(Device* device);
typedef RegisterBank* (*ToolDeviceGetRegisterBank)(Device* device, int regBankIndex);
typedef int           (*ToolDeviceWriteRegister)(RegisterBank* regBank, int regIndex, UInt32 value);
typedef int           (*ToolDeviceGetCallstackCount)(Device* device);
typedef Callstack*    (*ToolDeviceGetCallstack)(Device* device, int callstackIndex);
typedef int           (*ToolDeviceGetIoPortsCount)(Device* device);
typedef IoPorts*      (*ToolDeviceGetIoPorts)(Device* device, int ioPortIndex);
typedef int           (*ToolDeviceWriteIoPort)(IoPorts* ioPorts, int portIndex, UInt32 value);
typedef void          (*ToolAction)();
typedef void          (*ToolBreakpoint)(UInt16);
typedef char*         (*ToolPath)();
typedef void          (*ToolEmulatorVersion)(int* major, int* minor, int* buildNumber);
typedef void          (*ToolEnableVramAccessCheck)(int enable);
typedef struct {
    ToolSnapshotCreate              create;
    ToolSnapshotDestroy             destroy;
    ToolGetEmulatorState            getState;
    ToolSnapshotGetDeviceCount      getDeviceCount;
    ToolSnapshotGetDevice           getDevice;

    ToolDeviceGetMemoryBlockCount   getMemoryBlockCount;
    ToolDeviceGetMemoryBlock        getMemoryBlock;
    ToolDeviceWriteMemory           writeMemoryBlockSement;
    ToolDeviceGetRegisterBankCount  getRegisterBankCount;
    ToolDeviceGetRegisterBank       getRegisterBank;
    ToolDeviceWriteRegister         writeRegisterBankRegister;
    ToolDeviceGetCallstackCount     getCallstackCount;
    ToolDeviceGetCallstack          getCallstack;
    ToolDeviceGetIoPortsCount       getIoPortsCount;
    ToolDeviceGetIoPorts            getIoPorts;
    ToolDeviceWriteIoPort           writeIoPortsPort;

    ToolAction                      run;
    ToolAction                      stop;
    ToolAction                      pause;
    ToolAction                      step;

    ToolBreakpoint                  setBreakpoint;
    ToolBreakpoint                  clearBreakpoint;

    ToolPath                        getToolDirectory;
    ToolEmulatorVersion             getEmulatorVersion;

    ToolEnableVramAccessCheck       enableVramAccessCheck;
} Interface;

typedef int  (*CreateFn)(Interface*, char*, int);
typedef void (*NotifyFn)();
typedef void (*TraceFn)(const char*);
typedef void (*SetBpFn)(UInt16, UInt16, UInt16);
typedef void (*SetLgFn)(int);
typedef void (*ArgFn)(const char*, void*);
typedef const char* (*GetNameFn)();

#ifdef __cplusplus
}
#endif

#endif
