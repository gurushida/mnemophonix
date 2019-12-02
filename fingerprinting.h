#ifndef _FINGERPRINTING_H
#define _FINGERPRINTING_H

#include "errors.h"
#include "minhash.h"


/**
 * Given a 16-bit 44100Hz PCM wave file, this function
 * calculates an audio fingerprint for this file.
 *
 * @param wav The path to the file to fingerprint
 * @pararm fingerprint Where to store the fingerprint
 * @return SUCCESS on success
 *         CANNOT_READ_FILE if the file cannot be read
 *         MEMORY_ERROR in case of memory allocation error
 *         DECODING_ERROR if the file does not look like a wave file
 *         UNSUPPORTED_WAVE_FORMAT if the file looks like a wave file of
 *                                 a format other than uncompressed integer
 *                                 16-bit 44100Hz PCM
 *         FILE_TOO_SMALL if the wave file is too small to generate a fingerprint
 */
int generate_fingerprint(const char* wav, struct signatures* *fingerprint);


#endif
