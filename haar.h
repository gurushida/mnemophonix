#ifndef _HAARWAVELETS_H
#define _HAARWAVELETS_H

#include "frames.h"


// How many frames do we use to make a spectral image
// that will be transformed with Haar wavelets. It
// is no accident that this is a power of 2 as it makes
// it easier to implement the Haar transform
#define SPECTRAL_IMAGE_WIDTH 128


/**
 * The standard Haar transformation is a mechanism that
 * allows to represent data as a succession of operations
 * to apply to a raw initial value to get better and better
 * approximations of the original data. For instance, if you
 * take an image and pixelize it over and over again until you
 * have only one pixel left, then the Haar transform consists
 * of having the initial single pixel and all the steps needed
 * to convert the pixel into a tiny pixelized image, then into
 * a slightly bigger and less pixelized image and so on until
 * you have the original image.
 *
 * What makes this transform valuable for audio fingerprinting
 * is that it provides a way to identify "unimportant details"
 * that can be simplified to compress the original data (like
 * getting a smaller image by using the same color for parts that
 * are very similar).
 *
 * Given n frames, this function divide them into groups
 * that represent spectral images of the audio input:
 *
 * spectrogram bins
 *
 * 32 +----------+----------+--      --+----------+----------+------
 *    | spectral | spectral |          | spectral | spectral |
 *    | image 0  | image 1  |    ...   | image    | image N  |
 *    |          |          |          |   N-1    |          |
 *  0 +----------+----------+---     --+----------+----------+------
 *    0          128        256        (N-1)*128  N*128         n frames
 *
 *
 * The function applies the Haar transform to each spectral image to make
 * it suitable for further compression. Since the Haar transform produces
 * for each image an output of the same size, this function will modify the
 * given data in place. If the number of frames is not divisible by 128,
 * the remaining frames at the end are left unmodified.
 */
void apply_Haar_transform(struct frames* frames);


#endif
