
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"
#include "../include/stimuli.h"

int stimuliModule(float* in1d, float* in2d, float* in1c, float* in2c, int buffer_idx, char* subject) {

    // Define file names
    int filename_max_size = 100;
    char* filename1 = (char*)malloc(filename_max_size * sizeof(char));
    snprintf(filename1, filename_max_size, "%s%s/buffer1_%d.bin", VENG_DATA_FOLDER, subject, buffer_idx+1);
    char* filename2 = (char*)malloc(filename_max_size * sizeof(char));
    snprintf(filename2, filename_max_size, "%s%s/buffer2_%d.bin", VENG_DATA_FOLDER, subject, buffer_idx+1);
    
    // Read files
    int file_read_ctrl = 0;
    file_read_ctrl += read_input_ffile(filename1, in1d);
    file_read_ctrl += read_input_ffile(filename2, in2d);
    if (file_read_ctrl > 0) {
        return 1;
    }

    // Reverse polarity on channel 1
    for (int i=0; i<N_SAMPLES; i++) {
        in1d[i] = - in1d[i];
    }


    // CM signal is generated as 1/f noise
    if (INPUT_CM > 0) {
        float cm_band[2] = {INPUT_CM_FMIN, INPUT_CM_FMAX};
        float cm_power = INPUT_CM * INPUT_CM;
        mixed_noise_generator_nsamples(in1c, cm_power, INPUT_CM_FMAX, cm_band);
        mixed_noise_generator_nsamples(in2c, cm_power, INPUT_CM_FMAX, cm_band);
    } else {
        for (int i=0; i<N_SAMPLES; i++) {
            in1c[i] = 0.0f;
            in2c[i] = 0.0f;
        }   
    }


    free(filename1);
    free(filename2);

    return 0;
}

int read_input_ffile(char* filename, float* signal) {

    // Open file
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    // Read as double
    double* signal_double = malloc(INPUT_NSAMPLES * sizeof(double));
    size_t num_elem = fread(signal_double, sizeof(double), N_SAMPLES, file);
    if (num_elem != INPUT_NSAMPLES) {
        fprintf(stderr, "Read input ffile: File length = %d but expecting %d\n", (int)num_elem, INPUT_NSAMPLES);
        fclose(file);
        return 1;
    }

    // Convert to float and over-sample
    double signal_previous_value = 0.0;
    double signal_current_value = 0.0;
    double signal_dv;
    for (int i=0; i<INPUT_NSAMPLES; i++) {
        signal_previous_value = signal_current_value;
        signal_current_value = signal_double[i];
        signal_dv = (signal_current_value - signal_previous_value) / INPUT_FS_RATIO;
        for (int k=0; k<INPUT_FS_RATIO; k++) {
            signal[i*INPUT_FS_RATIO+k] = (float)(signal_previous_value + signal_dv * (k+1));
        }
    }

    fclose(file);
    free(signal_double);

    return 0;
}