#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "ffmpeg.h"
#include "fingerprinting.h"
#include "fingerprintio.h"
#include "lsh.h"
#include "search.h"

static long time_in_milliseconds() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


int main(int argc, char* argv[]) {
    if (argc < 2
        || (strcmp(argv[1], "index") && strcmp(argv[1], "search"))
        || (!strcmp(argv[1], "search") && argc != 4)) {

        fprintf(stderr, "\n");
        fprintf(stderr, "--- mnemophonix - An audio fingerprinting system ---\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "%s index <input>\n", argv[0]);
        fprintf(stderr, "  Prints to stdout the index data generated for the given input file\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "%s search <input> <index>\n", argv[0]);
        fprintf(stderr, "  Look for the given input file in the given index file\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "The input file format is 44100Hz 16-bit PCM. If it is not the case,\n");
        fprintf(stderr, "an attempt will be made to generate such a file using ffmpeg.\n");
        fprintf(stderr, "\n");
        return 1;
    }
    char* input = argv[2];

    struct signatures* fingerprint;
    char* artist;
    char* track_title;
    char* album_title;

    int res = generate_fingerprint(input, &fingerprint, &artist, &track_title, &album_title);
    if (res != SUCCESS) {
        switch (res) {
            case CANNOT_READ_FILE: fprintf(stderr, "Cannot read file '%s'\n", input); return 1;
            case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
            case DECODING_ERROR: fprintf(stderr, "Cannot decode file '%s'\n", input); return 1;
            case FILE_TOO_SMALL: fprintf(stderr, "'%s' is too small to generate a fingerprint\n", input); return 1;
            case UNSUPPORTED_WAVE_FORMAT:
            case NOT_A_WAVE_FILE: {
                // Not a wave file ? Let's try to convert it to a wave file
                // with ffmpeg
                char* generated_wav = generate_wave_file(input, &artist, &track_title, &album_title);
                if (generated_wav == NULL) {
                    fprintf(stderr, "'%s' is not a wave file and we could not convert it to one with fffmpeg\n", input);
                    return 1;
                }
                res = generate_fingerprint(generated_wav, &fingerprint, NULL, NULL, NULL);
                remove(generated_wav);
                free(generated_wav);
                switch (res) {
                case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
                case FILE_TOO_SMALL: fprintf(stderr, "'%s' is too small to generate a fingerprint\n", input); return 1;
                }
                break;
            }
        }
    }

    int ret_value = 0;

    if (!strcmp(argv[1], "index")) {
        save(stdout, fingerprint, input, artist, track_title, album_title);
    } else {
        const char* index = argv[3];
        struct index* database_index;
        printf("Loading database %s...\n", index);
        long before_loading_db = time_in_milliseconds();
        int res = read_index(index, &database_index);
        if (res != SUCCESS) {
            switch (res) {
                case CANNOT_READ_FILE: fprintf(stderr, "Cannot read file '%s'\n", index); return 1;
                case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
            }
        }
        long after_loading_db = time_in_milliseconds();
        printf("(raw database loading took %ld ms)\n", after_loading_db - before_loading_db);

        struct lsh* lsh = create_hash_tables(database_index);
        long after_lsh = time_in_milliseconds();
        printf("(lsh index building took %ld ms)\n", after_lsh - after_loading_db);

        printf("Searching...\n");

        int best_match = search(fingerprint, database_index, lsh);
        long after_search = time_in_milliseconds();
        printf("(Search took %ld ms)\n", after_search - after_lsh);

        if (best_match == NO_MATCH_FOUND) {
            printf("\nNo match found\n\n");
            ret_value  = 1;
        } else {
            printf("\nFound match: '%s'\n", database_index->entries[best_match]->filename);
            if (database_index->entries[best_match]->artist[0]) {
                printf("Artist: %s\n", database_index->entries[best_match]->artist);
            }
            if (database_index->entries[best_match]->track_title[0]) {
                printf("Track title: %s\n", database_index->entries[best_match]->track_title);
            }
            if (database_index->entries[best_match]->album_title[0]) {
                printf("Album title: %s\n", database_index->entries[best_match]->album_title);
            }
            printf("\n");
        }

        // Since we are done, the process will be terminated so there is no point
        // in cleaning up memory as all the pages will be recycled by the OS.
    }

    return ret_value;
}
