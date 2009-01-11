/*****************************************************************************
** $Source: WiiTimer.c,v $
**
** $Revision: 1.2 $
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
#include "ArchTimer.h"
#include <stdlib.h>
#include <sys/time.h>
#include <lwp.h>
#include <system.h>
#include <stdio.h>

static int (*timer_callback)(void*);
static int timer_period = 0;
static int timer_count = 0;
static int timer_running = 0;
static lwpq_t timer_queue;
static lwp_t timer_thread;
static u32 timer_ticks = 0;

static void timerWrapper(syswd_t alarm)
{
    timer_ticks++;
    if( timer_period != 0 ) {
        if( timer_count-- == 0 ) {
            (void)LWP_ThreadSignal(timer_queue);
            timer_count = timer_period-1;
        }
    }
}

static int timerThreadEntry(void* data)
{
    syswd_t alarm;
    struct timespec abstime;
    struct timespec tperiod;

    (void)LWP_InitQueue(&timer_queue);
    (void)SYS_CreateAlarm(&alarm);

    tperiod.tv_sec = 0;
    tperiod.tv_nsec = 1 * 1000 * 1000;
    abstime.tv_sec = 0;
    abstime.tv_nsec = 0;
    (void)SYS_SetPeriodicAlarm(alarm, &abstime, &tperiod, timerWrapper);
    for(;;) {
      (void)LWP_ThreadSleep(timer_queue);
      if( timer_running == 0 )
        break;
      (void)timer_callback(NULL);
    }

    (void)SYS_RemoveAlarm(alarm);
    LWP_CloseQueue(timer_queue);

    return 0;
}

void* archCreateTimer(int period, int (*timerCallback)(void*))
{
    if( timer_period ) {
        fprintf(stderr, "ERROR: Only one timer supported!\n");
        exit(0);
    }
    timer_callback = timerCallback;
    timer_count = 0;
    timer_period = period;
    return (void*)1;
}

void archTimerDestroy(void* timer)
{
    timer_period = 0;
}

UInt32 archGetSystemUpTime(UInt32 frequency)
{
    return archGetHiresTimer() / (1000 / frequency);
}

UInt32 archGetHiresTimer()
{
    return timer_ticks;
}

void WiiTimerInit(void)
{
    if( !timer_running ) {
        timer_running = 1;
        (void)LWP_CreateThread(&timer_thread, (void *(*)(void *))timerThreadEntry, NULL, NULL, 0, 125);
    }
}

void WiiTimerDestroy(void)
{
    if( timer_running ) {
        timer_period = 0;
        timer_running = 0;
        (void)LWP_ThreadSignal(timer_queue);
        (void)LWP_JoinThread(timer_thread, NULL);
    }
}

