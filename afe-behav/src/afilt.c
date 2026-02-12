
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"
#include "../include/afilt.h"

int afiltModule(float* in, float* outp, float* outn) {

    float* v_gain = malloc(N_SAMPLES * sizeof(float));
    // Gain
    for (int i=0; i<N_SAMPLES; i++) {
        v_gain[i] = AFILT_GAIN * in[i];
    }

    // HPF
    float* v_hpf = malloc(N_SAMPLES * sizeof(float));
    float hpf_alpha[3] = {AFILT_HPF_ALPHA_0, AFILT_HPF_ALPHA_1, AFILT_HPF_ALPHA_2};
    float hpf_beta[3] = {AFILT_HPF_BETA_0, AFILT_HPF_BETA_1, AFILT_HPF_BETA_2};
    iir_order_2(v_gain, v_hpf, N_SAMPLES, hpf_alpha, hpf_beta);
    free(v_gain);

    // LPF
    float* v_lpf = malloc(N_SAMPLES * sizeof(float));
    float lpf_alpha[3] = {AFILT_LPF_ALPHA_0, AFILT_LPF_ALPHA_1, AFILT_LPF_ALPHA_2};
    float lpf_beta[3] = {AFILT_LPF_BETA_0, AFILT_LPF_BETA_1, AFILT_LPF_BETA_2};
    iir_order_2(v_hpf, v_lpf, N_SAMPLES, lpf_alpha, lpf_beta);
    free(v_hpf);

    // Saturation
    float* v_sat = malloc(N_SAMPLES * sizeof(float));
    #ifdef SATURATE
        float dr_max_pi2 = HALF_PI * AFILT_DR_MAX;
        for (int i=0; i<N_SAMPLES; i++) {
            if (v_lpf[i] > dr_max_pi2) {
                v_sat[i] = AFILT_DR_MAX;
            } else if (v_lpf[i] < -dr_max_pi2) {
                v_sat[i] = -AFILT_DR_MAX;
            } else {
                v_sat[i] = AFILT_DR_MAX * sinf(v_lpf[i] / AFILT_DR_MAX);
            }
        }
    #else
        for (int i=0; i<N_SAMPLES; i++) {
            v_sat[i] = v_lpf[i];
        }
    #endif // SATURATE
    free(v_lpf);

    // DC value
    for (int i=0; i<N_SAMPLES; i++) {
        outp[i] = AFILT_DC_OUT + v_sat[i]/2;
        outn[i] = AFILT_DC_OUT - v_sat[i]/2;
    }
    free(v_sat);
    
    return 0;

}
