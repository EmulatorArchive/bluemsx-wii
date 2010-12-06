
#include <string>
#include <list>
#include <sstream>
#include <iomanip>

#include "../../Arch/ArchThread.h"

#include "Language.h"
#include "ToolInterface.h"
#include "Trainer.h"

// gui stuff
#include "../../GuiBase/GuiContainer.h"
#include "../../GuiBase/GuiEffectFade.h"
#include "../../GuiElements/GuiElmCheckList.h"
#include "../../GuiDialogs/GuiDlgMessageBox.h"

enum CompareType
{
    CMP_EQUAL,
    CMP_NOTEQUAL,
    CMP_LESSTHAN,
    CMP_LESSOREQUAL,
    CMP_GRATERTHAN,
    CMP_GREATEROREQUAL,
};

enum DisplayType
{
    DPY_DECIMAL,
    DPY_HEXADECIMAL,
};

enum DataSize
{
    DATASIZE_8BIT,
    DATASIZE_16BIT,
};

enum CompareValue
{
    CMP_OLDVALUE,
    CMP_CHANGE,
    CMP_SPECIFIC,
};

using namespace std;

static LanguageId langId = LID_ENGLISH;

static CompareType  compareType    = CMP_EQUAL;
static DisplayType  displayType    = DPY_DECIMAL;
static DataSize     dataSize       = DATASIZE_8BIT;
static CompareValue compareValue   = CMP_SPECIFIC;
static Int32        cmpChangeVal   = 0;
static Int32        cmpSpecificVal = 0;

static bool canSearch   = false;
static bool canUndo     = false;
static bool canAddCheat = false;
static bool hasSnapshot = false;
static bool canCheat    = false;
static bool canRemoveCheat = false;
static bool canRemoveAllCheat = false;

static Int32 DataMask[]  = { 0xff, 0xffff };
static char* DpyFormat[] = { "%d", "%X" };
static char* DpySizeFormat[2][2]  = { { "%d", "%.2X" }, { "%d", "%.4X" } };

static GuiContainer *container = NULL;
static void* cheatsThread = NULL;

#define CHEAT_TIMER_ID 29

struct CheatInfo {
    char description[128];
    UInt32      address;
    Int32       value;
    DataSize    dataSize;
    DisplayType displayType;
    bool        enabled;

    CheatInfo(const char* d, UInt32 a, Int32 v, DataSize ds, DisplayType dt, bool e) : address(a), value(v), dataSize(ds), displayType(dt), enabled(e) {
        strncpy(description, d, sizeof(description));
        description[sizeof(description) - 1] = 0;
    }
};

static void updateCheatList();

static UInt16 getData(DataSize dataSize, UInt8* mem, Int32 offset)
{
    if (dataSize == DATASIZE_16BIT) {
        return mem[offset] + 256 * mem[(offset + 1) & 0xffff];
    }
    return mem[offset];
}

struct SnapshotData
{
    UInt8 dataNew[0x10000];
    UInt8 dataOld[0x10000];
    UInt8 mask[0x10000];
    UInt8 maskRef[0x10000];
};

SnapshotData snapshotData;
static Snapshot* currentSnapshot = NULL;
MemoryBlock* memoryBlock = NULL;

void updateSnapshot()
{
    memoryBlock = NULL;

    bool isRunning = GetEmulatorState() == EMULATOR_RUNNING;

    if (isRunning) {
        EmulatorPause();
    }

    Snapshot* snapshot = SnapshotCreate();
    if (snapshot != NULL) {
        int deviceCount = SnapshotGetDeviceCount(snapshot);

        for (int i = 0; i < deviceCount; i++) {
            Device* device = SnapshotGetDevice(snapshot, i);
            int memCount = DeviceGetMemoryBlockCount(device);

            if (device->type == DEVTYPE_CPU && memCount > 0) {
                memoryBlock = DeviceGetMemoryBlock(device, 0);
            }
        }
    }

    if (currentSnapshot != NULL) {
        SnapshotDestroy(currentSnapshot);
    }

    currentSnapshot = snapshot;

    if (isRunning) {
        EmulatorRun();
    }
}

