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
#define MIN_SIGNATURE_MATCHES 10

// Minimum average score that an audio sample must have
// with a given database entry to be retained
#define MIN_AVERAGE_SCORE 30

// A score above this value is a strong indicator that the
// sample may be a good match
#define GOOD_SCORE 35

/**
 * Returns the number of bytes that are identical between
 * the given hashes.
 */
static unsigned int compare_hashes(uint8_t* hash1, uint8_t* hash2) {
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


struct entry_score {
    int entry_index;
    float score;
    int n_matches;
};


/**
 * a < b if a is considered a better match than b.
 */
static int compare_entry_scores(struct entry_score* a, struct entry_score* b) {
    float average_score_a = a->n_matches == 0 ? 0 : a->score / (float)a->n_matches;
    float average_score_b = b->n_matches == 0 ? 0 : b->score / (float)b->n_matches;

    float score_delta = abs(average_score_a - average_score_b);

    // If the score are close enough and there is a big difference in number
    // of matches, we sort by matches
    if (score_delta <= 3) {
        if (score_delta <= 5 && a->n_matches >= (b->n_matches + 5)) {
            // If a has more at least five more matches than b, we prefer it
            return -1;
        }

        if (b->n_matches >= (a->n_matches + 5)) {
            // If b has more at least five more matches than a, we prefer it
            return 1;
        }
    }

    // If the scores are very close, let's sort by number of matches
    if (score_delta < 0.5) {
        if (a->n_matches > b->n_matches) {
            return -1;
        }

        if (a->n_matches < b->n_matches) {
            return 1;
        }
    }

    // If the number of matches are the same or if the scores are not too
    // close, let's sort by scores

    if (average_score_a > average_score_b) {
        return -1;
    }
    if (average_score_b > average_score_a) {
        return 1;
    }

    return 0;
}


int search(struct signatures* sample, struct index* database, struct lsh* lsh, int verbose) {
    struct entry_score* scores = (struct entry_score*)calloc(database->n_entries, sizeof(struct entry_score));
    if (scores == NULL) {
        return MEMORY_ERROR;
    }

    for (unsigned int i = 0 ; i < database->n_entries ; i++) {
        scores[i].entry_index = i;
        scores[i].score = 0;
        scores[i].n_matches = 0;
    }

    for (unsigned int i = 0 ; i < sample->n_signatures ; i++) {
        struct signature_list* list;
        int res = get_matches(lsh, sample->signatures[i].minhash, &list);
        if (res == MEMORY_ERROR) {
            free(scores);
            return MEMORY_ERROR;
        }

        // Now that we have partial matches, we will put those matches in a sorted array
        // to be able to count how many bucket matches we have per signature
        struct signature_list* array = (struct signature_list*)malloc(res * sizeof(struct signature_list));
        if (array == NULL) {
            free_signature_list(list);
            free(scores);
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
                        scores[entry_index].score += score;
                        scores[entry_index].n_matches++;
                    }
                }
                n_identical_matches = 1;
            }
        }

        free(array);
    }

    qsort(scores, database->n_entries, sizeof(struct entry_score), (int (*)(const void *, const void *)) compare_entry_scores);


    int best_match = NO_MATCH_FOUND;
    float best_score = 0;
    for (unsigned int i = 0 ; i < database->n_entries && i < 10; i++) {
        int index = scores[i].entry_index;
        float average_score = scores[i].n_matches == 0 ? 0 : (scores[i].score / (float)scores[i].n_matches);
        if (verbose) printf("average_score = %f, n_matches = %d (%s)\n", average_score, scores[i].n_matches, database->entries[index]->filename);
        if ((scores[i].n_matches >= MIN_SIGNATURE_MATCHES || (average_score >= GOOD_SCORE && scores[i].n_matches >= MIN_SIGNATURE_MATCHES / 2))
            && average_score >= MIN_AVERAGE_SCORE)
        if (average_score > best_score) {
            best_score = average_score;
            best_match = index;
        }
    }
    if (best_match != NO_MATCH_FOUND) {
        if (verbose) printf("\n*** match = %f %s ***\n", best_score, database->entries[best_match]->filename);
    }
    if (verbose) printf("-----------------------------\n");

    free(scores);

    return best_match;
}
