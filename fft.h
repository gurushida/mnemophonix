#ifndef _FFT_H
#define _FFT_H

#include "errors.h"

/**
 * Given an array of real numbers representing a signal,
 * the Fourier transform calculates a decomposition of
 * the signal into a sum of cosine and sine functions that
 * represent primitives periodic signals that, when combined
 * together, produce the original signal.
 *
 * For instance, if you have an input made of 2 pure waveforms
 * corresponding to a loud 440Hz tone and an half as loud 220Hz tone,
 * the Fourier transform will tell you all that by giving a large
 * coefficient for the 440Hz frequency, a smaller coefficient for the
 * 220Hz frequency and 0 for all the other frequencies.
 *
 *
 * This function calculates the Fast Fourier Transform on the given
 * values. By design, the first complex value in the result is the sum
 * of all the inputs. Since our inputs are real numbers, they all
 * have 0 as their imaginary value, so real[0] will be the sum of the
 * values in the input array and imaginary[0] will be 0.
 *
 * Moreover, when the inputs are real numbers, the results contain
 * redundant information as for each complex result[x] = a + i.b
 * in the first part of the array, there is a corresponding conjugate
 * in the second half: result[2048 - x] = a - i.b
 * As a consequence, it is enough to operate on the first half of the
 * results, i.e. by only using the values from result[1] to result[1024].
 * For each such value result[i] (with i between 1 and 1024), the frequency
 * corresponding to this coefficient is given by the formula:
 *
 * frequency = samplingRate * i / nSamples
 *
 * @param source An input array of 2048 values
 * @param real An array of 2048 float representing the real parts
 *             of the results
 * @param imaginary An array of 2048 float representing the imaginary
 *             parts of the results
 * @return SUCCESS on success
 *         MEMORY_ERROR in case of memory allocation error
 */
int fft(float* source, float* real, float* imaginary);

#endif
