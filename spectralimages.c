#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fft.h"
#include "hannwindow.h"
#include "spectralimages.h"


#define N_THREADS 8


struct frames_to_bins_job {
    float* samples;
    float* bins;
    float* hann_window;
    unsigned int first_frame;
    unsigned int last_frame;
    int return_code;
};


struct build_spectral_images_job {
    float* bins;
    struct spectral_image* images;
    unsigned int first_image;
    unsigned int last_image;
};


/**
 * Returns the number of frames we can have when
 * taking frames of SAMPLES_PER_FRAME samples every
 * INTERVAL_BETWEEN_FRAMES samples.
 */
static unsigned int get_n_frames(int n_samples) {
    return 1 + ((n_samples - SAMPLES_PER_FRAME) / INTERVAL_BETWEEN_FRAMES);
}


/**
 * Returns the number of spectral images we can have when
 * taking images of SPECTRAL_IMAGE_WIDTH frames every
 * DISTANCE_BETWEEN_SPECTRAL_IMAGE_START frames.
 */
static unsigned int get_n_images(int n_frames) {
    return 1 + ((n_frames - SPECTRAL_IMAGE_WIDTH) / DISTANCE_BETWEEN_SPECTRAL_IMAGE_START);
}


static float scale(float value, float max) {
    float scaled = 255.0 * value / max;
    if (scaled > 255.0) {
        scaled = 255.0;
    }
    return logf(1 + scaled) / logf(256.0);
}


/**
 * This normalizes the values contained in the given spectral image
 * to distribute them between 0.0 and 1.0.
 */
static void scale_to_full_spectrum(float* spectral_image) {
    float max = spectral_image[0];
    for (unsigned int i = 1 ; i < SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS ; i++) {
        float f = spectral_image[i];
        if (f > max) {
            max = f;
        }
    }

    for (unsigned int i = 0 ; i < SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS ; i++) {
        spectral_image[i] = scale(spectral_image[i], max);
    }
}


static int frames_to_bins(float* samples, float* bins, float* hann_window, unsigned int first_frame, unsigned int last_frame) {
    // We will now apply a Fast Fourier Transform (FFT) to each
    // frame, which will produce an array of complex numbers
    float* temp = (float*)malloc(SAMPLES_PER_FRAME * sizeof(float));
    float* real = (float*)malloc(SAMPLES_PER_FRAME * sizeof(float));
    float* imaginary = (float*)malloc(SAMPLES_PER_FRAME * sizeof(float));
    if (temp == NULL || real == NULL || imaginary == NULL) {
        free(temp);
        free(real);
        free(imaginary);
        return MEMORY_ERROR;
    }

    for (unsigned int i = first_frame ; i <= last_frame ; i++) {
        for (unsigned int j = 0 ; j < SAMPLES_PER_FRAME ; j++) {
            // Before calculating the Fast Fourier Transform, we
            // apply to each sample a coefficient to avoid spectral leakage
            temp[j] = samples[i * INTERVAL_BETWEEN_FRAMES + j] * hann_window[j];
        }
        fft(temp, real, imaginary);
        calculate_bins(real, imaginary, &(bins[i * NUMBER_OF_BINS]));
    }

    free(temp);
    free(real);
    free(imaginary);
    return SUCCESS;
}


static void* launch_frames_to_bins_job(struct frames_to_bins_job* job) {
    job->return_code = frames_to_bins(job->samples, job->bins, job->hann_window, job->first_frame, job->last_frame);
    return NULL;
}


static void* launch_build_spectral_images_job(struct build_spectral_images_job* job) {
    unsigned int size = SPECTRAL_IMAGE_WIDTH * NUMBER_OF_BINS * sizeof(float);
    for (unsigned int i = job->first_image ; i <= job->last_image ; i++) {
        memcpy(job->images[i].image, &(job->bins[i * DISTANCE_BETWEEN_SPECTRAL_IMAGE_START * NUMBER_OF_BINS]), size);
        scale_to_full_spectrum(job->images[i].image);
    }
    return NULL;
}


