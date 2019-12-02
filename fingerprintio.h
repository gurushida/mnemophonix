#ifndef _FINGERPRINTIO_H
#define _FINGERPRINTIO_H

#include <stdio.h>
#include "minhash.h"


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


#endif
