#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "haar.h"


/**
 * Applies in place the 1-dimension Haar transform.
 *
 * @param data The values to transform
 * @param size The size of the array. Must be a power of 2
 */
static void transform_array(float* data, unsigned int size) {
    if (size != 32 && size != 128) {
        fprintf(stderr, "Internal error\n");
        exit(1);
    }
    float tmp[128];

    // We successively refine the array and retain its lower half
    // until we have only one element left
    while (size > 1) {
        size /= 2;
        for (unsigned int i = 0 ; i < size ; i++) {
            tmp[i] = (data[2 * i] + data[2 * i + 1]) / M_SQRT2;
            tmp[i + size] = (data[2 * i] - data[2 * i + 1]) / M_SQRT2;
        }
        for (unsigned int i = 0 ; i < 2 * size ; i++) {
            data[i] = tmp[i];
        }
    }
}


/**
 * Transforms in place a spectral image.
 *
 * @param data The frame buffer
 * @param image_index The index of the image to transform
 */
static void transform_image(struct spectral_image* image) {
    // The 2D standard Haar transform consists of applying
    // the 1D Haar transform to each row of the image and then
    // to each column of the result
    float row[SPECTRAL_IMAGE_WIDTH];

    // Let's transform the rows
    for (unsigned int y = 0 ; y < NUMBER_OF_BINS ; y++) {
        // For each row, we need to create a row
        // array that we can transform
        for (unsigned int i = 0 ; i < SPECTRAL_IMAGE_WIDTH ; i++) {
            row[i] = image->image[i * NUMBER_OF_BINS + y];
        }
        transform_array(row, SPECTRAL_IMAGE_WIDTH);
        // Now let's copy the transformed values back into the original array
        for (unsigned int i = 0 ; i < SPECTRAL_IMAGE_WIDTH ; i++) {
            image->image[i * NUMBER_OF_BINS + y] = row[i];
        }
    }

    // Now let's transform the columns. Since the columns are already
    // contiguous in the frame buffer, we don't need to copy data this time
    for (unsigned int i = 0 ; i < SPECTRAL_IMAGE_WIDTH ; i++) {
        transform_array(&(image->image[i * NUMBER_OF_BINS]), NUMBER_OF_BINS);
    }
}


void apply_Haar_transform(struct spectral_images* images) {
    for (unsigned int i = 0 ; i < images->n_images ; i++) {
        transform_image(&(images->images[i]));
    }
}
