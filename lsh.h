#ifndef _LSH_H
#define _LSH_H

#include "fingerprintio.h"
#include "minhash.h"

#define BYTES_PER_BUCKET_HASH 4

#define N_BUCKETS (SIGNATURE_LENGTH / BYTES_PER_BUCKET_HASH)


/**
 * This list structure is meant to represent a list of signatures.
 */
struct signature_list {
    // Index of the entry in the database this signature belongs to
    unsigned int entry_index;
    // Index of the the signature
    unsigned int signature_index;
    // Next list item
    struct signature_list* next;
};


/**
 * This structure represents one hash table per bucket.
 */
struct lsh {
    // The size of each hash table
    unsigned int size;

    // The array containing one hash table per bucket
    struct signature_list** buckets[N_BUCKETS];
};


/**
 * LSH stands for Locality Sensitive Hashing. Instead of using one big hash
 * per complex object that would be very likely to differ, this approach
 * consists of splitting the signature into many small hashes. The idea is
 * that if two hashes are similar, we can detect this similarity by counting
 * the small hashes they have in common. Completely different hashes will fail
 * fast on this, but similar ones will give results that can be further examined,
 * for instance by computing the raw distance between the full hashes.
 *
 * Given a raw database, returns a structure containing one hash table per bucket,
 * or NULL in case of memory allocation error.
 */
struct lsh* create_hash_tables(struct index* database);


/**
 * Frees all the memory associated to the given hash tables.
 */
void free_hash_tables(struct lsh* tables);


/**
 * Creates a new signature list item.
 */
struct signature_list* new_signature_list(unsigned int entry_index, unsigned int signature_index, struct signature_list* next);


/**
 * Frees all the memory associated to the given list.
 */
void free_signature_list(struct signature_list* list);


/**
 * Returns a list containing all the partial matches found for the
 * given hash in the given hash tables.
 *
 * @param tables The LSH tables to look into
 * @param hash A MinHash signature of SIGNATURE_LENGTH bytes
 * @param list Where to store all the partial matches. The caller
 *             is responsible for freeing it after use
 * @return The length of the list on success
 *         MEMORY_ERROR in case of memory allocation error
 */
int get_matches(struct lsh* tables, uint8_t* hash, struct signature_list* *list);


#endif
