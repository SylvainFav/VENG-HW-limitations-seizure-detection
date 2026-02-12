
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"

int iir_order_1(float* sig_in, float* sig_out, int size, float* alpha, float* beta){

    float x0 = 0.0f, x1 = 0.0f;
    float y0 = 0.0f, y1 = 0.0f;

    float a1 = alpha[1] / alpha[0];
    float b0 = beta[0] / alpha[0];
    float b1 = beta[1] / alpha[0];

    for (int i=0; i < size; i++) {
        x1 = x0;
        y1 = y0;
        x0 = sig_in[i];
        y0 = b1 * x1 + b0 * x0 - a1 * y1;
        sig_out[i] = y0; 
    }

    return 0;
}

int iir_order_2(float* sig_in, float* sig_out, int size, float* alpha, float* beta){

    float x0 = 0.0f, x1 = 0.0f, x2 = 0.0f;
    float y0 = 0.0f, y1 = 0.0f, y2 = 0.0f;

    float a1 = alpha[1] / alpha[0];
    float a2 = alpha[2] / alpha[0];
    float b0 = beta[0] / alpha[0];
    float b1 = beta[1] / alpha[0];
    float b2 = beta[2] / alpha[0];

    for (int i=0; i < size; i++) {
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        x0 = sig_in[i];
        y0 = b2 * x2 + b1 * x1 + b0 * x0 - a2 * y2 - a1 * y1;
        sig_out[i] = y0; 
    }

    return 0;
}


int iir_order_2_int(int* sig_in, int* sig_out, int size, float* alpha, float* beta){

    float x0 = 0.0f, x1 = 0.0f, x2 = 0.0f;
    float y0 = 0.0f, y1 = 0.0f, y2 = 0.0f;

    float a1 = alpha[1] / alpha[0];
    float a2 = alpha[2] / alpha[0];
    float b0 = beta[0] / alpha[0];
    float b1 = beta[1] / alpha[0];
    float b2 = beta[2] / alpha[0];

    for (int i=0; i < size; i++) {
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        x0 = (float)sig_in[i];
        y0 = b2 * x2 + b1 * x1 + b0 * x0 - a2 * y2 - a1 * y1;
        sig_out[i] = (int)roundf(y0); 
    }

    return 0;
}


float white_noise_sample_generator(float scale) {

    float u1 = ((float) rand() + 1.0f) / ((float) RAND_MAX + 2.0f);
    float u2 = ((float) rand()) / ((float) RAND_MAX);
    return scale * sqrtf(-2.0f * logf(u1)) * cosf(TWO_PI * u2);

}

int white_noise_generator(float* white_noise, int size, float power, float* power_band) {

    // Box-Muller method

    float scale;

    if (power_band != NULL) {
        float band = power_band[1] - power_band[0];
        scale = sqrtf(power * (((float) FS) * (0.5 - 1/size)) / band);
    } else {
        scale = sqrtf(power);
    }

    float u1;
    float u2;
    float u1_mult = 1 / ((float) RAND_MAX + 2.0f);
    float u2_mult = 1 / ((float) RAND_MAX);
    for (int i=0; i < size; i++) {
        u1 = ((float) rand() + 1.0f) * u1_mult;
        u2 = ((float) rand()) * u2_mult;
        white_noise[i] = scale * sqrtf(-2.0f * logf(u1)) * cosf(TWO_PI * u2);
    }

    return 0;

}

int pink_noise_generator(float* pink_noise, int size, float power, float* power_band) {

    // Voss - McCartney algorithm

    // Generate white noise from all sources
    float white_noise_power;
    float white_noise_power_sqrt;
    if (power_band != NULL) {
        float fratio = power_band[1] / power_band[0];
        white_noise_power = power * logf(size / 2.0f) / logf(fratio) * PINK_NOISE_NSOURCES;
    } else {
        white_noise_power = power * PINK_NOISE_NSOURCES;
    }
    white_noise_power_sqrt = sqrtf(white_noise_power);
    float white_noise[PINK_NOISE_NSOURCES];
    white_noise_generator(white_noise, PINK_NOISE_NSOURCES, white_noise_power, NULL);
    float running_sum = sumf(white_noise, PINK_NOISE_NSOURCES);

    // Algorithm
    int key = 0;
    int prev_key;
    int diff_key;
    int max_key = (1U << PINK_NOISE_NSOURCES) - 1;
    float new_val;
    for (int i=0; i < size; i++) {
        prev_key = key;
        key++;
        if (key > max_key) {
            key = 0;
        }
        diff_key = prev_key ^ key;

        for (int j=0; j < PINK_NOISE_NSOURCES; j++) {
            if (diff_key & (1U << j)) {
                new_val = white_noise_sample_generator(white_noise_power_sqrt);
                running_sum += new_val - white_noise[j];
                white_noise[j] = new_val;
            }
        }

        pink_noise[i] = running_sum / PINK_NOISE_NSOURCES;

    }

    return 0;
}

