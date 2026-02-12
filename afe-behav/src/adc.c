
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/adc.h"

int adcModule(float* inp, float* inn, int** out) {

    float inpval, innval;
    int rounded_val;
    for (int i=0; i<ADC_NSAMPLES; i++) {

        inpval = inp[i * ADC_FREQUENCY_RATIO];
        innval = inn[i * ADC_FREQUENCY_RATIO];
        
        // Clip inputs
        if (inpval > ADC_VMAX) {
            inpval = ADC_VMAX;
        } else if (inpval < ADC_VMIN) {
            inpval = ADC_VMIN;
        }
        if (innval > ADC_VMAX) {
            innval = ADC_VMAX;
        } else if (innval < ADC_VMIN) {
            innval = ADC_VMIN;
        }

        // Quantization
        rounded_val = (int)roundf(((inpval - innval) / (ADC_FULLSCALE/2) + 1)/2 * ADC_INTMAX);
        for (int n=0; n<ADC_NBITS; n++) {
            out[ADC_NBITS-1-n][i] = (rounded_val >> n) & 1;
        }

    }

    return 0;
}
