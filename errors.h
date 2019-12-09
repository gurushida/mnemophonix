#ifndef _ERRORS_H
#define _ERRORS_H


#define SUCCESS 0

// Returned when a memory allocation error has occurred
#define MEMORY_ERROR -1

// Returned when a file cannot be file
#define CANNOT_READ_FILE -2

// Returned when an operation fails because the input data
// cannot be decoded as expected
#define DECODING_ERROR -3

// Returned when trying to read a wave file other than uncompressed
// 16-bit PCM
#define UNSUPPORTED_WAVE_FORMAT -4

// Returned when the input wave file is too small to be able to generate
// a fingerprint
#define FILE_TOO_SMALL -5

// Returned when an input file does not look like a wave file at all
#define NOT_A_WAVE_FILE -6

// Returned when a search operation cannot find any match
#define NO_MATCH_FOUND -7

#endif
