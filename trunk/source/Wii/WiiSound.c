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

#define AUDIO_DEBUG 0

#define BUFFER_SIZE 16*1024
static char sample_buffer[BUFFER_SIZE] __attribute__((aligned(32)));
static unsigned int buffer_offset = 0;

static sem_t audio_free;
static int   audio_paused = 0;
static int   audio_initialized = 0;
static int   buffer_tuning = 0;
static Mixer *audio_mixer;

static void done_playing(void)
{
    if( !audio_paused ) {
        // (re)start DMA
        LWP_SemWait(audio_free);
        buffer_tuning = buffer_offset - BUFFER_SIZE/2;
        AUDIO_InitDMA((unsigned int)&sample_buffer, BUFFER_SIZE);
        AUDIO_StartDMA();
        LWP_SemPost(audio_free);
    }
}

static void inline copy_to_buffer(char* buffer, char* stream, unsigned int length)
{
    memcpy(buffer, stream, length);
    // Make sure the buffer is in RAM, not the cache
    DCFlushRange(buffer, length);
}

static Int32 soundWrite(void* dummy, Int16 *stream, UInt32 length, Int32 *tuning)
{
    // This shouldn't lose any data and works for any size
    unsigned int lengthi;
    unsigned int lengthLeft = length * 2; /* length is in samples, convert to bytes */
    unsigned int stream_offset = 0;
    if( !audio_initialized ) {
        return 0;
    }
    LWP_SemWait(audio_free);
    while(1){
        lengthi = (buffer_offset + lengthLeft < BUFFER_SIZE) ?
                   lengthLeft : (BUFFER_SIZE - buffer_offset);

        copy_to_buffer(sample_buffer + buffer_offset,
                       (char *)stream + stream_offset, lengthi);

        if(buffer_offset + lengthLeft < BUFFER_SIZE){
                buffer_offset += lengthi;
                break;
        }

        lengthLeft    -= lengthi;
        stream_offset += lengthi;

        buffer_offset = 0;
    }
    // tuning
    if( !audio_paused && buffer_tuning ) {
        int tune = 0;
        if( buffer_tuning > BUFFER_SIZE / 4 )
            tune = -100;
        else if( buffer_tuning > BUFFER_SIZE / 8 )
            tune = -10;
        else if( buffer_tuning < -BUFFER_SIZE / 4 )
            tune = 100;
        else if( buffer_tuning > -BUFFER_SIZE / 8 )
            tune = 10;
        if( tune ) {
            *tuning += tune;
#if AUDIO_DEBUG
            printf("Audio: Tune %d, %d, %d\n", tune, *tuning, buffer_tuning);
#endif
        }
        buffer_tuning = 0;
    }
    LWP_SemPost(audio_free);

    return 0;
}

void archSoundCreate(Mixer* mixer, UInt32 sampleRate, UInt32 bufferSize, Int16 channels)
{
    if( audio_initialized ) {
        archSoundDestroy();
    }
    // Check parameters
    if( sampleRate != 48000 ) {
        fprintf(stderr, "Audio: Samplerate of %d not supported\n", sampleRate);
        exit(1);
    }
    if( channels != 2 ) {
        fprintf(stderr, "Audio: Only two channel stereo supported\n");
        exit(1);
    }
    // Init audio
    audio_paused = 1;
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
    LWP_SemInit(&audio_free, 1, 0xffffffff);
    AUDIO_RegisterDMACallback(done_playing);
    // Initialize mixer
    audio_mixer = mixer;
    mixerSetStereo(mixer, channels == 2);
    mixerSetWriteCallback(mixer, soundWrite, NULL, BUFFER_SIZE/64);
    audio_initialized = 1;
}

void archSoundDestroy(void)
{
    if( audio_initialized ) {
        // De-register to mixer
        mixerSetWriteCallback(audio_mixer, NULL, NULL, 0);
        // Stop DMA
        AUDIO_StopDMA();
        // Destroy semaphores and suspend the thread so audio can't play
        LWP_SemWait(audio_free);
        LWP_SemDestroy(audio_free);
        audio_paused = 0;
        audio_initialized = 0;
    }
}

void archSoundSuspend(void)
{
    if( audio_initialized && !audio_paused ) {
#if AUDIO_DEBUG
        printf("Audio: Pause\n");
#endif
        AUDIO_StopDMA();
        audio_paused = 1;
    }
}

void archSoundResume(void)
{
    if( audio_initialized && audio_paused ){
#if AUDIO_DEBUG
        printf("Audio: Resume\n");
#endif
        LWP_SemWait(audio_free);
        memset(sample_buffer, 0, sizeof(sample_buffer));
        DCFlushRange(sample_buffer, sizeof(sample_buffer));
        audio_paused = 0;
        buffer_offset = BUFFER_SIZE/2;
        buffer_tuning = 0;
        LWP_SemPost(audio_free);
        done_playing();
    }
}

