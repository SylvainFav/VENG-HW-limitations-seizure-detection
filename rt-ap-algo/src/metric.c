
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/setup.h"
#include "../include/metric.h"

int metric_init(algo_t* algo) {

    algo->metric = (metric_t*) malloc(sizeof(metric_t));
    algo->metric->fg_window = (metric_window_t*) malloc(sizeof(metric_window_t));
    algo->metric->bg_window = (metric_window_t*) malloc(sizeof(metric_window_t));
    algo->metric->fg_window->amplitude = (float*) malloc(METRIC_FG_SIZE * sizeof(float));
    algo->metric->fg_window->frequency = (float*) malloc(METRIC_FG_SIZE * sizeof(float));
    algo->metric->bg_window->amplitude = (float*) malloc(METRIC_BG_SIZE * sizeof(float));
    algo->metric->bg_window->frequency = (float*) malloc(METRIC_BG_SIZE * sizeof(float));
    algo->metric->fg_window->mean_amplitude = 0.0;
    algo->metric->fg_window->mean_frequency = 0.0;
    algo->metric->bg_window->mean_amplitude = 0.0;
    algo->metric->bg_window->mean_frequency = 0.0;
    algo->metric->std_du_baseline = NAN;
    algo->metric->std_df_baseline = NAN;
    algo->metric->start_idx = N_BUFFERS + 1;
    algo->metric->amplitude = (float*) malloc(N_BUFFERS * sizeof(float));
    algo->metric->frequency = (float*) malloc(N_BUFFERS * sizeof(float));
    algo->metric->amplitude_slope = (float*) malloc(N_BUFFERS * sizeof(float));
    algo->metric->frequency_slope = (float*) malloc(N_BUFFERS * sizeof(float));
    algo->metric->metric = (float*) malloc(N_BUFFERS * sizeof(float));

    return 0;
}

