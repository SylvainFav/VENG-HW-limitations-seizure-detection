
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../include/setup.h"
#include "../include/utils.h"


int isinarray(uint32_t value, uint32_t* array, uint32_t arraySize) {
    for (uint32_t i=0; i<arraySize; i++) {
        if (array[i] == value) {
            return 1;
        }
    }
    return 0;
}

int isinfarray(float value, float* array, uint32_t arraySize) {
    for (uint32_t i=0; i<arraySize; i++) {
        if (array[i] == value) {
            return 1;
        }
    }
    return 0;
}

float min_farray(float* array, uint32_t arraySize) {
    float min_val = 9e99;
    for (uint32_t i=0; i<arraySize; i++) {
        if (array[i] < min_val) {
            min_val = array[i];
        }
    }
    return min_val;
}

float max_farray(float* array, uint32_t arraySize) {
    float max_val = -9e99;
    for (uint32_t i=0; i<arraySize; i++) {
        if (array[i] > max_val) {
            max_val = array[i];
        }
    }
    return max_val;
}


float sum_farray(float* array, uint32_t arraySize) {
    float sum = 0.0f;
    for (uint32_t i=0; i<arraySize; i++) {
        sum += array[i];
    }
    return sum;
}


float mean_farray(float* array, uint32_t arraySize) {
    return (sum_farray(array, arraySize) / arraySize);
}



float nansum_farray(float* array, uint32_t arraySize) {
    float sum = 0.0f;
    uint32_t nelem_nonnan = 0;
    for (uint32_t i=0; i<arraySize; i++) {
        if (!__isnanf(array[i])) {
            sum += array[i];
            nelem_nonnan++;
        }
    }
    if (nelem_nonnan > 0) {
        return sum;
    }
    else {
        return NAN;
    }
}


float nanmean_farray(float* array, uint32_t arraySize) {
    float sum = 0.0f;
    uint32_t nelem_nonnan = 0;
    for (uint32_t i=0; i<arraySize; i++) {
        if (!__isnanf(array[i])) {
            sum += array[i];
            nelem_nonnan++;
        }
    }

    if (nelem_nonnan > 0) {
        return sum / nelem_nonnan;
    }
    else {
        return NAN;
    }
}


float nanstd_farray(float* array, uint32_t arraySize) {
    
    float mean = nanmean_farray(array, arraySize);
    if (mean == NAN) {
        return NAN;
    }

    float sum_diff = 0.0f;
    uint32_t nelem_nonnan = 0;
    for (uint32_t i=0; i<arraySize; i++) {
        if (!__isnan(array[i])) {
            sum_diff += (array[i] - mean) * (array[i] - mean);
            nelem_nonnan++;
        }
    }

    return sqrt(sum_diff / nelem_nonnan);

}

int write_farray_to_file(float* array, uint32_t arraySize, char* filename) {

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    for (uint32_t i=0; i<arraySize; i++) {
        fprintf(file, "%f\n", array[i]);
    }

    fclose(file);

    return 0;
}

int write_multifarray_to_file(float** array, uint32_t nArrays, uint32_t arraySize, char* filename) {

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    for (uint32_t i=0; i<arraySize; i++) {
        for (uint32_t j=0; j<nArrays-1; j++) {
            fprintf(file, "%.8f,", array[j][i]);
        }
        fprintf(file, "%.8f\n", array[nArrays-1][i]);
    }

    fclose(file);

    return 0;
}


