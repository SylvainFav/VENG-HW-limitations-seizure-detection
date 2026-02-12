
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/setup.h"
#include "../include/spike_detection.h"


int spike_detection_init(algo_t* algo) {
    algo->spike_list = (spike_list_t*) malloc(sizeof(spike_list_t));
    algo->spike_list->nspikes = 0;
    algo->spike_list->amplitudes = (float*) malloc(DETECTION_MAX_NSPIKES * sizeof(float));
    algo->total_spike_amplitudes = (float*) malloc(DETECTION_MAX_TOTAL_NSPIKES * sizeof(float));
    algo->total_spike_locs = (int*) malloc(DETECTION_MAX_TOTAL_NSPIKES * sizeof(int));
    return 0;
}

int buffer_spike_detection(algo_t* algo) {

    // min/max spike amplitudes of spikes
    float min_amp = algo->sigRMS * DETECTION_MIN_AMP_RMS_RATIO;
    float max_amp = algo->sigRMS * DETECTION_MAX_AMP_RMS_RATIO;

    // zero-padd and normalize signal
    float** norm_sig = (float**) malloc(BUFFER_SIZE * sizeof(float*));
    for (uint32_t i=0; i<BUFFER_SIZE; i++) {
        norm_sig[i] = (float*) malloc(SPIKE_SIZE * sizeof(float));
    }
    float tmp_sig[SPIKE_SIZE];
    float norm;
    float norm_sq = 0.0f;
    float previous_first_value = 0.0f;
    for (uint32_t i=0; i<BUFFER_SIZE; i++) {
        // Zero-padd signal
        for (uint32_t j=0; j<SPIKE_SIZE; j++) {
            if (i + j < SPIKE_HALF_SIZE) {
                tmp_sig[j] = 0.0f;  // zero-padding at start
            }
            else if (i + j >= BUFFER_SIZE + SPIKE_HALF_SIZE ) {
                tmp_sig[j] = 0.0f; // zero-padding at end
            }
            else {
                tmp_sig[j] = algo->signal[i+j-SPIKE_HALF_SIZE];
            }
        }

        // Compute norm of tmp_sig
        if (i == 0) {
            for (uint32_t j=SPIKE_HALF_SIZE; j<SPIKE_SIZE; j++) {
                norm_sq += tmp_sig[j] * tmp_sig[j];
            }
        }
        else {
            norm_sq += tmp_sig[SPIKE_SIZE-1] * tmp_sig[SPIKE_SIZE-1] - previous_first_value * previous_first_value;
        }
        previous_first_value = tmp_sig[0];

        // Normalize sig
        norm = sqrt(norm_sq);
        for (uint32_t j=0; j<SPIKE_SIZE; j++) {
            norm_sig[i][j] = tmp_sig[j] / norm;
        }
    }

    // compute correlation
    float** correlation = (float**) malloc(algo->ntemplates * sizeof(float*));
    float correlation_sum;
    for (uint8_t itemplate=0; itemplate<algo->ntemplates; itemplate++) {
        correlation[itemplate] = (float*) malloc(BUFFER_SIZE * sizeof(float));
        for (uint32_t i=0; i<BUFFER_SIZE; i++) {
            correlation_sum = 0.0f;
            for (uint32_t j=0; j<SPIKE_SIZE; j++) {
                correlation_sum += norm_sig[i][j] * algo->templates[itemplate]->values[j];
            }
            correlation[itemplate][i] = fabs(correlation_sum); // abs because we also want to detect spikes pointing downwards
        }
    }

    for (uint32_t i=0; i<BUFFER_SIZE; i++) {
        free(norm_sig[i]);
    }
    free(norm_sig);

    // find peaks for each template in phase 1 (count spikes)
    if (algo->phase == 1) {
        uint32_t npeaks;
        uint32_t* peak_idx;
        for (uint8_t itemplate=0; itemplate<algo->ntemplates; itemplate++) {
            peak_idx = (uint32_t*) malloc(DETECTION_MAX_NSPIKES * sizeof(uint32_t));
            npeaks = 0;
            for (uint32_t i=1; i<BUFFER_SIZE-1; i++) {
                if ((npeaks == 0 || (i-peak_idx[npeaks-1] >= DETECTION_MIN_SPIKE_DISTANCE))
                    && correlation[itemplate][i] > DETECTION_CORRELATION_THRESHOLD
                    && correlation[itemplate][i] > correlation[itemplate][i-1] 
                    && correlation[itemplate][i] > correlation[itemplate][i+1]) {
                    // peak detected
                    peak_idx[npeaks++] = i;
                }
            }

            algo->templates[itemplate]->nspikes += npeaks;
            free(peak_idx);
        }
    }

    // select highest correlation among templates
    float* max_correlation = (float*) malloc(BUFFER_SIZE * sizeof(float));
    float max_correlation_i;
    for (uint32_t i=0; i<BUFFER_SIZE; i++) {
        max_correlation_i = 0.0f;
        for (uint8_t itemplate=0; itemplate<algo->ntemplates; itemplate++) {
            if (correlation[itemplate][i] > max_correlation_i) {
                max_correlation_i = correlation[itemplate][i];
            }
        }
        max_correlation[i] = max_correlation_i;
    }

    // find peaks in max correlation
    uint32_t npeaks = 0;
    uint32_t* peak_idx = (uint32_t*) malloc(BUFFER_SIZE * sizeof(uint32_t));
    float* peak_amp = (float*) malloc(BUFFER_SIZE * sizeof(float));
    float reconstructed_signal[SPIKE_SIZE];
    float amp_peak_to_peak;
    for (uint32_t i=1+SPIKE_SIZE; i<BUFFER_SIZE-SPIKE_SIZE; i++) {  // discard peaks on the edges
        if ((npeaks == 0 || (i-peak_idx[npeaks-1] >= DETECTION_MIN_SPIKE_DISTANCE))
            && max_correlation[i] > DETECTION_CORRELATION_THRESHOLD  
            && max_correlation[i] > max_correlation[i-1] 
            && max_correlation[i] > max_correlation[i+1]) {
                // check amplitude constraints
                for (uint8_t j=0; j<SPIKE_SIZE; j++) {
                    reconstructed_signal[j] = algo->signal[i+j-SPIKE_HALF_SIZE];
                }
                amp_peak_to_peak = max_farray(reconstructed_signal, SPIKE_SIZE) - min_farray(reconstructed_signal, SPIKE_SIZE);
                if (amp_peak_to_peak >= min_amp && amp_peak_to_peak <= max_amp) {
                    // valid spike
                    peak_idx[npeaks++] = i;
                    peak_amp[npeaks-1] = amp_peak_to_peak;
                }
            }
    }

    // save spikes in structure
    algo->spike_list->nspikes = npeaks;

    uint32_t k = 0;
    uint32_t loc_offset = algo->buffer_idx * BUFFER_SIZE;
    for (uint32_t i=0; i<npeaks;i++) {
        k = i + algo->nspikes_cumulated;
        algo->total_spike_amplitudes[k] = peak_amp[i];
        algo->total_spike_locs[k] = loc_offset + peak_idx[i];
    }
    algo->nspikes_cumulated += npeaks;
    for (uint32_t i=0; i<npeaks; i++) {
        algo->spike_list->amplitudes[i] = peak_amp[i];
    }

    #ifdef DO_PRINT
        float mean_amp = nanmean_farray(algo->spike_list->amplitudes, npeaks);
        printf("Found %d spikes with mean amplitude %.2f in buffer %d with %d templates\n", algo->spike_list->nspikes, mean_amp, algo->buffer_idx, algo->ntemplates);
    #endif

    // free memory
    for (uint8_t i=0; i<algo->ntemplates; i++) {
        free(correlation[i]);
    }
    free(correlation);
    free(max_correlation);
    free(peak_idx);
    free(peak_amp);

    return 0;
}

int spike_detection_free(algo_t* algo) {
    if (algo->spike_list != NULL) {
        if (algo->spike_list->amplitudes != NULL) {
            free(algo->spike_list->amplitudes);
            algo->spike_list->amplitudes = NULL;
        }
        free(algo->spike_list);
        algo->spike_list = NULL;
    }
    free(algo->total_spike_amplitudes);
    free(algo->total_spike_locs);
    return 0;
}
