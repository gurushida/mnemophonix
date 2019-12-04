#include <stdlib.h>
#include "fft.h"
#include "frames.h"
#include "hannwindow.h"
#include "spectrogram.h"

/**
 * Returns the number of frames we can have when
 * taking frames of SAMPLES_PER_FRAME samples every
 * INTERVAL_BETWEEN_FRAMES samples.
 */
static unsigned int get_n_frames(int n_samples) {
    return 1 + ((n_samples - SAMPLES_PER_FRAME) / INTERVAL_BETWEEN_FRAMES);
}


struct frames* build_frames(float* samples, unsigned int n_samples) {
    struct frames* frames = (struct frames*)malloc(sizeof(struct frames));
    if (frames == NULL) {
        return NULL;
    }

    frames->n_frames = get_n_frames(n_samples);
    frames->spectrograms = (float*)malloc(frames->n_frames * NUMBER_OF_BINS * sizeof(float));
    if (frames->spectrograms == NULL) {
        free(frames);
        return NULL;
    }

    // We will now apply a Fast Fourier Transform (FFT) to each
    // frame, which will produce an array of complex numbers
    float* temp = (float*)malloc(SAMPLES_PER_FRAME * sizeof(float));
    float* real = (float*)malloc(SAMPLES_PER_FRAME * sizeof(float));
    float* imaginary = (float*)malloc(SAMPLES_PER_FRAME * sizeof(float));
    if (temp == NULL || real == NULL || imaginary == NULL) {
        free(temp);
        free(real);
        free(imaginary);
        free(frames->spectrograms);
        free(frames);
        return NULL;
    }

    float* hann_window = get_Hann_window();

    for (unsigned int i = 0 ; i < frames->n_frames ; i++) {
        for (unsigned int j = 0 ; j < SAMPLES_PER_FRAME ; j++) {
            // Before calculating the Fast Fourier Transform, we
            // apply to each sample a coefficient to avoid spectral leakage
            temp[j] = samples[i * INTERVAL_BETWEEN_FRAMES + j] * hann_window[j];
        }
        fft(temp, real, imaginary);
        create_spectrogram(real, imaginary, &(frames->spectrograms[i * NUMBER_OF_BINS]));
    }

    free(temp);
    free(real);
    free(imaginary);

    return frames;
}


void free_frames(struct frames* frames) {
    free(frames->spectrograms);
    free(frames);
}
