
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/decim.h"


int decimModule(int* in, int* out) {

    int sum_in = 0;
    int adc_osr = pow(2, ADC_OSR_LOG);
    for (int i=0; i<OUT_NSAMPLES; i++) {
        sum_in = 0;
        for (int j=0; j<adc_osr; j++) {
            sum_in += in[(i << ADC_OSR_LOG) + j];
        }
        out[i] = sum_in >> ADC_OSR_LOG;
    }

    return 0;

}