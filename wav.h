#ifndef _WAV_H
#define _WAV_H

#include <stdio.h>
#include "errors.h"

struct wav_reader {
    // The file descriptor we are reading from
    FILE* f;

    // The file size obtained when adding 8 to the
    // RIFF chunk size read from the file (8 corresponds
    // to the 4 bytes used for the 'RIFF' header and to the
    // 32-bit chunk size that follows)
    u_int32_t file_size;

    // This are the information in the format chunk
    int16_t wFormatTag;
    uint16_t wChannels;
    u_int32_t dwSamplesPerSec;
    u_int32_t dwAvgBytesPerSec;
    uint16_t wBlockAlign;
    uint16_t wBitsPerSample;

    // The start position of the sample data bytes given in bytes
    // from the beginning of the file
    u_int32_t data_chunk_position;

    // The number of sound sample data bytes
    u_int32_t data_chunk_size;

    // The artist name, track title and album title, if found in the metadata section
    // of the wave file, if any
    char* artist;
    char* track_title;
    char* album_title;
};


/**
 * Load the given file as a 16-bit PCM wave file.
 *
 * @param wav The path to the file to read
 * @param reader Where to store the bookkeeping data structure
 * @return SUCCESS if the file can be opened and actually looks like a wave file
 *         CANNOT_READ_FILE if the file cannot be read
 *         MEMORY_ERROR in case of memory allocation error
 *         DECODING_ERROR if the file does not look like a wave file
 *         UNSUPPORTED_WAVE_FORMAT if the file looks like a wave file of
 *                                 a format other than uncompressed integer
 *                                 16-bit 44100Hz PCM
 *         NOT_A_WAVE_FILE if the file does not even look like a wave file
 */
int new_wav_reader(const char* wav, struct wav_reader* *reader);

/**
 * Frees all the memory associated to the given reader and closes
 * the underlying file descriptor.
 */
void free_wav_reader(struct wav_reader* reader);


/**
 * Converts all the file into mono 5512Hz samples represented as float values
 * between -1.0 and 1.0 and stores them in the given array.
 *
 * @param reader The reader to read from
 * @param samples Where to allocate space for the results.
 *                The caller is responsible for freeing this array
 * @return the number of samples that were read on success
 *         DECODING_ERROR in case of I/O error when reading the file
 *         MEMORY_ERROR in case of memory allocation error
 */
int read_samples(struct wav_reader* reader, float* *samples);


/**
 * Converts 44100Hz 16-bit PCM samplest to mono 5512Hz samples
 * represented as float values between -1.0 and 1.0 and stores
 * them in the given array.
 *
 * @param src_samples The samples to convert
 * @param src_size The number of bytes in the source sample array
 * @param dst_samples Where to allocate space for the results.
 *                The caller is responsible for freeing this array
 * @return the number of samples that were read on success
 *         DECODING_ERROR in case of I/O error when reading the file
 *         MEMORY_ERROR in case of memory allocation error
 */
int convert_samples(uint8_t* src_samples, unsigned int src_size, float* *dst_samples);

#endif
