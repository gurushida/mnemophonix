#ifndef _MINHASH_H
#define _MINHASH_H

#include "rawfingerprints.h"

// The length in bytes of a MinHash signature
#define SIGNATURE_LENGTH 100

struct signature {
    // Each element of the signature is a value between 0 and 254.
    // 255 is the fallback value when no bit set to 1 can be found in
    // the first 255 values of a bit permutation. Giving up after 255
    // tries allows us to reduce the size of the signature. Since we
    // have built the raw fingerprints by retaining the top 200 wavelets,
    // there is a high probability that a raw fingerprint contains
    // 200/8192 bits set to 1, which is on average 1/40. By examining
    // 255 values, we have good probability to find a bit set to 1, so
    // this should not reduce much the precision of the results
    uint8_t minhash[SIGNATURE_LENGTH];
};


struct signatures {
    // The number of signatures
    unsigned int n_signatures;

    // The array containing the signatures
    struct signature* signatures;
};


/**
 * The raw fingerprints consist of sparse bit arrays. We can see a fingerprint
 * containing N bits as a set that can have up to N elements E(i) where i is
 * the index of each bit. For instance, the binary sequence 0110001 could be
 * represented as the set { E(1), E(2), E(6) }.
 *
 * Using this set representation, we could use the Jaccard similarity index
 * to measure how similar two sets are. It is obtained by dividing the number
 * of elements that appear in both sets by the size of the union of these sets.
 * Sets that have no element in common get an index of 0 and identical sets get
 * an index of 1.
 *
 * The problem however is that we would have to compare all fingerprint pairs
 * which would become very inefficient as the fingerprint database grows, which
 * is why we use the MinHash algorithm instead.
 *
 * The MinHash algorithm works by using a random but known permutation of the
 * bits that constitute the fingerprint. Then, we look for the first position
 * at which we can find a 1 bit. This position is one value that can be used to
 * characterize a given fingerprint. By generating multiple such values using
 * multiple bit permutations, we obtain a signature as long as we want.
 * For instance, let's assume that for a bit sequence ABCD EFGH IJKL MNOP we
 * use the permutations EBPK GALN IOFC JDHM, GLFH EPIJ KNCM BAOD and DKJP BNIL ACGF OMEH.
 *
 * The following fingerprint:
 *
 * ABCD EFGH IJKL MNOP
 * 0000 0010 1000 0010
 *
 * gets transformed into:
 *
 * EBPK GALN IOFC JDHM
 * 0000 1000 1100 0000  => first 1 bit at position 4
 *
 * GLFH EPIJ KNCM BAOD
 * 1000 0010 0000 0010  => first 1 bit at position 0
 *
 * DKJP BNIL ACGF OMEH
 * 0000 0010 0010 1000  => first 1 bit at position 6
 *
 * which gives the signature (4, 0, 6).
 *
 * When the bit arrays are large and sparse, such signatures are good hashes
 * to look for similar bit arrays.
 *
 *
 * Given raw fingerprints, this function calculates such a signature for each
 * fingerprint that does not produce a degenerate signature where all the values
 * of the signature are 255, which suggests that we are either extremely unlucky or
 * that the fingerprint corresponds to a silence sequence (either way, there is no
 * point in indexing a signature that has zero discriminatory power).
 *
 * @param rawfingerprints The raw fingerprints generated with Haar wavelets
 * @return The signatures built for the fingerprints or NULL in case
 *         of memory allocation error
 */
struct signatures* build_signatures(struct rawfingerprints* rawfingerprints);


/**
 * Frees all the memory associated to the given signatures.
 */
void free_signatures(struct signatures* signatures);


#endif
