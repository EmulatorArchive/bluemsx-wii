/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Wii/WiiSound.c,v $
**
** $Revision: 1.0 $
**
** $Date: 2008/12/07 00:00:00 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2008 Tim Brugman
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
#include "ArchSound.h"
#include <stdlib.h>
#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <ogc/audio.h>
#include <ogc/cache.h>
#include <ogc/lwp.h>
#include <ogc/semaphore.h>

#define NUM_BUFFERS 4
#define BUFFER_SIZE 2*1024
static char buffer[NUM_BUFFERS][BUFFER_SIZE] __attribute__((aligned(32)));
static int which_buffer = 0;
static unsigned int buffer_offset = 0;
#define NEXT(x) (x=(x+1)%NUM_BUFFERS)

static lwp_t audio_thread;
static sem_t buffer_full;
static sem_t buffer_empty;
static sem_t audio_free;
static int   thread_running;
#define AUDIO_STACK_SIZE 1024 // MEM: I could get away with a smaller stack
static char  audio_stack[AUDIO_STACK_SIZE];
#define AUDIO_PRIORITY 100
static int   thread_buffer = 0;
static int   audio_paused = 0;
static int   audio_initialized = 0;
static Mixer *audio_mixer;

static void done_playing(void)
{
    // We're done playing, so we're releasing a buffer and the audio
    LWP_SemPost(buffer_empty);
    LWP_SemPost(audio_free);
}

static void inline play_buffer(void)
{
    // This thread will keep giving buffers to the audio as they come
    while(thread_running){

        // Wait for a buffer to be processed
        LWP_SemWait(buffer_full);

        // Make sure the buffer is in RAM, not the cache
        DCFlushRange(buffer[thread_buffer], BUFFER_SIZE);

        // Wait for the audio interface to be free before playing
        LWP_SemWait(audio_free);

        // Break-out when quitting
        if( !thread_running ) {
            break;
        }

        // Actually send the buffer out to be played
        AUDIO_InitDMA((unsigned int)&buffer[thread_buffer], BUFFER_SIZE);
        AUDIO_StartDMA();

        // Move the index to the next buffer
        NEXT(thread_buffer);
    }
}

static void inline copy_to_buffer(char* buffer, char* stream, unsigned int length)
{
        memcpy(buffer, stream, length);
}

static Int32 soundWrite(void* dummy, Int16 *stream, UInt32 length)
{
    // This shouldn't lose any data and works for any size
    unsigned int lengthi;
    unsigned int lengthLeft = length * 2; /* length is in samples, convert to bytes */
    unsigned int stream_offset = 0;
    if( !audio_initialized ) {
        return 0;
    }
    while(1){
        lengthi = (buffer_offset + lengthLeft < BUFFER_SIZE) ?
                   lengthLeft : (BUFFER_SIZE - buffer_offset);

        // Wait for a buffer we can copy into
        LWP_SemWait(buffer_empty);

        copy_to_buffer(buffer[which_buffer] + buffer_offset,
                       (char *)stream + stream_offset, lengthi);

        if(buffer_offset + lengthLeft < BUFFER_SIZE){
                buffer_offset += lengthi;

                // This is a little weird, but we didn't fill this buffer.
                //   So it is still considered 'empty', but since we 'decremented'
                //   buffer_empty coming in here, we want to set it back how
                //   it was, so we don't cause a deadlock
                LWP_SemPost(buffer_empty);
                break;
        }

        lengthLeft    -= lengthi;
        stream_offset += lengthi;

        // Let the audio thread know that we've filled a new buffer
        LWP_SemPost(buffer_full);

        NEXT(which_buffer);
        buffer_offset = 0;
    }
    return 0;
}

void archSoundCreate(Mixer* mixer, UInt32 sampleRate, UInt32 bufferSize, Int16 channels)
{
    if( audio_initialized ) {
        archSoundDestroy();
    }
    // Check parameters
    if( sampleRate != 48000 ) {
        fprintf(stderr, "Samplerate of %d not supported\n", sampleRate);
        exit(1);
    }
    if( channels != 2 ) {
        fprintf(stderr, "Only two channel stereo suppoorted\n");
        exit(1);
    }
    // Init audio
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
    // Create our semaphores and start/resume the audio thread; reset the buffer index
    LWP_SemInit(&buffer_full, 0, NUM_BUFFERS);
    LWP_SemInit(&buffer_empty, NUM_BUFFERS, NUM_BUFFERS);
    LWP_SemInit(&audio_free, 0, 1);
    thread_running = 1;
    LWP_CreateThread(&audio_thread, (void*)play_buffer, NULL, audio_stack, AUDIO_STACK_SIZE, AUDIO_PRIORITY);
    AUDIO_RegisterDMACallback(done_playing);
    thread_buffer = which_buffer = 0;
    audio_paused = 1;
    // Initialize mixer
    audio_mixer = mixer;
    mixerSetStereo(mixer, channels == 2);
    mixerSetWriteCallback(mixer, soundWrite, NULL, BUFFER_SIZE/2);
    audio_initialized = 1;
}

void archSoundDestroy(void)
{
    if( audio_initialized ) {
        // De-register to mixer
        mixerSetWriteCallback(audio_mixer, NULL, NULL, 0);
        // Stop DMA
        AUDIO_StopDMA();
        // Stop thread
        thread_running = 0;
        LWP_SemPost(buffer_full);
        LWP_SemPost(audio_free);
        LWP_JoinThread(audio_thread, NULL);
        // Destroy semaphores and suspend the thread so audio can't play
        LWP_SemDestroy(buffer_full);
        LWP_SemDestroy(buffer_empty);
        LWP_SemDestroy(audio_free);
        audio_paused = 0;
        audio_initialized = 0;
    }
}

void archSoundSuspend(void)
{
    // We just grab the audio_free 'lock' and don't let go
    //   when we have this lock, audio_thread must be waiting
    LWP_SemWait(audio_free);
    audio_paused = 1;

    AUDIO_StopDMA();
}

void archSoundResume(void)
{
    if(audio_paused){
        // When we're want the audio to resume, release the 'lock'
        LWP_SemPost(audio_free);
        audio_paused = 0;
    }
}

