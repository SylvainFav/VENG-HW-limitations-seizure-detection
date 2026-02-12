
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/setup.h"
#include "../include/signal.h"


int signal_init(algo_t* algo) {
    algo->signal = (float*) malloc(BUFFER_SIZE * sizeof(float));
    if (algo->signal == NULL) {
        fprintf(stderr, "Memory allocation failed for algo->signal\n");
        return 1;
    }
    return 0;
}


int signal_free(algo_t* algo){
    if (algo->signal != NULL) {
        free(algo->signal);
        algo->signal = NULL;
    }
    return 0;
}


int read_buffer_file(algo_t* algo) {

    // Define file name
    int filename_max_size = 100;
    char* filename = (char*) malloc(filename_max_size * sizeof(char));
    snprintf(filename, filename_max_size, "%s%s/ap_in/%s/buffer%d.bin", RUN_FOLDER, RUN_CATEGORY, algo->subject, algo->buffer_idx+1);


    // Open file
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    // Read signal as double
    double* tmp_signal = (double*) malloc(BUFFER_SIZE * sizeof(double));
    size_t num_elem = fread(tmp_signal, sizeof(double), BUFFER_SIZE, file);
    if (num_elem != BUFFER_SIZE) {
        fprintf(stderr, "Unexpected file length\nGot %d expected %d\n", (int)num_elem, BUFFER_SIZE);
        fclose(file);
        return 1;
    }

    // Convert signal to float
    for (int i=0; i<BUFFER_SIZE; i++) {
        algo->signal[i] = (float) tmp_signal[i];
    }

    // Compute RMS value
    float sum_sq = 0;
    for (int i=0; i<BUFFER_SIZE; i++) {
        sum_sq += algo->signal[i] * algo->signal[i];
    }
    algo->sigRMS = (float) sqrt(sum_sq / BUFFER_SIZE);

    #ifdef DO_PRINT
        printf("Read signal from file %s. RMS value: %.3f\n", filename, algo->sigRMS);
    #endif

    // Close file and free memory
    fclose(file);
    free(filename);
    filename = NULL;
    free(tmp_signal);

    return 0;

}





