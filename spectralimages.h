#ifndef _SPECTRALIMAGES_H
#define _SPECTRALIMAGES_H

#include "errors.h"
#include "logbins.h"


// How many float samples we use in one frame. Choosing
// a power of 2 is not an accident as it will allow us to use the
// Cooley-Tukey algorithm to calculate efficiently the fast Fourier
// transforms that will be needed.
#define SAMPLES_PER_FRAME 2048

// When you want to recognize a random piece of audio, you cannot
// predict beforehand where it will start and stop. So in order to
// be robust, the frames must overlap a lot. This is why we start
// a new frame every 64 samples
#define INTERVAL_BETWEEN_FRAMES 64

// How many frames do we use to make a spectral image
// that will be transformed with Haar wavelets. It
// is no accident that this is a power of 2 as it makes
// it easier to implement the Haar transform
#define SPECTRAL_IMAGE_WIDTH 128

// When we have converted each audio frame into a frame of 32 bins,
// these frames will be grouped to form spectral images. To make
// things more robust, the spectral images must overlap a lot, so
// we start a new one every 8 frames
#define DISTANCE_BETWEEN_SPECTRAL_IMAGE_START 8

/**
 * This represent one spectral image obtained
 * by putting together the bins of
 * SPECTRAL_IMAGE_WIDTH frames.
 */
struct spectral_image {
    float image[SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS];
};



struct spectral_images {
    // The number of spectral images
    unsigned int n_images;

    // The spectral images associated to the frames
    struct spectral_image* images;
};


/**
 * Given an array of audio samples, this function decomposes it
 * into zones that overlap a lot for robustness:
 *
 * Frame 0 =====================
 * Frame 1  =====================
 * Frame 2   =====================
 * ...
 *
 * For each zone, it decomposes the signal per frequencies using
 * the Fast Fourier Transform. The resulting information is then
 * compressed by calculating a spectral image which groups frequencies
 * into a few bins.
 *
 * @param samples An array of float samples in [-1.0;1.0] at 5512Hz
 * @param n_samples The size of the array
 * @param images Where to store the results
 * @return SUCCESS on success
 *         FILE_TOO_SMALL if there are not enough samples to compute at least one frame
 *         MEMORY_ERROR in case of memory allocation error
 */
int build_spectral_images(float* samples, unsigned int n_samples, struct spectral_images* *images);


/**
 * Frees all the memory associated to the given spectral images.
 */
void free_spectral_images(struct spectral_images* images);

#endif
