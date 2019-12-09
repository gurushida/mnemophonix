#include <pthread.h>
#include <stdlib.h>
#import <Foundation/Foundation.h>
#import <AVFoundation/AVCaptureDevice.h>
#import <AVFoundation/AVCaptureInput.h>
#import <AVFoundation/AVCaptureOutput.h>
#import <AVFoundation/AVCaptureSession.h>
#include "MicRecorder.h"
#include "fingerprinting.h"
#include "fingerprintio.h"
#include "lsh.h"
#include "search.h"
#include "wav.h"


static pthread_mutex_t mutex;
static pthread_cond_t condition;

static pthread_mutex_t audio_mutex;


void startCapture(struct index* database_index, struct lsh* lsh) {
    printf("Starting to listen...\n");
    AVCaptureSession *captureSession = [[AVCaptureSession alloc] init];
    
    [captureSession beginConfiguration];
    AVCaptureDevice *audioDevice =
    [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];
    
    NSError *error;
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:audioDevice error:&error];
    [captureSession addInput:input];

    AVCaptureAudioDataOutput *output = [[AVCaptureAudioDataOutput alloc] init];
    [captureSession addOutput:output];
    
    [captureSession commitConfiguration];
    
    [captureSession startRunning];
    
    pthread_cond_init(&condition, NULL);
    pthread_mutex_init(&audio_mutex, NULL);
    NSDictionary *settings = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithUnsignedInt:kAudioFormatLinearPCM],                    AVFormatIDKey,
                             [NSNumber numberWithFloat:44100],                  AVSampleRateKey,
                             [NSNumber numberWithUnsignedInteger:2],  AVNumberOfChannelsKey,
                             [NSNumber numberWithInt:16],                AVLinearPCMBitDepthKey,
                             [NSNumber numberWithBool:NO],                                AVLinearPCMIsNonInterleaved,
                             [NSNumber numberWithBool:NO], AVLinearPCMIsFloatKey,
                              nil];
    
    output.audioSettings = settings;

    dispatch_queue_t audioDataOutputQueue = dispatch_queue_create("AudioDataOutputQueue", DISPATCH_QUEUE_SERIAL);
    MicRecorder *controller = [[MicRecorder alloc] init];
    int nBytesInBuffer;
    uint8_t pcm_buffer[TEN_SECONDS];
    [controller setParameters:pcm_buffer nBytesInBuffer:&nBytesInBuffer mutex:&audio_mutex condition:&condition];
    [output setSampleBufferDelegate:controller queue:audioDataOutputQueue];
    
    int last_match = -1;
    while (true) {
        pthread_cond_wait(&condition, &mutex);
        pthread_mutex_lock(&audio_mutex);
        float* samples;
        int res = convert_samples(pcm_buffer, nBytesInBuffer, &samples);
        pthread_mutex_unlock(&audio_mutex);
        
        if (res > 0) {
            struct signatures* fingerprint;
            res = generate_fingerprint_from_samples(samples, res, &fingerprint);
            free(samples);
            if (res == SUCCESS) {
                int best_match = search(fingerprint, database_index, lsh);
                free_signatures(fingerprint);
                if (best_match >= 0 && best_match != last_match) {
                    last_match = best_match;
                    printf("\nFound match: '%s'\n", database_index->entries[best_match]->filename);
                    if (database_index->entries[best_match]->artist[0]) {
                        printf("Artist: %s\n", database_index->entries[best_match]->artist);
                    }
                    if (database_index->entries[best_match]->track_title[0]) {
                        printf("Track title: %s\n", database_index->entries[best_match]->track_title);
                    }
                    if (database_index->entries[best_match]->album_title[0]) {
                        printf("Album title: %s\n", database_index->entries[best_match]->album_title);
                    }
                    printf("\n");
                    [controller notifyMatch];
                }
            }
        }
    }

    pthread_mutex_destroy(&audio_mutex);
    pthread_cond_destroy(&condition);
    [captureSession stopRunning];
}


int main(int argc, const char * argv[]) {
    @autoreleasepool {
    }
    if (argc != 2) {
        fprintf(stderr, "\n");
        fprintf(stderr, " ---                                                       ---\n");
        fprintf(stderr, " \\  \\  mnemophonix - A simple audio fingerprinting system  \\  \\\n");
        fprintf(stderr, " O  O                                                      O  O\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "%s <index>\n", argv[0]);
        fprintf(stderr, "  Loads the given database index and starts listening from the microphone");
        fprintf(stderr, "  to try to identify what is captured.\n");
        fprintf(stderr, "\n");
        exit(0);
    }
    
    struct index* database_index;
    printf(" ---                                                       ---\n");
    printf(" \\  \\  mnemophonix - A simple audio fingerprinting system  \\  \\\n");
    printf(" O  O                                                      O  O\n");
    printf("Loading '%s'...\n", argv[1]);
    int res = read_index(argv[1], &database_index);
    if (res != SUCCESS) {
        switch (res) {
            case CANNOT_READ_FILE: fprintf(stderr, "Cannot read file '%s'\n", argv[1]); return 1;
            case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
        }
    }
    struct lsh* lsh = create_hash_tables(database_index);
    printf("Database loaded...\n");
    
    pthread_mutex_init(&mutex, NULL);

    // Let's ask for permission to use the camera
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio])
    {
        case AVAuthorizationStatusAuthorized:
        {
            break;
        }
        case AVAuthorizationStatusNotDetermined:
        {
            // We use a mutex since the callback is executed on an arbitrary dispatch queue
            pthread_mutex_lock(&mutex);
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
                if (!granted) {
                    fprintf(stderr, "Access to microphone was denied\n");
                    exit(1);
                }
                pthread_mutex_unlock(&mutex);
            }];
            break;
        }
        case AVAuthorizationStatusDenied:
        {
            fprintf(stderr, "Access to microphone was denied\n");
            exit(1);
        }
        case AVAuthorizationStatusRestricted:
        {
            fprintf(stderr, "User is not allowed to access the microphone\n");
            exit(1);
            return 1;
        }
    }
    
    // We can get here either directly because of AVAuthorizationStatusAuthorized or
    // indirectly because of requestAccessForMediaType's callback giving us permission
    // from another thread. We use a mutex lock/unlock sequence to make sure that we only
    // proceed when we do have permission
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);

    startCapture(database_index, lsh);
    
    pthread_mutex_destroy(&mutex);
    return 0;
}
