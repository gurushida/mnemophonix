#ifndef _HANNWINDOW_H
#define _HANNWINDOW_H

#include "spectralimages.h"


/**
 * When using the Fast Fourier Transform to decompose a signal
 * into frequencies, we would expect that, if the signal is a pure
 * tone using a single frequency, the output would contain exactly
 * one non-zero coefficient for this specific frequency.
 * Unfortunately, it does not work that easily due to a phenomemon
 * known as spectral leakage. This is an issue due to using
 * discrete windows of samples. The way to mitigate this problem
 * is to apply a bell-shaped function to all the samples before
 * computing the Fast Fourier Transform. As it turns out, a common
 * function used for this purpose is the Hann function.
 *
 * This returns an array of SAMPLES_PER_FRAME float values
 * representing the Hann function.
 */
float* get_Hann_window();

#endif

