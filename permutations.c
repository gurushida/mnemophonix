#include <stdlib.h>
#include "permutations.h"

// Since we are generating random permutations instead of hardcoding some permutations,
// we need to make sure that we generate the same permutations at index time and retrieval
// time otherwise we have no chance to ever match anything. Using a constant seed guarantees
// that the same permutations will be generated on all runs
#define PERMUTATION_SEED 678233

static u_int16_t permutations[N_PERMUTATIONS][PERMUTATION_LENGTH] = { { 0xFFFF } };


/**
 * Shuffles the array with Knuth shuffle.
 *
 * @param data An array of size 8192
 */
static void shuffle(u_int16_t* data) {
    for (unsigned int i = 0 ; i < 8190 ; i++) {
        // For each element #i, let's choose an element #j where j is between
        // i and the last element of the array, then let's swap them
        unsigned int j = i + rand() % (8192 - i);
        u_int16_t tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}


/**
 * Populates the given array with a permutation of the integers between 0 and 8191.
 * @param temp An array large enough to hold 8192 values
 */
static void create_permutation(u_int16_t* temp) {
    // First, let's populate the array
    for (unsigned int i = 0 ; i < 8192 ; i++) {
        temp[i] = i;
    }

    // Then let's shuffle it
    shuffle(temp);
}


/**
 * Generates random permutations.
 * @param seed The random generator seed so that we get deterministic results
 */
static void generate_permutations(unsigned int seed) {
    u_int16_t temp[8192];

    srand(seed);
    for (unsigned int i = 0 ; i < N_PERMUTATIONS ; i++) {
        create_permutation(temp);
        for (unsigned int j = 0 ; j < PERMUTATION_LENGTH ; j++) {
            permutations[i][j] = temp[j];
        }
    }
}


/**
 * For now, we generate random permutations using a constant seed
 * to make the results reproducible.
 */
u_int16_t* get_permutation(unsigned int n) {
    if (n >= N_PERMUTATIONS) {
        return NULL;
    }
    if (permutations[0][0] == 0xFFFF) {
        generate_permutations(PERMUTATION_SEED);
    }

    return &(permutations[n][0]);
}
