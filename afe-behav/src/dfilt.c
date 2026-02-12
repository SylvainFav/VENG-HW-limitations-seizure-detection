
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"
#include "../include/dfilt.h"


int dfiltModule(int** in, int* out) {

    // Convert input to single int value
    int* in_signed = malloc(ADC_NSAMPLES * sizeof(int));
    for (int i=0; i<ADC_NSAMPLES; i++) {
        in_signed[i] = - (1 << (ADC_NBITS-1));
        for (int n=0; n<ADC_NBITS; n++) {
            in_signed[i] += in[n][i] * (1 << (ADC_NBITS - 1 - n));
        }
        in_signed[i] = in_signed[i] << (OUT_NBITS - ADC_NBITS);
    }

    // HPF
    float hpf_alpha[3] = {DFILT_HPF_ALPHA_0, DFILT_HPF_ALPHA_1, DFILT_HPF_ALPHA_2};
    float hpf_beta[3] = {DFILT_HPF_BETA_0, DFILT_HPF_BETA_1, DFILT_HPF_BETA_2};
    int* hpf_out = malloc(ADC_NSAMPLES * sizeof(int));
    iir_order_2_int(in_signed, hpf_out, ADC_NSAMPLES, hpf_alpha, hpf_beta);

    // LPF
    float lpf_alpha[3] = {DFILT_LPF_ALPHA_0, DFILT_LPF_ALPHA_1, DFILT_LPF_ALPHA_2};
    float lpf_beta[3] = {DFILT_LPF_BETA_0, DFILT_LPF_BETA_1, DFILT_LPF_BETA_2};
    iir_order_2_int(hpf_out, out, ADC_NSAMPLES, lpf_alpha, lpf_beta);

    free(in_signed);
    free(hpf_out);

    return 0;
}

