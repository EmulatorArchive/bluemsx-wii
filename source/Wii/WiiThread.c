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

static int threadEntry(void* data)
{
    void (*entryPoint)() = data;
    entryPoint();
    return 0;
}

void* archThreadCreate(void (*entryPoint)(), int priority)
{
    if( priority == THREAD_PRIO_HIGH ) {
        priority = 110;
    }else{
        priority = 90;
    }

    lwp_t* t = (lwp_t*)malloc(sizeof(lwp_t));
    if( t == NULL ) {
        fprintf(stderr, "THREAD: Error 1\n");
        return NULL;
    }
    if( LWP_CreateThread(t, (void *(*)(void *))threadEntry, entryPoint, NULL, 0, priority) != 0 ) {
        free(t);
        fprintf(stderr, "THREAD: Error 2\n");
        return NULL;
    }
    return t;
}

void archThreadJoin(void* thread, int timeout)
{
    lwp_t* t = (lwp_t*)thread;

    while( LWP_JoinThread(*t, NULL) != 0 );
}

void  archThreadDestroy(void* thread)
{
}

void archThreadSleep(int milliseconds)
{
    do {
        VIDEO_WaitVSync();
        milliseconds -= 20;
    }while(milliseconds > 0);
}

