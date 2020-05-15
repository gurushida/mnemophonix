#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fft.h"

static uint16_t reversed[2048] = { 0xFFFF };

/**
 * Given a 16-bit value like 00000ABCDEFGHIJK, this
 * returns the value obtained when reversing the 11
 * rightmost bits (00000KJIHGFEDCBA).
 */
static uint16_t reverse_bits(uint16_t n) {
    uint16_t res = 0;
    for (int i = 0 ; i < 11 ; i++) {
        int bit = n & (1 << i);
        if (bit) {
            res |= (1 << (10 - i));
        }
    }
    return res;
}


/**
 * This is the in place implementation of the Cooley-Tukey algorithm (source:
 * https://introcs.cs.princeton.edu/java/97data/InplaceFFT.java.html). Instead
 * of using recursion (like in https://introcs.cs.princeton.edu/java/97data/FFT.java.html),
 * this algorithm uses some clever bit manipulations on indexes
 * to make sure that values can be computed in place without
 * overwriting the values already computed.
 */
static void inplace_fft(float* real, float* imaginary) {
    for (int k = 0 ; k < 2048 ; k++) {
        int j = reversed[k];
        if (j > k) {
            float tmp_re = real[j];
            float tmp_im = imaginary[j];
            real[j] = real[k];
            imaginary[j] = imaginary[k];
            real[k] = tmp_re;
            imaginary[k] = tmp_im;
        }
    }

    for (int l = 2; l <= 2048; l *= 2) {
        for (int k = 0; k < l / 2; k++) {
            float kth = -2.0 * k * M_PI / l;
            float w_real = cosf(kth);
            float w_imaginary = sinf(kth);
            for (int j = 0; j < 2048 / l; j++) {
                int index = j * l + k + (l / 2);
                float tao_real = w_real * real[index] - w_imaginary * imaginary[index];
                float tao_imaginary = w_real * imaginary[index] + w_imaginary * real[index];

                int index2 = j * l + k;
                real[index] = real[index2] - tao_real;
                imaginary[index] = imaginary[index2] - tao_imaginary;

                real[index2] += tao_real;
                imaginary[index2] += tao_imaginary;
            }
        }
    }
}


int fft(float* source, float* real, float* imaginary) {
    // If needed, let's initialize the 'reversed' array
    if (reversed[0] == 0xFFFF) {
        for (int i = 0 ; i < 2048 ; i++) {
            reversed[i] = reverse_bits(i);
        }
    }

    // When computing the FFT on floats, we need to treat them
    // as complex numbers so let's use null imaginary values
    for (int i = 0 ; i < 2048 ; i++) {
        real[i] = source[i];
        imaginary[i] = 0.0;
    }

    inplace_fft(real, imaginary);
    return SUCCESS;
}
