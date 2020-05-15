#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ffmpeg.h"


static void parse_metadata(char* metadata, char* *artist, char* *track_title, char* *album_title) {
    FILE* f = fopen(metadata, "r");
    if (f == NULL) {
        return;
    }

    char buffer[1024];
    while (NULL != fgets(buffer, 1024, f)) {
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        if ((*artist) == NULL && buffer == strstr(buffer, "artist=")) {
            *artist = strdup(buffer + strlen("artist="));
        }
        if ((*track_title) == NULL && buffer == strstr(buffer, "title=")) {
            *track_title = strdup(buffer + strlen("title="));
        }
        if ((*album_title) == NULL && buffer == strstr(buffer, "album=")) {
            *album_title = strdup(buffer + strlen("album="));
        }
    }
    fclose(f);
}


char* generate_wave_file(char* input, char* *artist, char* *track_title, char* *album_title) {
    char wav[] = "tmp-mnemophonix-wav-XXXXXX";
    mktemp(wav);
    char metadata[] = "tmp-mnemophonix-metadata-XXXXXX";
    mktemp(metadata);

    *artist = NULL;
    *track_title = NULL;
    *album_title = NULL;

    int pid;
    switch(pid = fork()) {
        case -1: return NULL;
        case 0: {
            // Child process
            execlp("ffmpeg", "ffmpeg", "-i", input, "-acodec", "pcm_s16le", "-ar", "44100", "-f", "wav", wav, "-f", "ffmetadata", metadata, NULL);
            return NULL;
        }
        default: {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            if (!WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
                return NULL;
            }
            parse_metadata(metadata, artist, track_title, album_title);
            remove(metadata);
        }
    }
    return strdup(wav);
}
