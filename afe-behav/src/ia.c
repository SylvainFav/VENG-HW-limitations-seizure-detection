
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"
#include "../include/ia.h"

int iaModule(float* in1d, float* in2d, float* in1c, float* in2c, float* out) {

    // Generate noise
    #ifdef NOISY
        float enbw[2] = {FL, FH};
        float* v_noise = malloc(N_SAMPLES * sizeof(float));
        mixed_noise_generator_nsamples(v_noise, IA_NOISE * IA_NOISE, IA_FCORNER, enbw);
    #endif // NOISY

    // Compute output
    float* v_intermediate = malloc(N_SAMPLES * sizeof(float));
    #ifdef NOISY
        for (int i=0; i<N_SAMPLES; i++) {
            v_intermediate[i] = IA_GAIN * ((in1d[i] + in2d[i])/2 + (in1c[i] + in2c[i])/2/IA_CMRR + v_noise[i]);
        }
        free(v_noise);
    #else
        for (int i=0; i<N_SAMPLES; i++) {
            v_intermediate[i] = IA_GAIN * ((in1d[i] + in2d[i])/2 + (in1c[i] + in2c[i])/2/IA_CMRR);
        }
    #endif // NOISY

    // Filter output
    float alpha[2] = {IA_ALPHA_0, IA_ALPHA_1};
    float beta[2] = {IA_BETA_0, IA_BETA_1};
    iir_order_1(v_intermediate, out, N_SAMPLES, alpha, beta);
    free(v_intermediate);

    return 0;

}
