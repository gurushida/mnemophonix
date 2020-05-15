#include <stdlib.h>
#include "lsh.h"


void free_signature_list(struct signature_list* list) {
    struct signature_list* tmp;
    while (list != NULL) {
        tmp = list->next;
        free(list);
        list = tmp;
    }
}


void free_hash_tables(struct lsh* tables) {
    for (unsigned int i = 0 ; i < N_BUCKETS ; i++) {
        for (unsigned int j = 0 ; j < tables->size ; j++) {
            free_signature_list(tables->buckets[i][j]);
        }
        free(tables->buckets[i]);
    }
    free(tables);
}


static unsigned int count_signatures(struct index* database) {
    unsigned int n = 0;
    for (unsigned int i = 0 ; i < database->n_entries ; i++) {
        n += database->entries[i]->signatures->n_signatures;
    }
    return n;
}


struct signature_list* new_signature_list(unsigned int entry_index, unsigned int signature_index, struct signature_list* next) {
    struct signature_list* l = (struct signature_list*)malloc(sizeof(struct signature_list));
    if (l == NULL) {
        return NULL;
    }

    l->entry_index = entry_index;
    l->signature_index = signature_index;
    l->next = next;

    return l;
}


static uint32_t get_minhash(uint8_t* hash, int index) {
    int base = index * BYTES_PER_BUCKET_HASH;
    return (hash[base] << 24) | (hash[base + 1] << 16) | (hash[base + 2] << 8) | hash[base + 3];
}


struct lsh* create_hash_tables(struct index* database) {
    struct lsh* tables = (struct lsh*)calloc(1, sizeof(struct lsh));
    if (tables == NULL) {
        return NULL;
    }
    unsigned int total_signatures = count_signatures(database);
    tables->size = total_signatures / 2;
    for (unsigned int i = 0 ; i < N_BUCKETS ; i++) {
        tables->buckets[i] = (struct signature_list**)calloc(tables->size, sizeof(struct signature_list*));
        if (tables->buckets[i] == NULL) {
            free_hash_tables(tables);
            return NULL;
        }
    }

    for (unsigned int i = 0 ; i < database->n_entries ; i++) {
        for (unsigned int j = 0 ; j < database->entries[i]->signatures->n_signatures ; j++) {
            uint8_t* hash = database->entries[i]->signatures->signatures[j].minhash;
            for (unsigned int k = 0 ; k < N_BUCKETS ; k++) {
                uint32_t index = get_minhash(hash, k) % tables->size;
                struct signature_list* tmp = new_signature_list(i, j, tables->buckets[k][index]);
                if (tmp == NULL) {
                    free_hash_tables(tables);
                    return NULL;
                }
                tables->buckets[k][index] = tmp;
            }
        }
    }

    return tables;
}


int get_matches(struct lsh* tables, uint8_t* hash, struct signature_list* *list) {
    (*list) = NULL;
    int n = 0;

    for (unsigned int i = 0 ; i < N_BUCKETS ; i++) {
        uint32_t index = get_minhash(hash, i) % tables->size;
        struct signature_list* tmp = tables->buckets[i][index];

        // Let's add all these matches to our list
        while (tmp != NULL) {
            struct signature_list* new_item = new_signature_list(tmp->entry_index, tmp->signature_index, *list);
            if (new_item == NULL) {
                free_signature_list(*list);
                return MEMORY_ERROR;
            }
            (*list) = new_item;
            n++;

            tmp = tmp->next;
        }
    }

    return n;
}
