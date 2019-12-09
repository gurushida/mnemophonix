#import "MicRecorder.h"
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

@implementation MicRecorder

- (id)init
{
	self = [super init];
	return self;
}


// Let's keep a buffer of 11 seconds worth of 16-bit PCM data
#define BUFSIZE (11 * 44100 * 4)

// When the buffer grows past 10 seconds, we want to shift the data by 1 second
// to maintain a usable shifting window
#define ONE_SECOND (44100 * 4)

u_int8_t pcm_buffer[BUFSIZE];
int *nBytesInBuffer;
unsigned int pos_in_buffer = 0;

u_int8_t* exchange_pcm_buffer;
pthread_mutex_t* audio_mutex;
pthread_cond_t* condition;
char searchHit = 0;

-(void)setParameters:(u_int8_t*)buffer
                    nBytesInBuffer:(int*)n
                    mutex:(pthread_mutex_t*)mutex
                    condition:(pthread_cond_t*)cond;

{
    exchange_pcm_buffer = buffer;
    nBytesInBuffer = n;
    audio_mutex = mutex;
    condition = cond;
}

- (void)notifyMatch
{
    pthread_mutex_lock(audio_mutex);
    searchHit = 1;
    pthread_mutex_unlock(audio_mutex);
}


- (void)captureOutput:(AVCaptureOutput *)captureOutput
        didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
        fromConnection:(AVCaptureConnection *)connection
{
    CMItemCount numberOfFrames = CMSampleBufferGetNumSamples(sampleBuffer); // corresponds to the number of CoreAudio audio frames
		
    size_t total = CMSampleBufferGetTotalSampleSize(sampleBuffer);
    
    char data[total];
    AudioBuffer buffer;
    buffer.mDataByteSize = (uint32_t)total;
    buffer.mNumberChannels = 2;
    buffer.mData = &(data[0]);
    
    AudioBufferList audioBufferList;
    audioBufferList.mNumberBuffers = 1;
    audioBufferList.mBuffers[0] = buffer;

    CMSampleBufferCopyPCMDataIntoAudioBufferList(sampleBuffer, 0, (uint32_t)numberOfFrames, &audioBufferList);
    
    pthread_mutex_lock(audio_mutex);
    if (searchHit) {
        // When we have had a hit, we want to reset the buffer so that
        // if we switch to a different song, we won't try to analyse
        // the data already identified
        pos_in_buffer = 0;
        searchHit = 0;
    }
    pthread_mutex_unlock(audio_mutex);

    memcpy(pcm_buffer + pos_in_buffer, data, total);
    
    int seconds_before = pos_in_buffer / ONE_SECOND;
    pos_in_buffer += total;
    int seconds_after = pos_in_buffer / ONE_SECOND;
    
    if (pos_in_buffer > TEN_SECONDS) {
        memmove(pcm_buffer, pcm_buffer + ONE_SECOND, TEN_SECONDS);
        pos_in_buffer -= ONE_SECOND;
    }
    
    if (seconds_before != seconds_after && seconds_after >= 2) {
        pthread_mutex_lock(audio_mutex);
        (*nBytesInBuffer) = pos_in_buffer;
        memcpy(exchange_pcm_buffer, pcm_buffer, TEN_SECONDS);
        pthread_mutex_unlock(audio_mutex);
        pthread_cond_signal(condition);
    }
}

@end

