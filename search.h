#ifndef _SEARCH_H
#define _SEARCH_H

#include "fingerprintio.h"
#include "minhash.h"


/**
 * Given the fingerprint of an audio sample, this function looks
 * for the best match in the given database.
 *
 * @param sample The fingerprint of the sample to identify
 * @param database The database to search into
 * @return The index of the database entry or -1 if no good match is found
 */
int search(struct signatures* sample, struct index* database);

#endif
