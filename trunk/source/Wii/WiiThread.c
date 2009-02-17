/*****************************************************************************
** $Source: WiiThread.c,v $
**
** $Revision: 1.3 $
**
** $Date: 2006/06/24 02:27:08 $
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
#include "ArchThread.h"
#include <lwp.h>
#include <stdlib.h>
#include <system.h>
#include <gccore.h>

#include <stdio.h>

typedef struct {
  lwp_t thread;
  void *stack;
  u32 ssize;
} thread_t;

static int threadEntry(void* data)
{
    void (*entryPoint)() = data;
    entryPoint();
    return 0;
}

void* archThreadCreateEx(void (*entryPoint)(), int priority, int stacksize)
{
    if( priority == THREAD_PRIO_HIGH ) {
        priority = 110;
    }else{
        priority = 90;
    }

    thread_t* t = (thread_t*)malloc(sizeof(thread_t));
    if( t == NULL ) {
        fprintf(stderr, "THREAD: Error 1\n");
        return NULL;
    }
    if( stacksize ) {
        t->stack = malloc(stacksize);
        t->ssize = (u32)stacksize;
    }else{
        t->stack = NULL;
        t->ssize = 0;
    }
    if( LWP_CreateThread(&t->thread, (void *(*)(void *))threadEntry, entryPoint, t->stack, t->ssize, priority) != 0 ) {
        free(t);
        fprintf(stderr, "THREAD: Error 2\n");
        return NULL;
    }
    return t;
}

void* archThreadCreate(void (*entryPoint)(), int priority)
{
    return archThreadCreateEx(entryPoint, priority, 128*1024);
}

void archThreadJoin(void* thread, int timeout)
{
    thread_t* t = (thread_t*)thread;

    LWP_JoinThread(t->thread, NULL);
}

void  archThreadDestroy(void* thread)
{
    thread_t* t = (thread_t*)thread;

    if( t->ssize != 0 && t->stack ) {
        free(t->stack);
        t->stack = NULL;
    }
    free(thread);
}

void archThreadSleep(int milliseconds)
{
    do {
        VIDEO_WaitVSync();
        milliseconds -= 20;
    }while(milliseconds > 0);
}

