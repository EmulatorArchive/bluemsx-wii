#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gccore.h>
#include <context.h>
#include <ogc/semaphore.h>
#include <machine/processor.h>
#include "MsxTypes.h"

#if MALLOC_LOGGING

#define MALLOC_LOG_DEBUG 0

#define SIGNATURE_VALID  0x01234567
#define SIGNATURE_FREE   0xFEDCBA98

typedef struct _LOGMALLOC {
    UInt32 signature;
    UInt32 size;
    UInt32 line;
    const char *file;
    struct _LOGMALLOC *prev;
    struct _LOGMALLOC *next;
    UInt32 dummy[2]; /* for now struct must be 32 bytes to let memalign work */
} LOGMALLOC;

static LOGMALLOC *first_entry = NULL;
static LOGMALLOC *last_entry = NULL;
static LOGMALLOC *marked_entry = NULL;

static void *lowest_addr = (void*)0xffffffff;
static void *highest_addr = NULL;

static sem_t loggingSemaphore;
static int loggingInitialized = 0;
static int loggingEnabled = 0;

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef memalign

extern void *memalign(int align, int size);
extern void VIDEO_SetFramebuffer(void *);
static void *exception_xfb = (void*)0xC1710000;			//we use a static address above ArenaHi.
static char exception_str[1024];

static void ( * Reload ) () = ( void ( * ) () ) 0x80001800;

static void waitForReload() {

	u32 level;

	while ( 1 ) {

		PAD_ScanPads();

		int buttonsDown = PAD_ButtonsDown(0);

		if( (buttonsDown & PAD_TRIGGER_Z) || SYS_ResetButtonDown() ) {
			_CPU_ISR_Disable(level);
			Reload ();
		}

		if ( buttonsDown & PAD_BUTTON_A )
		{
			kprintf("Reset\n");
			SYS_ResetSystem(SYS_HOTRESET,0,FALSE);
		}

        VIDEO_SetFramebuffer(exception_xfb);
	}
}

static void allocError(char *error)
{
    VIDEO_SetFramebuffer(exception_xfb);
    console_init(exception_xfb,20,20,640,574,1280);

    kprintf("\n\n%s", error);

    waitForReload();
}

int printEntry(LOGMALLOC *log)
{
    if( log->signature == SIGNATURE_VALID ) {
        printf("%s line %d, size: %d\n", log->file, log->line, log->size);
        return 1;
    }else
    if( log->signature == SIGNATURE_FREE ) {
        printf("printEntry: allready freed entry\n");
        return 0;
    }else{
        printf("printEntry: invalid signature\n");
        return 0;
    }
}

void allocLogStart(void)
{
    if( !loggingInitialized ) {
        LWP_SemInit(&loggingSemaphore, 1, 0xffffffff);
        loggingInitialized = 1;
    }
    loggingEnabled = 1;
}

void allocLogStop(void)
{
    loggingEnabled = 0;
}

void allocLogSetMarker(void)
{
    marked_entry = last_entry;
}

void allocLogPrint(void)
{
    LOGMALLOC *p;
    if( loggingInitialized && loggingEnabled ) {
        LWP_SemWait(loggingSemaphore);

        printf("Malloc log:\n");
        if( marked_entry != NULL ) {
            p = marked_entry->next;
            marked_entry = NULL;
        }else{
            p = first_entry;
        }
        while( p != NULL ) {
            if( !printEntry(p) ) {
                break;
            }
            p = p->next;
        }
        printf("end log\n");

        LWP_SemPost(loggingSemaphore);
    }
}

static void checkValidBuffer(void *buf, const char *prefix, const char *file, int line)
{
    LOGMALLOC *log = (LOGMALLOC*)buf;
    if( log == NULL ) {
        sprintf(exception_str, "%s: %s line %d\nNULL pointer\n", prefix, file, line);
        allocError(exception_str);
    }
    if( (void*)log < lowest_addr ) {
        sprintf(exception_str, "%s: %s line %d\nPointer 0x%08x is before first entry 0x%08x\n",
               prefix, file, line, (UInt32)log, (UInt32)lowest_addr);
        allocError(exception_str);
    }
    if( (void*)log > highest_addr ) {
        sprintf(exception_str, "%s: %s line %d\nPointer 0x%08x is behind last entry 0x%08x\n",
               prefix, file, line, (UInt32)log, (UInt32)highest_addr);
        allocError(exception_str);
    }
    if( log->signature == SIGNATURE_FREE ) {
        sprintf(exception_str, "%s: %s line %d\nPointer 0x%08x is allready freed\n",
               prefix, file, line, (UInt32)log);
        allocError(exception_str);
    }
    if( log->signature != SIGNATURE_VALID ) {
        sprintf(exception_str, "%s: %s line %d\nPointer 0x%08x has bad signature\n",
               prefix, file, line, (UInt32)log);
        allocError(exception_str);
    }
}

static void addLogEntry(LOGMALLOC *log, int size, const char *file, int line)
{
    log->size = size;
    log->line = line;
    log->file = file;
    log->prev = NULL;
    log->next = NULL;
    if( last_entry != NULL ) {
        log->prev = last_entry;
        last_entry->next = log;
    }
    if( first_entry == NULL ) {
        first_entry = log;
    }
    last_entry = log;
    if( (void*)log < lowest_addr ) {
        lowest_addr = log;
    }
    if( (void*)log > highest_addr ) {
        highest_addr = log;
    }
    log->signature = SIGNATURE_VALID;
}