int mixed_noise_generator_nsamples(float* noise, float total_power, float fcorner, float* power_band) {

    float pink_noise_factor;
    float white_noise_factor;
    if (power_band != NULL) {
        pink_noise_factor = fcorner * logf(power_band[1] / power_band[0]);
        white_noise_factor = power_band[1] - power_band[0];
    } else {
        pink_noise_factor = fcorner * logf(N_SAMPLES/2);
        white_noise_factor = ((float) FS) * (0.5 - 1/N_SAMPLES);
    }
    float white_psd = total_power / (white_noise_factor + pink_noise_factor);
    float white_noise_power = white_psd * white_noise_factor;
    float pink_noise_power = white_psd * pink_noise_factor;

    float white_noise[N_SAMPLES];
    float pink_noise[N_SAMPLES];
    white_noise_generator(white_noise, N_SAMPLES, white_noise_power, power_band);
    pink_noise_generator(pink_noise, N_SAMPLES, pink_noise_power, power_band);

    for (int i=0; i < N_SAMPLES; i++) {
        noise[i] = white_noise[i] + pink_noise[i];
    }

    return 0;
}


float sumf(float* array, int size) {
    
    float sum = 0.0;
    for (int i=0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

float meanf(float* array, int size) {
    
    float sum = 0.0;
    for (int i=0; i < size; i++) {
        sum += array[i];
    }
    return sum / size;
}

float maxf(float* array, int size) {

    float max_val = 0.0f;
    for (int i=0; i<size; i++) {
        if (array[i] > max_val) {
            max_val = array[i];
        }
    }
    return max_val;
}

float minf(float* array, int size) {

    float min_val = 0.0f;
    for (int i=0; i<size; i++) {
        if (array[i] < min_val) {
            min_val = array[i];
        }
    }
    return min_val;
}

float rmsf(float* array, int size) {

    float sum = 0.0;
    for (int i=0; i < size; i++) {
        sum += array[i] * array[i];
    }

    return sqrtf(sum / size);

}

float rmsf_nomean(float* array, int size) {

    float mean = meanf(array, size);

    float sum = 0.0;
    for (int i=0; i < size; i++) {
        sum += (array[i] - mean) * (array[i] - mean);
    }

    return sqrtf(sum / size);

}

int write_farray_to_file(float* array, int size, char* filename) {

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Write float array to file: Error opening file %s\n", filename);
        return 1;
    }

    for (int i=0; i<size; i++) {
        fprintf(file, "%.10f\n", array[i]);
    }

    fclose(file);

    return 0;
}

int write_intarray_to_file(int* array, int size, char* filename) {

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Write int array to file: Error opening file %s\n", filename);
        return 1;
    }

    for (int i=0; i<size; i++) {
        fprintf(file, "%d\n", array[i]);
    }

    fclose(file);

    return 0;
}

int write_multifarray_to_file(float** array, uint32_t nArrays, uint32_t arraySize, char* filename) {

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Write multi float array to file: Error opening file %s\n", filename);
        return 1;
    }

    for (uint32_t i=0; i<arraySize; i++) {
        for (uint32_t j=0; j<nArrays-1; j++) {
            fprintf(file, "%.10f,", array[j][i]);
        }
        fprintf(file, "%.10f\n", array[nArrays-1][i]);
    }

    fclose(file);

    return 0;
}

int write_multiintarray_to_file(int** array, uint32_t nArrays, uint32_t arraySize, char* filename) {

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Write multi int array to file: Error opening file %s\n", filename);
        return 1;
    }

    for (uint32_t i=0; i<arraySize; i++) {
        for (uint32_t j=0; j<nArrays-1; j++) {
            fprintf(file, "%d,", array[j][i]);
        }
        fprintf(file, "%d\n", array[nArrays-1][i]);
    }

    fclose(file);

    return 0;
}


