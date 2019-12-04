#ifndef _AUDIONORMALIZER_H
#define _AUDIONORMALIZER_H


/**
 * The loudness may vary between the original audio material
 * that is indexed and a recorded piece that we try to
 * identify with fingerprinting. Audio normalization helps
 * by making things more homogeneous.
 *
 * Normalizes the given audio samples in place using the Root
 * Mean Square method.
 */
void normalize(float* samples, unsigned int size);

#endif
