#include <stdio.h>
#include <stdlib.h>
#include "fingerprinting.h"
#include "frames.h"
#include "haar.h"
#include "minhash.h"
#include "rawfingerprints.h"
#include "wav.h"



int generate_fingerprint(const char* wav, struct signatures* *fingerprint,
                            char* *artist, char* *track_title, char* *album_title) {
    // Let's make sure we have a wave file we can read
    struct wav_reader* reader;
    int res = new_wav_reader(wav, &reader);
    if (res != SUCCESS) {
        return res;
    }

    if (reader->artist != NULL) {
        fprintf(stderr, "Artist: %s\n", reader->artist);
    }
    if (reader->track_title != NULL) {
        fprintf(stderr, "Track title: %s\n", reader->track_title);
    }
    if (reader->album_title != NULL) {
        fprintf(stderr, "Album title: %s\n", reader->album_title);
    }

    // Let's steal the metadata from the wav reader
    *artist = reader->artist;
    *track_title = reader->track_title;
    *album_title = reader->album_title;
    reader->artist = NULL;
    reader->track_title = NULL;
    reader->album_title = NULL;

    // Let's downsample the file into 5512Hz mono float samples between -1.0 and 1.0
    float* samples;
    int n = read_samples(reader, &samples);
    free_wav_reader(reader);
    fprintf(stderr, "%d 5512Hz mono samples\n", n);
    if (n < SAMPLES_PER_FRAME) {
        free(samples);
        return n < 0 ? n : FILE_TOO_SMALL;
    }

    // TODO: normalize samples

    // Once we get the 5512Hz mono audio samples, the next step
    // is to build many small signatures corresponding to small
    // sample zones that overlap a lot.
    struct frames* frames = build_frames(samples, n);
    free(samples);

    if (frames == NULL) {
        return MEMORY_ERROR;
    }

    fprintf(stderr, "Got %d frames\n", frames->n_frames);
    apply_Haar_transform(frames);

    struct rawfingerprints* rawfingerprints = build_raw_fingerprints(frames);
    free_frames(frames);
    if (rawfingerprints == NULL) {
        return MEMORY_ERROR;
    }

    struct signatures* signatures = build_signatures(rawfingerprints);
    free_rawfingerprints(rawfingerprints);
    if (signatures == NULL) {
        return MEMORY_ERROR;
    }
    fprintf(stderr, "Generated %d signatures\n", signatures->n_signatures);

    *fingerprint = signatures;
    return SUCCESS;
}
