#ifndef _FINGERPRINTING_H
#define _FINGERPRINTING_H

#include "errors.h"
#include "minhash.h"


/**
 * Given a 16-bit 44100Hz PCM wave file, this function
 * calculates an audio fingerprint for this file.
 *
 * @param wav The path to the file to fingerprint
 * @param fingerprint Where to store the fingerprint
 * @param artist Where to store the artist name read from the .wav file, if any
 * @param track_title Where to store the track title read from the .wav file, if any
 * @param album_title Where to store the album title read from the .wav file, if any
 * @return SUCCESS on success
 *         CANNOT_READ_FILE if the file cannot be read
 *         MEMORY_ERROR in case of memory allocation error
 *         DECODING_ERROR if the file does not look like a wave file
 *         UNSUPPORTED_WAVE_FORMAT if the file looks like a wave file of
 *                                 a format other than uncompressed integer
 *                                 16-bit 44100Hz PCM
 *         FILE_TOO_SMALL if the wave file is too small to generate a fingerprint
 */
int generate_fingerprint(const char* wav, struct signatures* *fingerprint,
                            char* *artist, char* *track_title, char* *album_title);


/**
 * Given an array of mono float 5512Hz samples, this function
 * calculates an audio fingerprint.
 *
 * @param samples The samples
 * @param size The number of samples
 * @param fingerprint Where to store the fingerprint
 * @return SUCCESS on success
 *         MEMORY_ERROR in case of memory allocation error
 *         FILE_TOO_SMALL if the sample array is too small to generate a fingerprint
 */
int generate_fingerprint_from_samples(float* samples, unsigned int size, struct signatures* *fingerprint);


#endif
