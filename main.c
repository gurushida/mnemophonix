#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fingerprinting.h"
#include "fingerprintio.h"
#include "search.h"

int main(int argc, char* argv[]) {
    if (argc < 2
        || (strcmp(argv[1], "index") && strcmp(argv[1], "search"))
        || (!strcmp(argv[1], "search") && argc != 4)) {

        fprintf(stderr, "\n");
        fprintf(stderr, "--- mnemophonix - An audio fingerprinting system ---\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "%s index <wav>\n", argv[0]);
        fprintf(stderr, "  Prints to stdout the index data generated for the given wave file\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "%s search <wav> <index>\n", argv[0]);
        fprintf(stderr, "  Look for the given wave file in the given index\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "\n");
        return 1;
    }
    const char* wav = argv[2];

    struct signatures* fingerprint;
    char* artist;
    char* track_title;
    char* album_title;

    int res = generate_fingerprint(wav, &fingerprint, &artist, &track_title, &album_title);
    if (res != SUCCESS) {
        switch (res) {
            case CANNOT_READ_FILE: fprintf(stderr, "Cannot read file '%s'\n", wav); return 1;
            case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
            case DECODING_ERROR: fprintf(stderr, "Cannot decode file '%s'\n", wav); return 1;
            case UNSUPPORTED_WAVE_FORMAT: fprintf(stderr, "Unsupported wave format used in '%s'\n", wav); return 1;
            case FILE_TOO_SMALL: fprintf(stderr, "'%s' is too small to generate a fingerprint\n", wav); return 1;
        }
    }

    int ret_value = 0;

    if (!strcmp(argv[1], "index")) {
        save(stdout, fingerprint, wav, artist, track_title, album_title);
    } else {
        const char* index = argv[3];
        struct index* database_index;
        int res = read_index(index, &database_index);
        if (res != SUCCESS) {
            switch (res) {
                case CANNOT_READ_FILE: fprintf(stderr, "Cannot read file '%s'\n", index); return 1;
                case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
            }
        }

        int best_match = search(fingerprint, database_index);
        if (best_match == -1) {
            printf("No match found\n");
            ret_value  = 1;
        } else {
            printf("Found match: '%s'\n", database_index->entries[best_match]->filename);
            if (database_index->entries[best_match]->artist) {
                printf("Artist: %s\n", database_index->entries[best_match]->artist);
            }
            if (database_index->entries[best_match]->track_title) {
                printf("Track title: %s\n", database_index->entries[best_match]->track_title);
            }
            if (database_index->entries[best_match]->album_title) {
                printf("Album title: %s\n", database_index->entries[best_match]->album_title);
            }
        }
        free_index(database_index);
    }
    free_signatures(fingerprint);

    return ret_value;
}
