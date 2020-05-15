#include <stdlib.h>
#include "minhash.h"
#include "permutations.h"


/**
 * Given a fingerprint, calculates the corresponding signature.
 *
 * @param fp The raw fingerprint
 * @param signature The signature object to populate
 * @return 1 in case of success or 0 if the all the values of the signature are equal to 255
 */
static int calculate_signature(struct rawfingerprint* fp, struct signature* signature) {
    int meaningful_signature = 0;
    for (unsigned int i = 0 ; i < SIGNATURE_LENGTH ; i++) {
        uint16_t* permutation = get_permutation(i);
        signature->minhash[i] = PERMUTATION_LENGTH;
        for (unsigned int j = 0 ; j < PERMUTATION_LENGTH ; j++) {
            uint16_t bit_index = permutation[j];
            if (fp->bit_array[bit_index / 8] & (1 << (bit_index % 8))) {
                meaningful_signature = 1;
                signature->minhash[i] = j;
                break;
            }
        }
    }
    return meaningful_signature;
}


struct signatures* build_signatures(struct rawfingerprints* rawfingerprints) {
    // Let's allocate as many signatures as there are fingerprints
    // We will just not fill up all the array in case of degenerate signatures
    struct signatures* signatures = (struct signatures*)malloc(sizeof(struct signatures));
    if (signatures == NULL) {
        return NULL;
    }

    signatures->signatures = (struct signature*)malloc(rawfingerprints->size * sizeof(struct signature));
    if (signatures->signatures == NULL) {
        free(signatures);
        return NULL;
    }

    signatures->n_signatures = 0;
    for (unsigned int i = 0 ; i < rawfingerprints->size ; i++) {
        if (!rawfingerprints->fingerprints[i].is_silence && calculate_signature(&(rawfingerprints->fingerprints[i]), &(signatures->signatures[signatures->n_signatures]))) {
            // Let's increase the counter if we have actually calculated a signature
            (signatures->n_signatures)++;
        }
    }

    return signatures;
}


void free_signatures(struct signatures* signatures) {
    free(signatures->signatures);
    free(signatures);
}
