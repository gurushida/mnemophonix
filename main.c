#include <stdio.h>
#include <stdlib.h>
#include "fingerprinting.h"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        fprintf(stderr, "Usage: %s <wav>\n", argv[0]);
        return 1;
    }
    const char* wav = argv[1];

    struct signatures* fingerprint;
    int res = generate_fingerprint(wav, &fingerprint);
    if (res != SUCCESS) {
        switch (res) {
            case CANNOT_READ_FILE: fprintf(stderr, "Cannot read file '%s'\n", wav); return 1;
            case MEMORY_ERROR: fprintf(stderr, "Memory allocation error\n"); return 1;
            case DECODING_ERROR: fprintf(stderr, "Cannot decode file '%s'\n", wav); return 1;
            case UNSUPPORTED_WAVE_FORMAT: fprintf(stderr, "Unsupported wave format used in '%s'\n", wav); return 1;
            case FILE_TOO_SMALL: fprintf(stderr, "'%s' is too small to generate a fingerprint\n", wav); return 1;
        }
    }

    return 0;
}
