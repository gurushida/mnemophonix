#include <math.h>
#include "hannwindow.h"


static float window[SAMPLES_PER_FRAME] = { 666 };


static void initialize() {
    for (unsigned int i = 0 ; i < SAMPLES_PER_FRAME ; i++) {
        window[i] = (float)(0.5 * (1 - cosf(2 * M_PI * i / (SAMPLES_PER_FRAME - 1))));
    }
}


float* get_Hann_window() {
    if (window[0] > 600) {
        initialize();
    }

    return window;
}
