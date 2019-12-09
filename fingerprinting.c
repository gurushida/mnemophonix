#include <stdio.h>
#include <stdlib.h>
#include "fingerprinting.h"
#include "haar.h"
#include "minhash.h"
#include "rawfingerprints.h"
#include "spectralimages.h"
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
    if (artist != NULL) *artist = reader->artist;
    if (track_title != NULL) *track_title = reader->track_title;
    if (album_title != NULL) *album_title = reader->album_title;
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

    // Once we get the 5512Hz mono audio samples, the next step
    // is to build many small signatures corresponding to small
    // sample zones that overlap a lot.
    struct spectral_images* spectral_images;
    res = build_spectral_images(samples, n, &spectral_images);
    free(samples);

    if (res != SUCCESS) {
        return res;
    }

    fprintf(stderr, "Got %d spectral images\n", spectral_images->n_images);
    fprintf(stderr, "Applying Haar transform to spectral images\n");
    apply_Haar_transform(spectral_images);

    fprintf(stderr, "Building raw fingerprints\n");
    struct rawfingerprints* rawfingerprints = build_raw_fingerprints(spectral_images);
    free_spectral_images(spectral_images);
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


int generate_fingerprint_from_samples(float* samples, unsigned int size, struct signatures* *fingerprint) {
    if (size < SAMPLES_PER_FRAME) {
        return FILE_TOO_SMALL;
    }

    struct spectral_images* spectral_images;
    int res = build_spectral_images(samples, size, &spectral_images);

    if (res != SUCCESS) {
        return res;
    }

    apply_Haar_transform(spectral_images);

    struct rawfingerprints* rawfingerprints = build_raw_fingerprints(spectral_images);
    free_spectral_images(spectral_images);
    if (rawfingerprints == NULL) {
        return MEMORY_ERROR;
    }

    struct signatures* signatures = build_signatures(rawfingerprints);
    free_rawfingerprints(rawfingerprints);
    if (signatures == NULL) {
        return MEMORY_ERROR;
    }

    *fingerprint = signatures;
    return SUCCESS;
}
