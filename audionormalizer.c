#include <math.h>
#include "audionormalizer.h"


void normalize(float* samples, unsigned int size) {
    float square_sum = 0;
    for (unsigned int i = 0 ; i < size ; i++) {
        square_sum += samples[i] * samples[i];
    }

    // The 10.0 coefficient, the 0.1 minimum and 3.0 maximum
    // are taken from the original implementation from
    // https://github.com/AddictedCS/soundfingerprinting/blob/develop/src/SoundFingerprinting/Audio/AudioSamplesNormalizer.cs
    // I have to admit they are magic values that I don't understand
    float rms = sqrtf(square_sum / size) * 10.0;
    if (rms < 0.1) {
        rms = 0.1;
    } else if (rms > 3.0) {
        rms = 3.0;
    }

    for (unsigned int i = 0 ; i < size ; i++) {
        float value = samples[i] / rms;
        if (value < -1.0) {
            samples[i] = -1.0;
        } else if (value > 1.0) {
            samples[i] = 1.0;
        } else {
            samples[i] = value;
        }
    }
}
