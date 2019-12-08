#ifndef _SEARCH_H
#define _SEARCH_H

#include "errors.h"
#include "fingerprintio.h"
#include "lsh.h"
#include "minhash.h"


/**
 * Given the fingerprint of an audio sample, this function looks
 * for the best match in the given database.
 *
 * @param sample The fingerprint of the sample to identify
 * @param database The database to search into
 * @param lsh The hash tables to use for efficiency
 * @return The index of the database entry on success
 *         NO_MATCH_FOUND if no good match is found
 *         MEMORY_ERROR in case of memory allocation error
 */
int search(struct signatures* sample, struct index* database, struct lsh* lsh);



#endif
