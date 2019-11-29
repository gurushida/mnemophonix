#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"


// Uncompressed integer PCM
#define WAVE_FORMAT_PCM 1

// Uncompressed float PCM
#define WAVE_FORMAT_IEEE_FLOAT 3


/**
 * Reads n bytes from the give file and store them
 * in the given buffer that is supposed to be large enough.
 * Returns 1 on success; 0 otherwise.
 */
static int read_bytes(FILE* f, size_t n, u_int8_t* buffer) {
    size_t res = fread(buffer, 1, n, f);
    return res == n;
}


/**
 * Reads an unsigned 32-bit integer stored in little endian order from the given
 * source.
 *
 * Returns 1 on success; 0 otherwise.
 */
static int read_uint32(struct wav_reader* reader, u_int32_t *n) {
    u_int8_t t[4];
    if (!read_bytes(reader->f, 4, t)) {
        return 0;
    }

    (*n) = (t[3] << 24) + (t[2] << 16) + (t[1] << 8) + t[0];
    return 1;
}


/**
 * Reads an unsigned 16-bit integer stored in little endian order from the given
 * source.
 *
 * Returns 1 on success; 0 otherwise.
 */
static int read_uint16(struct wav_reader* reader, u_int16_t *n) {
    u_int8_t t[2];
    if (!read_bytes(reader->f, 2, t)) {
        return 0;
    }

    (*n) = (t[1] << 8) + t[0];
    return 1;
}


/**
 * Reads a signed 16-bit integer stored in little endian order from the given
 * source.
 *
 * Returns 1 on success; 0 otherwise.
 */
static int read_int16(struct wav_reader* reader, int16_t *n) {
    u_int8_t t[2];
    if (!read_bytes(reader->f, 2, t)) {
        return 0;
    }

    (*n) = (t[1] << 8) + t[0];
    return 1;
}


/**
 * Read 12 bytes from the given file.
 * The first 4 bytes are supposed to be 'RIFF'.
 * The next 4 bytes are the size of file minus 8
 * in little endian order.
 * The next 4 bytes must be 'WAVE'.
 *
 * Returns 1 on success; 0 otherwise.
 */
static int read_file_header(struct wav_reader* reader) {
    u_int8_t t[4];

    if (!read_bytes(reader->f, 4, t) || 0 != memcmp(t, "RIFF", 4)) {
        return 0;
    }

    u_int32_t riff_chunk_size;
    if (!read_uint32(reader, &riff_chunk_size)) {
        return 0;
    }
    reader->file_size = 8 + riff_chunk_size;

    if (!read_bytes(reader->f, 4, t) || 0 != memcmp(t, "WAVE", 4)) {
        return 0;
    }

    return 1;
}


/**
 * Reads the format chunk that gives information about how
 * the data is encoded in this wav file.
 *
 * Returns SUCCESS on success
 *         DECODING_ERROR if the expected data cannot be read from the file
 *         UNSUPPORTED_WAVE_FORMAT if the file is not an uncompressed
 *                                 integer 16-bit 44100Hz PCM wave file
 */
static int read_format_chunk(struct wav_reader* reader) {
    u_int8_t t[4];
    if (!read_bytes(reader->f, 4, t) || 0 != memcmp(t, "fmt ", 4)) {
        return DECODING_ERROR;
    }

    u_int32_t size;
    if (!read_uint32(reader, &size) || size < 16) {
        return DECODING_ERROR;
    }

    if (!read_int16(reader, &(reader->wFormatTag))) return DECODING_ERROR;
    if (!read_uint16(reader, &(reader->wChannels))) return DECODING_ERROR;
    if (!read_uint32(reader, &(reader->dwSamplesPerSec))) return DECODING_ERROR;
    if (!read_uint32(reader, &(reader->dwAvgBytesPerSec))) return DECODING_ERROR;
    if (!read_uint16(reader, &(reader->wBlockAlign))) return DECODING_ERROR;
    if (!read_uint16(reader, &(reader->wBitsPerSample))) return DECODING_ERROR;

    if (reader->wFormatTag != WAVE_FORMAT_PCM || size != 16
        || reader->dwSamplesPerSec != 44100
        || reader->wBitsPerSample != 16) {
        return UNSUPPORTED_WAVE_FORMAT;
    }

    // For compressed wave files, there may be more fields to read
    // if size > 16 but since we don't support compressed wave files,
    // we don't have to worry about reading more bytes
    return SUCCESS;
}


/**
 * Skips the optional chunks, if any, that come after the format chunk.
 * The function stops when it has found the 'data' chunk ID. It then
 * reads the corresponding chunk size and save the position of the chunk
 * data in the reader structure.
 *
 * Returns SUCCESS on success
 *         DECODING_ERROR if the expected data cannot be read from the file
 */
static int skip_optional_chunks(struct wav_reader* reader) {
    u_int8_t t[4];
    u_int32_t chunk_size;
    while (1) {
        if (!read_bytes(reader->f, 4, t) || !read_uint32(reader, &chunk_size)) {
            return DECODING_ERROR;
        }

        if (0 == memcmp("data", t, 4)) {
            break;
        }
        // We have an unknown chunk, let's skip it
        if (0 != fseek(reader->f, chunk_size, SEEK_CUR)) {
            return DECODING_ERROR;
        }
    }

    reader->data_chunk_size = chunk_size;
    reader->data_chunk_position = ftell(reader->f);
    return SUCCESS;
}


/**
 * This function attempts to read a LIST chunk with an INFO section after the data chunk.
 *
 * Returns SUCCESS on success, including if no INFO chunk was found
 *         DECODING_ERROR if the expected data cannot be read from the file
 *         MEMORY_ERROR in case of memory allocation error
 */