int build_spectral_images(float* samples, unsigned int n_samples, struct spectral_images* *images) {
    unsigned int n_frames = get_n_frames(n_samples);
    if (n_frames < SPECTRAL_IMAGE_WIDTH) {
        return FILE_TOO_SMALL;
    }

    (*images) = (struct spectral_images*)malloc(sizeof(struct spectral_images));
    if ((*images) == NULL) {
        return MEMORY_ERROR;
    }

    (*images)->n_images = get_n_images(n_frames);
    (*images)->images = (struct spectral_image*)malloc((*images)->n_images * sizeof(struct spectral_image));
    if ((*images)->images == NULL) {
        free(images);
        return MEMORY_ERROR;
    }

    // An array of size (n_frames x NUMBER_OF_BINS) that gives for
    // each frame of SAMPLES_PER_FRAME samples a corresponding
    // list of NUMBER_OF_BINS float values.
    // The array layout is as follows:
    //
    //  +---------------+---------------+----
    //  |    frame 0    |    frame 1    | ...
    //  +---------------+---------------+----
    //  0               32              64
    float* bins = (float*)malloc(n_frames * NUMBER_OF_BINS * sizeof(float));
    if (bins == NULL) {
        free(images);
        free((*images)->images);
        return MEMORY_ERROR;
    }

    pthread_t thread[N_THREADS];
    struct frames_to_bins_job jobs[N_THREADS];
    float* hann_window = get_Hann_window();

    unsigned int frames_per_thread = n_frames / N_THREADS;
    for (unsigned int k = 0 ; k < N_THREADS ; k++) {
        unsigned int start = k * frames_per_thread;
        unsigned int end = (k == N_THREADS - 1)
                        ? n_frames - 1
                        : (k + 1) * frames_per_thread - 1;
        jobs[k].samples = samples;
        jobs[k].bins = bins;
        jobs[k].hann_window = hann_window;
        jobs[k].first_frame = start;
        jobs[k].last_frame = end;
        jobs[k].return_code = SUCCESS;

        pthread_create(&(thread[k]), NULL, (void* (*)(void*))launch_frames_to_bins_job, &(jobs[k]));
    }

    int res = SUCCESS;
    for (unsigned int k = 0 ; k < N_THREADS ; k++) {
	    pthread_join(thread[k], NULL);
        if (jobs[k].return_code == MEMORY_ERROR) {
            res = MEMORY_ERROR;
        }
    }

    if (res == MEMORY_ERROR) {
        free_spectral_images((*images));
        return MEMORY_ERROR;
    }

    struct build_spectral_images_job spectral_image_jobs[N_THREADS];

    unsigned int n_spectral_image_threads = N_THREADS;
    if ((*images)->n_images < 2 * N_THREADS) {
        n_spectral_image_threads = 1;
    }

    // Now that we have calculated all the bins for all the frames,
    // it is time to build spectral images by grouping these bins

    unsigned int images_per_thread = (*images)->n_images / n_spectral_image_threads;
    for (unsigned int k = 0 ; k < n_spectral_image_threads ; k++) {
        unsigned int start = k * images_per_thread;
        unsigned int end = (k == n_spectral_image_threads - 1)
                        ? (*images)->n_images - 1
                        : (k + 1) * images_per_thread - 1;
        spectral_image_jobs[k].images = (*images)->images;
        spectral_image_jobs[k].bins = bins;
        spectral_image_jobs[k].first_image = start;
        spectral_image_jobs[k].last_image = end;
        pthread_create(&(thread[k]), NULL, (void* (*)(void*))launch_build_spectral_images_job, &(spectral_image_jobs[k]));
    }

    for (unsigned int k = 0 ; k < n_spectral_image_threads ; k++) {
	    pthread_join(thread[k], NULL);
    }

    return SUCCESS;
}


void free_spectral_images(struct spectral_images* images) {
    free(images->images);
    free(images);
}
