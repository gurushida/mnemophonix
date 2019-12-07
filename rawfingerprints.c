#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "rawfingerprints.h"


/**
 * In order to retain the N biggest coefficients, the easiest
 * is to sort them in a way that remembers their original positions.
 * The purpose of this structure is therefore to represent a value and
 * its original position as one unit.
 */
struct coeff_and_index {
    float coeff;
    unsigned int index;
};


/**
 * Compares the absolute values of the coefficients to sort them
 * in decreasing order.
 */
static int compare_by_absolute_values(struct coeff_and_index* a, struct coeff_and_index* b) {
    float abs_a = fabsf(a->coeff);
    float abs_b = fabsf(b->coeff);
    if (abs_a > abs_b) {
        return -1;
    }
    if (abs_a < abs_b) {
        return 1;
    }
    return 0;
}


static void convert_top_wavelets(struct coeff_and_index* sorted_data, struct rawfingerprint* fp) {
    for (unsigned int i = 0 ; i < TOP_WAVELETS ; i++) {
        if (sorted_data[i].coeff > 0.001) {
            int bit_pos = 2 * sorted_data[i].index;
            fp->bit_array[bit_pos / 8] |= (1 << (bit_pos % 8));
        } else if (sorted_data[i].coeff < -0.001) {
            int bit_pos = 2 * sorted_data[i].index + 1;
            fp->bit_array[bit_pos / 8] |= (1 << (bit_pos % 8));
        }
    }
}


struct rawfingerprints* build_raw_fingerprints(struct spectral_images* haar_transformed_images) {
    unsigned int n_images = haar_transformed_images->n_images;

    struct rawfingerprints* rfp = (struct rawfingerprints*)malloc(sizeof(struct rawfingerprints));
    if (rfp == NULL) {
        return NULL;
    }

    rfp->size = n_images;
    rfp->fingerprints = (struct rawfingerprint*)calloc(n_images, sizeof(struct rawfingerprint));
    if (rfp->fingerprints == NULL) {
        free(rfp);
        return NULL;
    }

    unsigned int N = NUMBER_OF_BINS * SPECTRAL_IMAGE_WIDTH;
    struct coeff_and_index temp[N];

    for (unsigned int i = 0 ; i < n_images ; i++) {
        // For each spectral image, let's copy the coefficients
        // and their positions into the temp array
        for (unsigned int j = 0 ; j < N ; j++) {
            temp[j].coeff = haar_transformed_images->images[i].image[j];
            temp[j].index = j;
        }

        // Let's sort this array
        qsort(temp, N, sizeof(struct coeff_and_index), (int (*)(const void *, const void *)) compare_by_absolute_values);

        // Let's retain the 200 highest wavelet coefficients and convert them
        // to 01, 10 or 00 whether they are negative, positive or null
        convert_top_wavelets(temp, &(rfp->fingerprints[i]));
    }

    return rfp;
}


void free_rawfingerprints(struct rawfingerprints* f) {
    free(f->fingerprints);
    free(f);
}
