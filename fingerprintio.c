#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fingerprintio.h"


static void free_index_entry(struct index_entry* entry);


void save(FILE* f, struct signatures* fingerprint, const char* wavname,
            const char* artist, const char* track_title, const char* album_title) {
    fprintf(f, "%s\n", wavname);
    fprintf(f, "%s\n", artist != NULL ? artist : "");
    fprintf(f, "%s\n", track_title != NULL ? track_title : "");
    fprintf(f, "%s\n", album_title != NULL ? album_title : "");
    fprintf(f, "%d\n", fingerprint->n_signatures);
    for (unsigned int i = 0 ; i < fingerprint->n_signatures ; i++) {
        for (unsigned int j = 0 ; j < SIGNATURE_LENGTH ; j++) {
            fprintf(f, "%02x", fingerprint->signatures[i].minhash[j]);
        }
        fprintf(f,"\n");
    }
}


/**
 * Reads a line from the given file and puts it, without the \n
 * in the given buffer. If dst is not NULL, the content of the
 * buffer is strduped into it.
 *
 * @param f The file to read from
 * @param buffer The buffer to fill
 * @param size The size of the buffer
 * @param dst If not NULL, where to strdup the buffer
 * @return SUCCESS on success
 *         CANNOT_READ_FILE if we have reached the end of file
 *         MEMORY_ERROR in case of memory allocation error
 *         DECODING_ERROR if the file does not end with \n (either because
 *                        there is a line that is too long or because we have reached
 *                        the end of the file)
 */
static int readline(FILE* f, char* buffer, unsigned int size, char* *dst) {
    if (NULL == fgets(buffer, size, f)) {
        return CANNOT_READ_FILE;
    }

    int len = (int)strlen(buffer);
    if (len == 0 || buffer[len - 1] != '\n') {
        return DECODING_ERROR;
    }
    buffer[len - 1] = '\0';

    if (dst != NULL) {
        (*dst) = strdup(buffer);
        if ((*dst) == NULL) {
            return MEMORY_ERROR;
        }
    }

    return SUCCESS;
}


/**
 * Reads an index entry from the given file.
 *
 * @param f The file to read from
 * @param entry Where to store the result
 * @return SUCCESS on success
 *         CANNOT_READ_FILE if we have reached the end of file
 *         MEMORY_ERROR in case of memory allocation error
 *         DECODING_ERROR if the file cannot be parsed correctly
 */
static int read_entry(FILE* f, struct index_entry* *entry) {
    char buffer[1024];
    if (CANNOT_READ_FILE == readline(f, buffer, 1024, NULL)) {
        return CANNOT_READ_FILE;
    }

    (*entry) = (struct index_entry*)calloc(1, sizeof(struct index_entry));
    if ((*entry) == NULL) {
        return MEMORY_ERROR;
    }

    (*entry)->filename = strdup(buffer);
    if ((*entry)->filename == NULL) {
        free_index_entry(*entry);
        return MEMORY_ERROR;
    }

    int res = readline(f, buffer, 1024, &((*entry)->artist));
    if (res != SUCCESS) {
        free_index_entry(*entry);
        return res == CANNOT_READ_FILE ? DECODING_ERROR : res;
    }

    res = readline(f, buffer, 1024, &((*entry)->track_title));
    if (res != SUCCESS) {
        free_index_entry(*entry);
        return res == CANNOT_READ_FILE ? DECODING_ERROR : res;
    }

    res = readline(f, buffer, 1024, &((*entry)->album_title));
    if (res != SUCCESS) {
        free_index_entry(*entry);
        return res == CANNOT_READ_FILE ? DECODING_ERROR : res;
    }

    res = readline(f, buffer, 1024, NULL);
    if (res != SUCCESS) {
        free_index_entry(*entry);
        return res == CANNOT_READ_FILE ? DECODING_ERROR : res;
    }
    (*entry)->signatures = (struct signatures*)malloc(sizeof(struct signatures));
    if ((*entry)->signatures == NULL) {
        free_index_entry(*entry);
        return MEMORY_ERROR;
    }
    if (1 != sscanf(buffer, "%u", &((*entry)->signatures->n_signatures))) {
        free_index_entry(*entry);
        return DECODING_ERROR;
    }

    (*entry)->signatures->signatures = (struct signature*)malloc((*entry)->signatures->n_signatures * sizeof(struct signature));
    if ((*entry)->signatures->signatures == NULL) {
        free_index_entry(*entry);
        return MEMORY_ERROR;
    }

    for (unsigned int i = 0 ; i < (*entry)->signatures->n_signatures ; i++) {
        int res = readline(f, buffer, 1024, NULL);
        if (res != SUCCESS) {
            free_index_entry(*entry);
            return res == CANNOT_READ_FILE ? DECODING_ERROR : res;
        }
        if (strlen(buffer) != 200) {
            // We expect 100 values represented each with 2 hexadecimal digits
            free_index_entry(*entry);
            return DECODING_ERROR;
        }
        uint8_t* signature =  &((*entry)->signatures->signatures[i].minhash[0]);
        unsigned int tmp;
        for (unsigned int j = 0 ; j < SIGNATURE_LENGTH ; j++) {
            if (1 != sscanf(&(buffer[2 * j]), "%2x", &tmp)) {
                free_index_entry(*entry);
                return DECODING_ERROR;
            }
            signature[j] = (uint8_t)tmp;
        }
    }

    return SUCCESS;
}


int read_index(const char* filename, struct index* *index) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return CANNOT_READ_FILE;
    }

    (*index) = (struct index*)malloc(sizeof(struct index));
    if ((*index) == NULL) {
        fclose(f);
        return MEMORY_ERROR;
    }

    (*index)->n_entries = 0;
    unsigned int capacity = 1;
    (*index)->entries = (struct index_entry**)malloc(capacity * sizeof(struct index_entry*));
    if ((*index)->entries == NULL) {
        free(*index);
        fclose(f);
        return MEMORY_ERROR;
    }

    struct index_entry* tmp;
    while (1) {
        int res = read_entry(f, &tmp);
        if (res == CANNOT_READ_FILE) {
            break;
        }

        if (res != SUCCESS) {
            free_index(*index);
            fclose(f);
            return res;
        }

        if (capacity == (*index)->n_entries) {
            // If the array is full, it's time to reallocate
            capacity = 2 * capacity;
            struct index_entry** new_array = (struct index_entry**)realloc((*index)->entries, capacity * sizeof(struct index_entry*));
            if (new_array == NULL) {
                free_index(*index);
                fclose(f);
                return MEMORY_ERROR;
            }
            (*index)->entries = new_array;
        }
        (*index)->entries[((*index)->n_entries)++] = tmp;
    }

    fclose(f);
    return SUCCESS;
}


static void free_index_entry(struct index_entry* entry) {
    free(entry->filename);
    free(entry->artist);
    free(entry->track_title);
    free(entry->album_title);
    if (entry->signatures) {
        free_signatures(entry->signatures);
    }
    free(entry);
}


void free_index(struct index* index) {
    for (unsigned int i = 0 ; i < index->n_entries ; i++) {
        free_index_entry(index->entries[i]);
    }
    free(index->entries);
    free(index);
}