static void removeLogEntry(LOGMALLOC *log)
{
    if( log->prev != NULL ) {
        log->prev->next = log->next;
    }
    if( log->next != NULL ) {
        log->next->prev = log->prev;
    }
    if( first_entry == log ) {
        first_entry = log->next;
    }
    if( last_entry == log ) {
        last_entry = log->prev;
    }
    if( marked_entry == log ) {
        marked_entry = log->prev;
    }
}

static void updateLogEntry(LOGMALLOC *newlog, LOGMALLOC *oldlog)
{
    if( newlog->prev != NULL ) {
        newlog->prev->next = newlog;
    }
    if( newlog->next != NULL ) {
        newlog->next->prev = newlog;
    }
    if( first_entry == oldlog ) {
        first_entry = newlog;
    }
    if( last_entry == oldlog ) {
        last_entry = newlog;
    }
    if( marked_entry == oldlog ) {
        marked_entry = newlog;
    }
    if( (void*)newlog < lowest_addr ) {
        lowest_addr = newlog;
    }
    if( (void*)newlog > highest_addr ) {
        highest_addr = newlog;
    }
}

void *my_malloc(int size, const char *file, int line)
{
    if( loggingEnabled ) {
        void *buf;
#if MALLOC_LOG_DEBUG
        printf("malloc %s %d 0x%X\n", file, line, size);
#endif
        LWP_SemWait(loggingSemaphore);
        buf = malloc(size + sizeof(LOGMALLOC));
        if( buf == NULL ) {
            sprintf(exception_str, "malloc: %s line %d failed, memory full?\n",
                   file, line);
            allocError(exception_str);
        }
        addLogEntry((LOGMALLOC*)buf, size, file, line);
        LWP_SemPost(loggingSemaphore);
#if MALLOC_LOG_DEBUG
        printf("done\n");
#endif
        return (char*)buf + sizeof(LOGMALLOC);
    }else{
        return malloc(size);
    }
}

void *my_calloc(int num, int size, const char *file, int line)
{
    if( loggingEnabled ) {
        void *buf;
#if MALLOC_LOG_DEBUG
        printf("calloc %s %d %d 0x%X\n", file, line, num, size);
#endif
        LWP_SemWait(loggingSemaphore);
        size = num * size;
        buf = calloc(1, size + sizeof(LOGMALLOC));
        if( buf == NULL ) {
            sprintf(exception_str, "calloc: %s line %d failed, memory full?\n",
                   file, line);
            allocError(exception_str);
        }
        addLogEntry((LOGMALLOC*)buf, size, file, line);
        LWP_SemPost(loggingSemaphore);
#if MALLOC_LOG_DEBUG
        printf("done\n");
#endif
        return (char*)buf + sizeof(LOGMALLOC);
    }else{
        return calloc(num, size);
    }
}

void *my_memalign(int align, int size, const char *file, int line)
{
    if( loggingEnabled ) {
        void *buf;
#if MALLOC_LOG_DEBUG
        printf("memalign %s %d %d 0x%X\n", file, line, align, size);
#endif
        LWP_SemWait(loggingSemaphore);
        buf = memalign(align, size + sizeof(LOGMALLOC));
        if( buf == NULL ) {
            sprintf(exception_str, "memalign: %s line %d failed, memory full?\n",
                   file, line);
            allocError(exception_str);
        }
        addLogEntry((LOGMALLOC*)buf, size, file, line);
        LWP_SemPost(loggingSemaphore);
#if MALLOC_LOG_DEBUG
        printf("done\n");
#endif
        return (char*)buf + sizeof(LOGMALLOC);
    }else{
        return memalign(align, size);
    }
}

void *my_realloc(void *buf, int size, const char *file, int line)
{
    if( loggingEnabled ) {
        if( buf != NULL ) {
            void *oldbuf;
#if MALLOC_LOG_DEBUG
            printf("realloc %s %d 0x%X\n", file, line, size);
#endif
            LWP_SemWait(loggingSemaphore);
            buf = oldbuf = (char*)buf - sizeof(LOGMALLOC);
            checkValidBuffer(buf, "realloc", file, line);
            buf = realloc(buf, size + sizeof(LOGMALLOC));
            if( buf == NULL ) {
                sprintf(exception_str, "realloc: %s line %d failed, memory full?\n",
                       file, line);
                allocError(exception_str);
            }
            updateLogEntry((LOGMALLOC*)buf, (LOGMALLOC*)oldbuf);
            ((LOGMALLOC*)buf)->size = size;
            LWP_SemPost(loggingSemaphore);
#if MALLOC_LOG_DEBUG
            printf("done\n");
#endif
        }else{
            return my_malloc(size, file, line);
        }
        return (char*)buf + sizeof(LOGMALLOC);
    }else{
        return realloc(buf, size);
    }
}

char *my_strdup(const char *str, const char *file, int line)
{
    char *p;
    if( str == NULL ) {
        sprintf(exception_str, "strdup: %s line %d on NULL string\n",
                file, line);
        allocError(exception_str);
    }
    p = (char*)my_malloc(strlen(str)+1, file, line);
    strcpy(p, str);
    return p;
}

void my_free(void *buf, const char *file, int line)
{
    if( loggingEnabled ) {
#if MALLOC_LOG_DEBUG
        printf("free %s %d\n", file, line);
#endif
        LWP_SemWait(loggingSemaphore);
        if( buf != NULL ) {
            buf = (char*)buf - sizeof(LOGMALLOC);
        }
        checkValidBuffer(buf, "free", file, line);
        removeLogEntry((LOGMALLOC*)buf);
        ((LOGMALLOC*)buf)->signature = SIGNATURE_FREE;
        free(buf);
        LWP_SemPost(loggingSemaphore);
#if MALLOC_LOG_DEBUG
        printf("done\n");
#endif
    }else{
        free(buf);
    }
}

#endif