int compute_metric(algo_t* algo) {


    // Update algo phases
    if (algo->phase == 1 && algo->buffer_idx >= algo->metric->start_idx) {
        #ifdef DO_PRINT
            printf("Buffer %d ; go to phase 2\n", algo->buffer_idx);
        #endif
        algo->phase = 2;
    }
    if (algo->phase == 2 && algo->buffer_idx >= algo->metric->start_idx + METRIC_WDW_SIZES) {
        #ifdef DO_PRINT
            printf("Buffer %d ; go to phase 3\n", algo->buffer_idx);
        #endif
        algo->phase = 3;
    }
    else if (algo->phase == 3 && algo->buffer_idx > BASELINE_END_IDX) {
        #ifdef DO_PRINT
            printf("Buffer %d ; go to phase 4\n", algo->buffer_idx);
        #endif
        algo->phase = 4;
    }

    // Compute mean amp/freq
    float mean_amp = nanmean_farray(algo->spike_list->amplitudes, algo->spike_list->nspikes);
    float mean_freq = ((float) algo->spike_list->nspikes) / BUFFER_DURATION;

    algo->metric->amplitude[algo->buffer_idx] = mean_amp;
    algo->metric->frequency[algo->buffer_idx] = mean_freq;

    // Compute window arrays
    uint32_t window_idx;
    if (algo->buffer_idx <= algo->metric->start_idx) {
        // Do nothing
    } else if (algo->buffer_idx <= algo->metric->start_idx + METRIC_BG_SIZE - 1) {
        // Fill BG window
        window_idx = algo->buffer_idx - algo->metric->start_idx; // Goes from 0 to BG_SIZE-1
        algo->metric->bg_window->amplitude[window_idx] = mean_amp;
        algo->metric->bg_window->frequency[window_idx] = mean_freq;
        if (!__isnanf(mean_amp)) {
            algo->metric->bg_window->mean_amplitude += mean_amp / METRIC_BG_SIZE;
        }
        algo->metric->bg_window->mean_frequency += mean_freq / METRIC_BG_SIZE;

    } else if (algo->buffer_idx <= algo->metric->start_idx + METRIC_WDW_SIZES - 1) {
        // Fill FG window
        window_idx = algo->buffer_idx - algo->metric->start_idx - METRIC_BG_SIZE; // Goes from 0 to FG_SIZE-1
        algo->metric->fg_window->amplitude[window_idx] = mean_amp;
        algo->metric->fg_window->frequency[window_idx] = mean_freq;
        if (!__isnanf(mean_amp)) {
            algo->metric->fg_window->mean_amplitude += mean_amp / METRIC_FG_SIZE;
        }
        algo->metric->fg_window->mean_frequency += mean_freq / METRIC_FG_SIZE;
    } else {
        // Update arrays in windows
        for (uint32_t i=0; i<METRIC_BG_SIZE-1; i++) {
            algo->metric->bg_window->amplitude[i] = algo->metric->bg_window->amplitude[i+1];
            algo->metric->bg_window->frequency[i] = algo->metric->bg_window->frequency[i+1];
        }
        algo->metric->bg_window->amplitude[METRIC_BG_SIZE-1] = algo->metric->fg_window->amplitude[0];
        algo->metric->bg_window->frequency[METRIC_BG_SIZE-1] = algo->metric->fg_window->frequency[0];

        for (uint32_t i=0; i<METRIC_FG_SIZE-1; i++) {
            algo->metric->fg_window->amplitude[i] = algo->metric->fg_window->amplitude[i+1];
            algo->metric->fg_window->frequency[i] = algo->metric->fg_window->frequency[i+1];
        }
        algo->metric->fg_window->amplitude[METRIC_FG_SIZE-1] = mean_amp;
        algo->metric->fg_window->frequency[METRIC_FG_SIZE-1] = mean_freq;

        // Update mean values in windows
        if (!__isnanf(algo->metric->bg_window->amplitude[0])) {
            algo->metric->bg_window->mean_amplitude -= algo->metric->bg_window->amplitude[0] / METRIC_BG_SIZE;
        }
        if (!__isnanf(algo->metric->fg_window->amplitude[0])) {
            algo->metric->bg_window->mean_amplitude += algo->metric->fg_window->amplitude[0] / METRIC_BG_SIZE;
            algo->metric->fg_window->mean_amplitude -= algo->metric->fg_window->amplitude[0] / METRIC_FG_SIZE;
        }
        if (!__isnanf(mean_amp)) {
            algo->metric->fg_window->mean_amplitude += mean_amp / METRIC_FG_SIZE;
        }
        algo->metric->bg_window->mean_frequency += (algo->metric->fg_window->frequency[0] - algo->metric->bg_window->frequency[0]) / METRIC_BG_SIZE;
        algo->metric->fg_window->mean_frequency += (mean_freq - algo->metric->fg_window->frequency[0]) / METRIC_FG_SIZE;
    }

    // Compute slopes
    if (algo->phase >= 3) {
        algo->metric->amplitude_slope[algo->buffer_idx] = algo->metric->fg_window->mean_amplitude / algo->metric->bg_window->mean_amplitude;
        algo->metric->frequency_slope[algo->buffer_idx] = algo->metric->fg_window->mean_frequency / algo->metric->bg_window->mean_frequency;
    } else {
        algo->metric->amplitude_slope[algo->buffer_idx] = NAN;
        algo->metric->frequency_slope[algo->buffer_idx] = NAN;
    }

    // Compute STDs at the end of phase 3
    if (algo->buffer_idx == BASELINE_END_IDX) {
        algo->metric->std_du_baseline = nanstd_farray(algo->metric->amplitude_slope, algo->buffer_idx+1);
        algo->metric->std_df_baseline = nanstd_farray(algo->metric->frequency_slope, algo->buffer_idx+1);
        #ifdef DO_PRINT
            printf("Computed STDs at the end of phase 3: amplitude: %.3f ; frequency: %.3f\n", algo->metric->std_du_baseline, algo->metric->std_df_baseline);
        #endif
    }

    // Compute metric
    if (algo->phase >= 4) {
        float amp_slope_norm = (algo->metric->amplitude_slope[algo->buffer_idx] - 1) / algo->metric->std_du_baseline;
        float freq_slope_norm = (algo->metric->frequency_slope[algo->buffer_idx] - 1) / algo->metric->std_df_baseline;
        algo->metric->metric[algo->buffer_idx] = (amp_slope_norm + 1) * (freq_slope_norm + 1);
        #ifdef DO_PRINT
            printf("Computed metric at buffer %d with value %.3f\n", algo->buffer_idx, algo->metric->metric[algo->buffer_idx]);
        #endif    
    } else {
        algo->metric->metric[algo->buffer_idx] = NAN;
    }



    return 0;
}

int metric_free(algo_t* algo) {

    if (algo->metric != NULL) {
        if (algo->metric->fg_window != NULL) {
            if (algo->metric->fg_window->amplitude != NULL) {
                free(algo->metric->fg_window->amplitude);
                algo->metric->fg_window->amplitude = NULL;
            }
            if (algo->metric->fg_window->frequency != NULL) {
                free(algo->metric->fg_window->frequency);
                algo->metric->fg_window->frequency = NULL;
            }
            free(algo->metric->fg_window);
            algo->metric->fg_window = NULL;
        }
        if (algo->metric->bg_window != NULL) {
            if (algo->metric->bg_window->amplitude != NULL) {
                free(algo->metric->bg_window->amplitude);
                algo->metric->bg_window->amplitude = NULL;
            }
            if (algo->metric->bg_window->frequency != NULL) {
                free(algo->metric->bg_window->frequency);
                algo->metric->bg_window->frequency = NULL;
            }
            free(algo->metric->bg_window);
            algo->metric->bg_window = NULL;
        }
        if (algo->metric->amplitude != NULL) {
            free(algo->metric->amplitude);
            algo->metric->amplitude = NULL;
        }
        if (algo->metric->frequency != NULL) {
            free(algo->metric->frequency);
            algo->metric->frequency = NULL;
        }
        if (algo->metric->metric != NULL) {
            free(algo->metric->metric);
            algo->metric->metric = NULL;
        }
        free(algo->metric);
        algo->metric = NULL;
    }

    return 0;
}

