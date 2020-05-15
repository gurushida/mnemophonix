#ifndef _RAWFINGERPRINTS_H
#define _RAWFINGERPRINTS_H

#include <stdint.h>
#include "haar.h"
#include "logbins.h"
#include "permutations.h"
#include "spectralimages.h"


// Number of wavelets to retain to create a raw fingerprint.
// Researchers have found that 200 is a good value
#define TOP_WAVELETS 200

// Size in bytes of a raw fingerprint
#define RAW_FINGERPRINT_SIZE ((NUMBER_OF_BINS * SPECTRAL_IMAGE_WIDTH * 2) / 8)

/**
 * The raw fingerprint of a spectral image consists of one tri-state
 * value for each value of the image. This structure encodes each
 * tri-state value as a 2-bit value (00, 01 or 10).
 */
struct rawfingerprint {
    // If not 0, means that this fingerprint is too close
    // to silence and should be skipped
    char is_silence;

    uint8_t bit_array[RAW_FINGERPRINT_SIZE];
};


/**
 * This structure represents all the raw fingerprints computed for
 * an audio input.
 */
struct rawfingerprints {
    // The number of raw fingerprints in the array
    unsigned int size;

    // The raw fingerprint array
    struct rawfingerprint* fingerprints;
};


/**
 * The result of the Haar transform on a spectral image is a matrix of the
 * same dimension as the image so that the non homogeneous parts of the
 * input produce bigger coefficients that the homogeneous parts. We can take
 * advantage of this by only retaining the N biggest coefficients in absolute value.
 * Moreover researchers have shown that the actual coefficients are not needed to
 * get an effective search, and that only using their signs is enough (Jacobs,  C.,
 * Finkelstein,  A.,  Salesin,  D.  (1995)  Fast  Multiresolution Image Querying.
 * in Proc of SIGGRAPH 95.). By representing negative values as 01, positive values
 * as 10 and other values as 00, this gives a very sparse bit array suitable for
 * further compression.
 *
 * Given spectral images that have already been transformed into Haar wavelets,
 * this function calculates raw fingerprints.
 *
 * @param haar_transformed_images The spectral images after the Haar transform
 * @return The raw fingerprints or NULL in case of memory allocation error
 */
struct rawfingerprints* build_raw_fingerprints(struct spectral_images* haar_transformed_images);


/**
 * Frees all the memory associated to the given raw fingerprints.
 */
void free_rawfingerprints(struct rawfingerprints* f);

#endif
