#ifndef _HAARWAVELETS_H
#define _HAARWAVELETS_H

#include "spectralimages.h"


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
 * The function applies the Haar transform to the given spectral images to make
 * it suitable for further compression. Since the Haar transform produces
 * for each image an output of the same size, this function will modify the
 * given data in place.
 */
void apply_Haar_transform(struct spectral_images* images);


#endif