static int read_info(struct wav_reader* reader) {
    u_int32_t pos_after_data = reader->data_chunk_position + reader->data_chunk_size;
    if (pos_after_data == reader->file_size) {
        return SUCCESS;
    }
    if (0 != fseek(reader->f, pos_after_data, SEEK_SET)) {
        return DECODING_ERROR;
    }

    u_int8_t t[4];
    u_int32_t size;
    if (!read_bytes(reader->f, 4, t) || !read_uint32(reader, &size)) {
        return DECODING_ERROR;
    }

    if (0 != memcmp("LIST", t, 4)) {
        // Not a LIST chunk ? Let's abort
        return SUCCESS;
    }

    if (!read_bytes(reader->f, 4, t)) {
        return DECODING_ERROR;
    }

    if (0 != memcmp("INFO", t, 4)) {
        // Not an INFO section in the LIST chunk ? Let's abort
        return SUCCESS;
    }

    // Appart from the 4 bytes of 'INFO', we know how many
    // bytes we should read
    u_int32_t n_to_read = size - 4;

    while (n_to_read > 0) {
        if (!read_bytes(reader->f, 4, t)) {
            return DECODING_ERROR;
        }
        if (!read_uint32(reader, &size)) {
            return DECODING_ERROR;
        }
        if (0 == memcmp("IART", t, 4) || 0 == memcmp("INAM", t, 4) || 0 == memcmp("IPRD", t, 4)) {
            char* s = (char*)malloc(size + 1);
            if (s == NULL) {
                return MEMORY_ERROR;
            }
            if (!read_bytes(reader->f, size, (u_int8_t*)s)) {
                return DECODING_ERROR;
            }
            s[size] = '\0';
            if (0 == memcmp("IART", t, 4)) {
                reader->artist = s;
            } else if (0 == memcmp("INAM", t, 4)) {
                reader->track_title = s;
            } else {
                reader->album_title = s;
            }
        } else {
            // It is not a metadata field we want, let's skip it
            if (0 != fseek(reader->f, size, SEEK_CUR)) {
                return DECODING_ERROR;
            }
        }

        n_to_read -= (8 + size);
    }

    return SUCCESS;
}


int new_wav_reader(const char* wav, struct wav_reader* *reader) {
    (*reader) = (struct wav_reader*)malloc(sizeof(struct wav_reader));
    if ((*reader) == NULL) {
        return MEMORY_ERROR;
    }
    (*reader)->artist = NULL;
    (*reader)->track_title = NULL;
    (*reader)->album_title = NULL;

    (*reader)->f = fopen(wav, "r");
    if ((*reader)->f == NULL) {
        free_wav_reader(*reader);
        return CANNOT_READ_FILE;
    }

    if (!read_file_header(*reader)) {
        free_wav_reader(*reader);
        return DECODING_ERROR;
    }

    int res;
    if (SUCCESS != (res = read_format_chunk(*reader))) {
        free_wav_reader(*reader);
        return res;
    }

    if (SUCCESS != (res = skip_optional_chunks(*reader))) {
        free_wav_reader(*reader);
        return res;
    }

    if (SUCCESS != (res = read_info(*reader))) {
        free_wav_reader(*reader);
        return res;
    }

    // Finally, before we return, let's seek to the beginning
    // of the data chunk to be ready to read samples
    fseek((*reader)->f, (*reader)->data_chunk_position, SEEK_SET);

    return SUCCESS;
}


void free_wav_reader(struct wav_reader* reader) {
    fclose(reader->f);
    free(reader->artist);
    free(reader->track_title);
    free(reader->album_title);
    free(reader);
}


/**
 * Returns the number of samples that can be read from the file assuming
 * that the file is downsampled to 5512Hz and that it is converted to mono.
 */
static unsigned int get_sample_count(struct wav_reader* reader) {
    // Number of 44100Hz samples
    int n_original_samples = (reader->data_chunk_size / reader->wBlockAlign);
    // Let's divide by 8 to get the number of samples at 5512Hz
    return n_original_samples / 8;
}


int read_samples(struct wav_reader* reader, float* *samples) {
    unsigned int n_samples = get_sample_count(reader);
    (*samples) = (float*)malloc(n_samples * sizeof(float));
    if ((*samples) == NULL) {
        return MEMORY_ERROR;
    }

    int n_src_bytes_for_one_dest_sample = 8 * reader->wBlockAlign;
    u_int8_t* src_samples = (u_int8_t*)malloc(n_src_bytes_for_one_dest_sample);
    if (src_samples == NULL) {
        return DECODING_ERROR;
    }
    for (unsigned int i = 0 ; i < n_samples ; i++) {
        if (!read_bytes(reader->f, n_src_bytes_for_one_dest_sample, src_samples)) {
            free(src_samples);
            free(*samples);
            return DECODING_ERROR;
        }
        int sum = 0;
        for (unsigned int j = 0 ; j < reader->wChannels ; j++) {
            // Each 16-bit sample must be converted to a signed int
            u_int16_t sample = src_samples[2 * j] + (src_samples[2 * j + 1] << 8);
            sum +=  (int16_t)sample;
        }
        // To get a mono float sample, we need to take the average by
        // dividing by the number of channels and then to normalize
        // the value between -32767.0 and 32767.0 into a value between -1.0 and 1.0
        float res = ((sum / (float)reader->wChannels)) / 32767.0;
        (*samples)[i] = res;
    }

    free(src_samples);
    return n_samples;
}
