#import <AVFoundation/AVFoundation.h>

#define TEN_SECONDS (10 * 44100 * 4)

@interface MicRecorder : NSObject <AVCaptureAudioDataOutputSampleBufferDelegate> {}

- (void)setParameters:(u_int8_t*)buffer
            nBytesInBuffer:(int*)nBytesInBuffer
            mutex:(pthread_mutex_t*)mutex
            condition:(pthread_cond_t*)cond;

- (void)notifyMatch;

@end
