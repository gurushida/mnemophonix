#include <math.h>
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


/**
 * Returns the number of spectral images we can have when
 * taking images of SPECTRAL_IMAGE_WIDTH frames every
 * DISTANCE_BETWEEN_SPECTRAL_IMAGE_START frames.
 */
static unsigned int get_n_images(int n_frames) {
    return 1 + ((n_frames - SPECTRAL_IMAGE_WIDTH) / DISTANCE_BETWEEN_SPECTRAL_IMAGE_START);
}


static float scale(float value, float max) {
    float scaled = 255.0 * value / max;
    if (scaled > 255.0) {
        scaled = 255.0;
    }
    return logf(1 + scaled) / logf(256.0);
}


/**
 * This normalizes the values contained in the given spectral image
 * to distribute them between 0.0 and 1.0.
 */
static void scale_to_full_spectrum(float* spectral_image) {
    float max = spectral_image[0];
    for (unsigned int i = 1 ; i < SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS ; i++) {
        float f = spectral_image[i];
        if (f > max) {
            max = f;
        }
    }

    for (unsigned int i = 0 ; i < SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS ; i++) {
        spectral_image[i] = scale(spectral_image[i], max);
    }
}


struct spectral_images* build_spectral_images(float* samples, unsigned int n_samples) {
    struct spectral_images* images = (struct spectral_images*)malloc(sizeof(struct spectral_images));
    if (images == NULL) {
        return NULL;
    }

    unsigned int n_frames = get_n_frames(n_samples);
    images->n_images = get_n_images(n_frames);
    images->images = (struct spectral_image*)malloc(images->n_images * sizeof(struct spectral_image));
    if (images->images == NULL) {
        free(images);
        return NULL;
    }

    // An array of size (n_frames x NUMBER_OF_BINS) that gives for
    // each frame of SAMPLES_PER_FRAME samples a corresponding
    // list of NUMBER_OF_BINS float values.
    // The array layout is as follows:
    //
    //  +---------------+---------------+----
    //  |    frame 0    |    frame 1    | ...
    //  +---------------+---------------+----
    //  0               32              64
    float* bins = (float*)malloc(n_frames * NUMBER_OF_BINS * sizeof(float));
    if (bins == NULL) {
        free(images);
        free(images->images);
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
        free(bins);
        free(images);
        free(images->images);
        return NULL;
    }

    float* hann_window = get_Hann_window();

    for (unsigned int i = 0 ; i < n_frames ; i++) {
        for (unsigned int j = 0 ; j < SAMPLES_PER_FRAME ; j++) {
            // Before calculating the Fast Fourier Transform, we
            // apply to each sample a coefficient to avoid spectral leakage
            temp[j] = samples[i * INTERVAL_BETWEEN_FRAMES + j] * hann_window[j];
        }
        fft(temp, real, imaginary);
        calculate_bins(real, imaginary, &(bins[i * NUMBER_OF_BINS]));
    }

    // Now that we have calculated all the bins for all the frames,
    // it is time to build spectral images by grouping these bins
    for (unsigned int i = 0 ; i < images->n_images ; i++) {
        unsigned int size = SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS * sizeof(float);
        memcpy(images->images[i].image, &(bins[i * DISTANCE_BETWEEN_SPECTRAL_IMAGE_START * NUMBER_OF_BINS]), size);
        scale_to_full_spectrum(images->images[i].image);
    }

    free(temp);
    free(real);
    free(imaginary);
    free(bins);

    return images;
}


void free_spectral_images(struct spectral_images* images) {
    free(images->images);
    free(images);
}
