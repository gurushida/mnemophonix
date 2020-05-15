#ifndef _PERMUTATIONS_H
#define _PERMUTATIONS_H

#include <stdint.h>

// Number of permutations to use to generate signatures
#define N_PERMUTATIONS 100

// Number of indexes to retain from a permutation
#define PERMUTATION_LENGTH 255


/**
 * The MinHash algorithm needs permutations of all the bits that compose
 * raw fingerprints. Since a raw fingerprint contains 8192 bits, a way to
 * represent such a permutation is to shuffle an array containing all the
 * integers from 0 to 8191. Then, instead of scanning all the bits from bit[0]
 * to bit[8191], the caller will scan the bits from bit[permutation[0]] to
 * bit[permutation[8191]].
 *
 *
 * Returns an array containing the first PERMUTATION_LENGTH
 * indexes of the permutation #n.
 *
 * @param n Index of the permutation between 0 and N_PERMUTATIONS - 1
 * @return An array of PERMUTATION_LENGTH integers between 0 and 8191
 *         or NULL if n is invalid
 */
uint16_t* get_permutation(unsigned int n);


#endif
