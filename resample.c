#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "resample.h"


#define FILTER_SIZE 31

static float low_pass_filter[FILTER_SIZE] = { 666 };


static float sinc(float x) {
    return sinf(M_PI * x) / (M_PI * x);
}


static float blackman_window(float x) {
    return (0.42 - 0.5 * cosf(2 * M_PI * (x - 15) / 30) + 0.08 * cosf(4 * M_PI * (x-15) / 30));
}

/**
 * This creates a low pass filter using the recipe found at:
 * https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter
 *
 * The filter is represented by an array of discrete values taken from the curve.
 */
static void initialize_low_pass_filter() {
    for (int x = -15 ; x <= 15 ; x++ ) {
        if (x == 0) {
            low_pass_filter[x + 15] = 0.125;
        } else {
            low_pass_filter[x + 15] = 0.125 * sinc(x * 0.125) * blackman_window(x);
        }
    }
}


static float get_5512Hz_sample(float* samples_44100Hz, unsigned int start, unsigned int n_samples) {
    float res = 0;
    for (unsigned int j = 0 ; j < FILTER_SIZE && (start + j) < n_samples; j++) {
        res += samples_44100Hz[start + j] * low_pass_filter[j];
    }
    return res;
}


float* resample(float* samples_44100Hz, unsigned int n_samples) {
    if (low_pass_filter[0] > 600) {
        initialize_low_pass_filter();
    }

    float* samples_5512Hz = (float*)malloc((n_samples / 8) * sizeof(float));
    if (samples_5512Hz == NULL) {
        return NULL;
    }

    for (unsigned int i = 0 ; i < (n_samples / 8) ; i++) {
        samples_5512Hz[i] = get_5512Hz_sample(samples_44100Hz, i * 8, n_samples);
    }

    return samples_5512Hz;
}
