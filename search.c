#include <math.h>
#include <string.h>
#include "search.h"


// Checking hashes by buckets of 4 bytes is meant to
// fail fast. This value is the minimum number of bucket
// matches that we require before giving a closer look
// at a potential match
#define MIN_BUCKET_MATCH_FOR_DEEP_CHECK 2


// Once a pair of hashes has at least MIN_BUCKET_MATCH_FOR_DEEP_CHECK
// bucket matches, we want a minimum score to retain this pair
#define MIN_SCORE 30


/**
 * In order to get forgiving hashing, each hash of 100 bytes is viewed
 * as 25 buckets of 4-byte hashes. Given, two hashes, this function
 * returns the number of buckets where the subhashes are identical.
 */
static unsigned int get_matching_buckets(u_int8_t* hash1, uint8_t* hash2) {
    unsigned int n = 0;
    for (unsigned int i = 0 ; i < SIGNATURE_LENGTH / 4 ; i++) {
        if (0 == memcmp(&(hash1[i * 4]), &(hash2[i * 4]), 4)) {
            n++;
        }
    }
    return n;
}


/**
 * Returns the number of bytes that are identical between
 * the given hashes.
 */
static unsigned int compare_hashes(u_int8_t* hash1, uint8_t* hash2) {
    unsigned int n = 0;
    for (unsigned int i = 0 ; i < SIGNATURE_LENGTH ; i++) {
        if (hash1[i] == hash2[i]) {
            n++;
        }
    }
    return n;
}


int search(struct signatures* sample, struct index* database) {
    int best_match = -1;
    int best_score = 0;

    // For each entry in the database
    for (unsigned int i = 0 ; i < database->n_entries ; i++) {
        struct index_entry* entry = database->entries[i];


        // We compare each signature of the entry wich each signature
        // of the sample
        unsigned int score_for_entry = 0;
        unsigned int buckets = 0;
        unsigned int matches = 0;
        for (unsigned int j = 0 ; j < entry->signatures->n_signatures ; j++) {
            for (unsigned int k = 0 ; k < sample->n_signatures ; k++) {
                int n_matching_buckets = get_matching_buckets(entry->signatures->signatures[j].minhash, sample->signatures[k].minhash);
                if (n_matching_buckets >= MIN_BUCKET_MATCH_FOR_DEEP_CHECK) {
                    unsigned int score = compare_hashes(entry->signatures->signatures[j].minhash, sample->signatures[k].minhash);
                    if (score >= MIN_SCORE) {
                        score_for_entry += score;
                        buckets += n_matching_buckets;
                        matches++;
                    }
                }
            }
        }

        float total_score = score_for_entry / (float)matches;
        if (total_score > best_score) {
            best_score = total_score;
            best_match = i;
        }
    }
    return best_match;
}
