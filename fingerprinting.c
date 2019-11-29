#include <stdio.h>
#include <stdlib.h>
#include "fingerprinting.h"
#include "frames.h"
#include "haar.h"
#include "rawfingerprints.h"
#include "wav.h"




int generate_fingerprint(const char* wav) {
    // Let's make sure we have a wave file we can read
    struct wav_reader* reader;
    int res = new_wav_reader(wav, &reader);
    if (res != SUCCESS) {
        return res;
    }

    if (reader->artist != NULL) {
        printf("Artist: %s\n", reader->artist);
    }
    if (reader->track_title != NULL) {
        printf("Track title: %s\n", reader->track_title);
    }
    if (reader->album_title != NULL) {
        printf("Album title: %s\n", reader->album_title);
    }

    // Let's downsample the file into 5512Hz mono float samples between -1.0 and 1.0
    float* samples;
    int n = read_samples(reader, &samples);
    printf("%d 5512Hz mono samples\n", n);
    if (n < SAMPLES_PER_FRAME) {
        free(samples);
        free_wav_reader(reader);
        return n < 0 ? n : FILE_TOO_SMALL;
    }

    // TODO: normalize samples

    // Once we get the 5512Hz mono audio samples, the next step
    // is to build many small signatures corresponding to small
    // sample zones that overlap a lot.
    struct frames* frames = build_frames(samples, n);
    free(samples);

    if (frames == NULL) {
        free_wav_reader(reader);
        return MEMORY_ERROR;
    }

    printf("Got %d frames\n", frames->n_frames);
    apply_Haar_transform(frames);

    struct rawfingerprints* rawfingerprints = build_raw_fingerprints(frames);
    free_frames(frames);
    if (rawfingerprints == NULL) {
        free_wav_reader(reader);
        return MEMORY_ERROR;
    }

    free_rawfingerprints(rawfingerprints);
    free_wav_reader(reader);
    return SUCCESS;
}
