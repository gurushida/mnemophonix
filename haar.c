#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "haar.h"

#define N_THREADS 8


struct haar_transform_job {
    struct spectral_image* images;
    unsigned int first_image;
    unsigned int last_image;
};


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
 */
void transform_image(struct spectral_image* image) {
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


static void* launch_Haar_job(struct haar_transform_job* job) {
    for (unsigned int i = job->first_image ; i <= job->last_image ; i++) {
        transform_image(&(job->images[i]));
    }

    return NULL;
}


void apply_Haar_transform(struct spectral_images* spectral_images) {
    unsigned int n_Haar_threads = N_THREADS;
    if (spectral_images->n_images < 2 * N_THREADS) {
        n_Haar_threads = 1;
    }
    pthread_t thread[N_THREADS];
    struct haar_transform_job haar_job[N_THREADS];
    unsigned int images_per_thread = spectral_images->n_images / n_Haar_threads;

    for (unsigned int k = 0 ; k < n_Haar_threads ; k++) {
        unsigned int start = k * images_per_thread;
        unsigned int end = (k == n_Haar_threads - 1)
                        ? spectral_images->n_images - 1
                        : (k + 1) * images_per_thread - 1;
        haar_job[k].images = spectral_images->images;
        haar_job[k].first_image = start;
        haar_job[k].last_image = end;

        pthread_create(&(thread[k]), NULL, (void* (*)(void*))launch_Haar_job, &(haar_job[k]));
    }
    for (unsigned int k = 0 ; k < n_Haar_threads ; k++) {
        pthread_join(thread[k], NULL);
    }
}

