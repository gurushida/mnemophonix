#ifndef _RESAMPLE_H
#define _RESAMPLE_H


/**
 * When resampling audio, a problem known as aliasing may occur.
 * When we go down from 44100Hz to 5512Hz, any frequency above 5512Hz
 * would become indistinguishable from other frequencies which
 * would make subsequent operations like the Fast Fourier Transform
 * produce corrupted results. The way to avoid this problem is to
 * apply a low pass filter that will eliminate the undesirable
 * frequencies. A low pass filter is a function that looks like this:
 *
 *
 *  0.125       /-\
 *             /   \
 *            /     \
 *  0 --+    /       \    +--
 *       \--/         \--/
 *
 * Since going from 44100Hz to 5512Hz is dividing by 8, the peak
 * value is 1/8 = 0.125
 *
 * In order to produce one sample at 5512Hz, we take N 44100Hz samples,
 * we multiply them by the low pass filter and we sum them.
 *
 *
 * Resamples the given samples to 5512Hz.
 *
 * @param samples_44100Hz Mono float samples between -1.0 and 1.0
 * @param n_samples The size of the input array
 * @return An array of n_samples/8 5512Hz samples or NULL in case
 *         of memory error
 */
float* resample(float* samples_44100Hz, unsigned int n_samples);

#endif
