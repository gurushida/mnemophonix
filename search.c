#include <stdlib.h>
#include <string.h>
#include "lsh.h"
#include "search.h"


// Checking hashes by buckets of 4 bytes is meant to
// fail fast. This value is the minimum number of bucket
// matches that we require before giving a closer look
// at a potential match
#define MIN_BUCKET_MATCH_FOR_DEEP_CHECK 2


// Once a pair of hashes has at least MIN_BUCKET_MATCH_FOR_DEEP_CHECK
// bucket matches, we want a minimum score to retain this pair
#define MIN_SCORE 30


// Minimum number of full signature matches that an audio sample must have
// with a given database entry to be retained
#define  MIN_SIGNATURE_MATCHES 2


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


static int compare(struct signature_list* a, struct signature_list* b) {
    int diff = a->entry_index - b->entry_index;
    if (diff != 0) {
        return diff;
    }
    return a->signature_index - b->signature_index;
}


int search(struct signatures* sample, struct index* database, struct lsh* lsh) {
    float* scores = (float*)calloc(database->n_entries, sizeof(float));
    if (scores == NULL) {
        return MEMORY_ERROR;
    }
    int* n_matches = (int*)calloc(database->n_entries, sizeof(int));
    if (n_matches == NULL) {
        free(scores);
        return MEMORY_ERROR;
    }


    for (unsigned int i = 0 ; i < sample->n_signatures ; i++) {
        struct signature_list* list;
        int res = get_matches(lsh, sample->signatures[i].minhash, &list);
        if (res == MEMORY_ERROR) {
            free(scores);
            free(n_matches);
            return MEMORY_ERROR;
        }

        // Now that we have partial matches, we will put those matches in a sorted array
        // to be able to count how many bucket matches we have per signature
        struct signature_list* array = (struct signature_list*)malloc(res * sizeof(struct signature_list));
        if (array == NULL) {
            free_signature_list(list);
            free(scores);
            free(n_matches);
            return MEMORY_ERROR;
        }

        for (int j = 0 ; j < res ; j++, list = list->next) {
            array[j].entry_index = list->entry_index;
            array[j].signature_index = list->signature_index;
        }
        free_signature_list(list);

        qsort(array, res, sizeof(struct signature_list), (int (*)(const void *, const void *)) compare);

        unsigned int n_identical_matches = 1;
        for (int j = 1 ; j < res ; j++) {
            if (array[j].entry_index == array[j - 1].entry_index
                && array[j].signature_index == array[j - 1].signature_index) {
                    n_identical_matches++;
            } else {
                if (n_identical_matches >= MIN_BUCKET_MATCH_FOR_DEEP_CHECK) {
                    int entry_index = array[j - 1].entry_index;
                    int signature_index = array[j - 1].signature_index;
                    unsigned int score = compare_hashes(database->entries[entry_index]->signatures->signatures[signature_index].minhash,
                                                        sample->signatures[i].minhash);
                    if (score >= MIN_SCORE) {
                        scores[entry_index] += score;
                        n_matches[entry_index]++;
                    }
                }
                n_identical_matches = 1;
            }
        }

        free(array);
    }

    int best_match = NO_MATCH_FOUND;
    int best_score = 0;
    for (unsigned int i = 0 ; i < database->n_entries ; i++) {
        float average_score = scores[i] / (float)n_matches[i];
        if (n_matches[i] >= MIN_SIGNATURE_MATCHES)
        if (average_score > best_score) {
            best_score = average_score;
            best_match = i;
        }
    }

    free(scores);
    free(n_matches);

    return best_match;
}
