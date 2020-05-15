#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "logbins.h"

#define MINIMUM_FREQUENCY 318
#define MAXIMUM_FREQUENCY 2000

static uint16_t bin_indexes[NUMBER_OF_BINS + 1] = { 0xFFFF };


/**
 * Given a frequency value between 0Hz and (5512 / 2)Hz, this function
 * returns the index between 1 and 1024 that corresponds to the closest
 * frequency in the FFT results, where the frequency for index i is given
 * by the following formula:
 *
 * frequency = samplingRate * i / nSamples
 */
static int frequency_to_index(float frequency) {
    int index = roundf(1024.0 * frequency / 2756.0);
    if (index < 1) {
        return 1;
    }
    if (index > 1024) {
        return 1024;
    }
    return index;
}


/**
 * For each of the 32 target bins, this function
 * calculates the frequency that corresponds to the
 * beginning of the logarithmic interval for this
 * bin. Then, this frequency is converted into
 * the corresponding index in the FFT results.
 *
 * This means that in order to calculate the value
 * for the bin #i, we need to take into account
 * all the FFT values between the indexes bin_indexes[i]
 * and bin_indexes[i+1].
 */
static void generate_bin_indexes() {
    float log_min = log2f(MINIMUM_FREQUENCY);
    float log_max = log2f(MAXIMUM_FREQUENCY);
    float delta = (log_max - log_min) / NUMBER_OF_BINS;

    float current = log_min;
    for (unsigned int i = 0 ; i <= NUMBER_OF_BINS ; i++) {
        float frequency = powf(2, current);
        current += delta;
        bin_indexes[i] = frequency_to_index(frequency);
    }
}


void calculate_bins(float* real, float* imaginary, float* bins) {
    if (bin_indexes[0] == 0xFFFF) {
        // If needed, let's initialize the bin indexes
        generate_bin_indexes();
    }

    for (unsigned int i = 0 ; i < NUMBER_OF_BINS ; i++) {
        unsigned int min_index = bin_indexes[i];
        unsigned int max_index = bin_indexes[i + 1];

        float sum = 0;
        for (unsigned int j = min_index ; j < max_index ; j++) {
            float re = real[j] / 1024.0;
            float im = imaginary[j] / 1024.0;
            sum += (re * re) + (im * im);
        }
        bins[i] = sum / (max_index - min_index);
    }
}