void createSnapshot(bool reset = false)
{
    if ((!hasSnapshot && !reset) || GetEmulatorState() == EMULATOR_RUNNING) {
        return;
    }

    Snapshot* snapshot = SnapshotCreate();
    if (snapshot != NULL) {
        int deviceCount = SnapshotGetDeviceCount(snapshot);

        for (int i = 0; i < deviceCount; i++) {
            Device* device = SnapshotGetDevice(snapshot, i);
            int memCount = DeviceGetMemoryBlockCount(device);

            if (device->type == DEVTYPE_CPU && memCount > 0) {
                MemoryBlock* mem = DeviceGetMemoryBlock(device, 0);
                if (mem->size == 0x10000) {
                    if (reset) {
                        memcpy(snapshotData.dataOld, mem->memory, 0x10000);
                        memset(snapshotData.mask,    1,           0x10000);
                        memset(snapshotData.maskRef, 1,           0x10000);
                    }
                    else {
                        memcpy(snapshotData.dataOld, snapshotData.dataNew, 0x10000);
                        memcpy(snapshotData.maskRef, snapshotData.mask,    0x10000);
                    }
                    memcpy(snapshotData.dataNew, mem->memory, 0x10000);
                    hasSnapshot = true;
                }
            }
        }
    }

    SnapshotDestroy(snapshot);
}

////////////////////////////////////////////////////////////////////////

typedef std::list<CheatInfo> CheatList;

CheatList cheatList;

void clearAllCheats()
{
    while(!cheatList.empty()) {
        cheatList.pop_front();
    }

    canRemoveAllCheat = false;
}

void addCheat(char* description, UInt32 address, Int32 value, DataSize dataSize, DisplayType displayType, bool enabled)
{
    CheatInfo ci(description, address, value, dataSize, displayType, enabled);
    CheatList::iterator i = cheatList.begin();

    for (; i != cheatList.end(); ++i) {
        if (i->address > address) break;
    }
    cheatList.insert(i, ci);

    canRemoveAllCheat = !cheatList.empty();
}

void addCheat(CheatInfo* ci)
{
    addCheat(ci->description, ci->address, ci->value, ci->dataSize, ci->displayType, ci->enabled);
}

void removeCheat(int idx)
{
    for (CheatList::iterator i = cheatList.begin(); i != cheatList.end(); ++i) {
        if (idx-- == 0) {
            cheatList.erase(i);
            break;
        }
    }

    canRemoveAllCheat = !cheatList.empty();
}

void updateEnableCheat(int idx, bool enable)
{
    for (CheatList::iterator i = cheatList.begin(); i != cheatList.end(); ++i) {
        if (idx-- == 0) {
            (*i).enabled = enable;
            break;
        }
    }
}

void toggleEnableCheat(int idx)
{
    for (CheatList::iterator i = cheatList.begin(); i != cheatList.end(); ++i) {
        if (idx-- == 0) {
            (*i).enabled = !(*i).enabled;
            break;
        }
    }
}

static void executeCheats()
{
    if (!canCheat) {
        return;
    }

    if (memoryBlock == NULL) {
        updateSnapshot();
    }

    if (memoryBlock == NULL) {
        return;
    }

    for (CheatList::iterator i = cheatList.begin(); i != cheatList.end(); ++i) {
        CheatInfo& ci = (*i);

        if (ci.enabled) {
            int size = 1;
            UInt8 data[2];
            data[0] = (UInt8)(ci.value & 0xff);
            if (ci.dataSize == DATASIZE_16BIT) {
                size = 2;
                data[1] = data[0];
                data[0] = (UInt8)(ci.value >> 8);
            }
            DeviceWriteMemoryBlockMemory(memoryBlock, data, ci.address, size);
        }
    }
}

static void executeCheatsThread(void)
{
    while(1) {
        executeCheats();
        archThreadSleep(100);
    }
}

static CheatInfo* getCheat(int index)
{
    int idx = 0;
    for (CheatList::iterator i = cheatList.begin(); i != cheatList.end(); ++i) {
        if (idx++ == index) {
            return &(*i);
        }
    }
    return NULL;
}

