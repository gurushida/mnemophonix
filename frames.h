#ifndef _FRAMES_H
#define _FRAMES_H

#include "spectrogram.h"

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


struct frames {
    // The number of frames
    unsigned int n_frames;

    // A array of size (n_frames x NUMBER_OF_BINS) that gives for
    // each frame of SAMPLES_PER_FRAME samples a corresponding
    // spectrogram consisting of NUMBER_OF_BINS float values.
    // The array layout is as follows:
    //
    //  +---------------+---------------+----
    //  | spectrogram 0 | spectrogram 1 | ...
    //  +---------------+---------------+----
    //  0               32              64
    float* spectrograms;
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
 * compressed by calculating a spectrogram which groups frequencies
 * into a few bins.
 *
 * @param samples An array of float samples in [-1.0;1.0] at 5512Hz
 * @param n_samples The size of the array
 * @return The calculated frames or NULL in case of memory allocation error
 */
struct frames* build_frames(float* samples, unsigned int n_samples);


/**
 * Frees all the memory associated to the given frames.
 */
void free_frames(struct frames* frames);

#endif
