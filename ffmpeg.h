#ifndef _FFMPEG_H
#define _FFMPEG_H


/**
 * Tries to convert the input file to a 44100Hz wave file,
 * extracting if possible metadata about artist, track and album.
 *
 * @return On success, the name of the generated wave file. The caller is
 *         responsible for deleting the file and freeing the name pointer
 *         after use; NULL if the generation failed
 */
char* generate_wave_file(char* input, char* *artist, char* *track_title, char* *album_title);

#endif
