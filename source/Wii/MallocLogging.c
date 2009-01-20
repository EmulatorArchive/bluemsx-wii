#include <stdlib.h>
#include <stdio.h>

#if 0
// 'free' crash interception. To build this, libogc needs to be rebuild with
// the function 'c_default_exceptionhandler' renamed to '_c_default_exceptionhandler'
#include <gccore.h>
#include <context.h>
#include <console.h>
extern void VIDEO_SetFramebuffer(void *);
static const char *free_file;
static int free_line;
static void *exception_xfb = (void*)0xC1710000;			//we use a static address above ArenaHi.

extern void __console_init(void *framebuffer,int xstart,int ystart,int xres,int yres,int stride);
extern void _c_default_exceptionhandler(frame_context *pCtx);

static int reent = 0;

void c_default_exceptionhandler(frame_context *pCtx)
{
    if( free_file ) {
        VIDEO_SetFramebuffer(exception_xfb);
        __console_init(exception_xfb,20,20,640,574,1280);
        
        kprintf("free crash: %d %s line %d\n", reent, free_file, free_line);
        
        for(;;) VIDEO_WaitVSync();
    }else{
        _c_default_exceptionhandler(pCtx);
    }
}

void my_free(void *a, const char *file, int line)
{
    free_file = file;
    free_line = line;
    reent++;
    free(a);
    reent--;
    free_file = NULL;
    free_line = 0;
}
#endif

#if MALLOC_LOGGING

#undef free
#undef malloc
#undef calloc

void my_free(void *a, const char *file, int line)
{
    printf("free %s %d\n", file, line);
    free(a);
}

void *my_malloc(int a, const char *file, int line)
{
    printf("malloc %s %d 0x%X\n", file, line, a);
    return malloc(a);
}

void *my_calloc(int a, int b, const char *file, int line)
{
    printf("calloc %s %d 0x%X\n", file, line, a*b);
    return calloc(a, b);
}

#endif

