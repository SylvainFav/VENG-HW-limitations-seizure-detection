
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"
#include "../include/pcb.h"


int pcbModule(float* in1d, float* in2d, float* in1c, float* in2c, float* out1d, float* out2d, float* out1c, float* out2c) {

    float alpha[2] = {PCB_ALPHA_0, PCB_ALPHA_1};
    float beta[2] = {PCB_BETA_0, PCB_BETA_1};

    iir_order_1(in1d, out1d, N_SAMPLES, alpha, beta);
    iir_order_1(in2d, out2d, N_SAMPLES, alpha, beta);
    iir_order_1(in1c, out1c, N_SAMPLES, alpha, beta);
    iir_order_1(in2c, out2c, N_SAMPLES, alpha, beta);

    return 0;

}