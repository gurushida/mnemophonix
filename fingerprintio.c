#include "fingerprintio.h"

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
