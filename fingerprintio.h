#ifndef _FINGERPRINTIO_H
#define _FINGERPRINTIO_H

#include <stdio.h>
#include "errors.h"
#include "minhash.h"

/**
 * This structure represents one entry in a fingerprint database.
 */
struct index_entry {
    char* filename;
    char* artist;
    char* track_title;
    char* album_title;
    struct signatures* signatures;
};

/**
 * This structure represents a fingerprint database.
 */
struct index {
    // Then number of entries
    unsigned int n_entries;

    // The entries
    struct index_entry** entries;
};

/**
 * Saves the given signature to the given file using the following text format.
 * A fingerprint entry starts with 5 lines of text:
 * - name of the original .wav file
 * - artist (may be empty)
 * - track title (may be empty)
 * - album title (may be empty)
 * - number of hashes
 *
 * This is followed by one line per hash, where each hash line consists of the hexadecimal representation
 * of the 100 bytes that constitute a hash.
 *
 * @param f The file to save to
 * @param fingerprint The fingerprint to save
 * @param wavname The name of the .wav file that was fingerprinted
 * @param artist If not NULL, the artist name to save
 * @param track_title If not NULL, the track title to save
 * @param album_title If not NULL, the album title to save
 */
void save(FILE* f, struct signatures* fingerprint, const char* wavname,
            const char* artist, const char* track_title, const char* album_title);

/**
 * Loads the index contained in the given file.
 *
 * @param filename The file to load
 * @param index Where to store the results
 * @return SUCCESS on success
 *         MEMORY_ERROR in case of memory allocation error
 *         CANNOT_READ_FILE if the file cannot be read
 *         DECODING_ERROR if the file cannot be parsed correctly
 */
int read_index(const char* filename, struct index* *index);


/**
 * Frees all the memory associated to the given index.
 */
void free_index(struct index* index);

#endif