static bool loadCheatFile(const char* filename)
{
    clearAllCheats();

    if (filename == NULL) {
        return false;
    }

    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        GuiDlgMessageBox *msgbox = new GuiDlgMessageBox(container, "invalidchaet");
        msgbox->Create(MSGT_TEXT, NULL, 128, "Invalid cheat file!");
        container->AddTop(msgbox, new GuiEffectFade(10));
        archThreadSleep(2000);
        container->RemoveAndDelete(msgbox, new GuiEffectFade(10));
        return false;
    }

    for (int i = 0; i < 1000; i++) {
        char buffer[1024];
        if (NULL == fgets(buffer, sizeof(buffer), f)) {
            break;
        }
        if (buffer[0] == '!') {
            continue;
        }
        int memType;
        int address;
        int value;
        int dummy;
        int dataSize = DATASIZE_8BIT;
        int displayType = DPY_HEXADECIMAL;

        int rv = sscanf(buffer, "%X:%X:%d:%d\n", &address, &value, &dataSize, &displayType);
        if (rv != 4) {
            rv = sscanf(buffer, "%d,%d,%d,%d", &memType, &address, &value, &dummy);
        }
        if (rv != 4) continue;
        char* desc = buffer + strlen(buffer) - 1;
        if (desc[0] == '\r' || desc[0] == '\n') desc[0] = 0;
        if (desc[-1] == '\r' || desc[-1] == '\n') desc[-1] = 0;
        desc = buffer;
        int commaCnt = 0;
        while (*desc && commaCnt < 4) {
            if (*desc == ',' || *desc == ':') commaCnt++;
            desc++;
        }

        addCheat(desc, address, value, (DataSize)dataSize, (DisplayType)displayType, false);
    }

    fclose(f);

    if( cheatsThread == NULL ) {
        cheatsThread = archThreadCreate(executeCheatsThread, THREAD_PRIO_NORMAL);
    }

    return true;
}

void OnCreateTool() {
}

void OnDestroyTool() {
    if( currentSnapshot ) {
        SnapshotDestroy(currentSnapshot);
    }
    if( cheatsThread ) {
        archThreadDestroy(cheatsThread);
    }
}

void OnShowTool()
{
    int num_items = cheatList.size();
    canAddCheat = false;

    Language::SetLanguage(langId);

    const char **title_list = (const char**)malloc(num_items * sizeof(const char*));
    bool *enabled_list = (bool *)malloc(num_items * sizeof(bool));
    int idx = 0;
    for (CheatList::iterator i = cheatList.begin(); i != cheatList.end(); ++i) {
        title_list[idx] = (*i).description;
        enabled_list[idx++] = (*i).enabled;
    }

    GuiElmCheckList *check_list = new GuiElmCheckList(container, "checklist", 9);
    bool leave = false;
    do {
        int selection;
        SELRET action = check_list->DoModal(&selection, title_list, enabled_list, num_items, 484+12);
        switch( action ) {
            case SELRET_SELECTED:
                toggleEnableCheat(selection);
                break;
            case SELRET_KEY_B:
            case SELRET_KEY_HOME:
                leave = true;
                break;
            default:
                break;
        }
    }while( !leave );
    delete check_list;

    free(title_list);
    free(enabled_list);
    delete check_list;
}

void OnEmulatorStart() {
    canCheat = true;
}

void OnEmulatorStop() {
    memoryBlock = NULL;
    canCheat = false;
}

void OnEmulatorPause() {
    createSnapshot();
    canCheat = false;
}

void OnEmulatorResume() {
    canCheat = true;
}

void OnEmulatorReset() {
}

void OnEmulatorTrace(const char* message)
{
}

void OnEmulatorSetBreakpoint(UInt16 address) {
}

void OnEmulatorSetBreakpoint(UInt16 slot, UInt16 address) {
}

void OnEmulatorSetBreakpoint(UInt16 slot, UInt16 page, UInt16 address) {
}

const char* OnGetName() {
    return Language::trainerName;
}

void OnSetLanguage(LanguageId languageId)
{
    langId = languageId;
    Language::SetLanguage(langId);
}

void OnAddArgument(const char *str, void* arg)
{
    if (strcmp(str, "CheatFile") == 0) {
        loadCheatFile((const char *)arg);
    } else if (strcmp(str, "GuiContainer") == 0) {
        container = (GuiContainer*)arg;
    } else {
        printf("Unknown argument: %s\n", str);
    }
}

