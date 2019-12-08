#ifndef _LOGBINS_H
#define _LOGBINS_H

// How many bins do we produce for one frame
#define NUMBER_OF_BINS 32

/**
 * The Fourier transform produces for each frame of 2048 samples at
 * 5512Hz a decomposition in 1024 frequencies from 2.7Hz to 2756Hz.
 * This is still too much information for fingerprinting so we need
 * to reduce the amount of data for each frame.
 *
 * Since we want the fingerprint to mimic what humans feel similar,
 * the first thing is to ignore frequencies that are either too low
 * or too high to be perceived in a significant way by humans.
 * According to researchers, retaining the range between 318Hz and
 * 2000Hz is sufficient.
 *
 * The second thing we will do is to group frequencies into 32 bins.
 * However, we perceive sound in a logarithmic fashion rather than
 * a linear one. Let's take for instance the music tone A 440 that
 * corresponds to the 440Hz frequency. In order to get the A tone
 * that is one octave higher, you need to double the frequency and
 * in order to go one octave lower, you need to divide the frequency
 * by 2. Here is what it looks like on a piano keyboard:
 *
 *                  A 220                A 440                A 880
 *                    v                    v                    v
 *  -+--#--#--+--#--#--#--+--#--#--+--#--#--#--+--#--#--+--#--#--#--+-
 *   |  #  #  |  #  #  #  |  #  #  |  #  #  #  |  #  #  |  #  #  #  |
 *   |  #  #  |  #  #  #  |  #  #  |  #  #  #  |  #  #  |  #  #  #  |
 *   |  #  #  |  #  #  #  |  #  #  |  #  #  #  |  #  #  |  #  #  #  |
 *   |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 *   |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 *   |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 *  -+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+-
 *
 * As a consequence, the 32 bins we will use are not obtained by
 * dividing the frequencies between 318Hz and 2000Hz in a linear fashion
 * (which would be having an interval size of 52 = (2000-318)/2 and then
 * having the intervals [318, 318+52], [318+52, 318+2*52], ...). Instead,
 * the frequency spectrum from 318Hz to 2000Hz is divided using a logarithmic
 * scale which means that the logarithmic size delta of an interval will be:
 *
 * min = log2(318)
 * max = log2(2000)
 * delta = (max - min) / 32
 *
 * and that the intervals will then be:
 *
 * [2^min, 2^(min+delta)], [2^(min+delta)), 2^(min+2*delta)], ... , [2^(max-delta)), 2^max]
 *
 *
 * Given the results of the Fourier transform applied to 2048 samples,
 * this function calcules the 32 bins.
 *
 * @param real The 2048 real values produced by the FFT
 * @param imaginary The 2048 imaginary values produced by the FFT
 * @param bins Where to store the results. The array is supposed
 *                    large enough to hold 32 float values
 */
void calculate_bins(float* real, float* imaginary, float* bins);


#endif
