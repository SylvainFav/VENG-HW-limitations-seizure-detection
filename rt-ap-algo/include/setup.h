
#ifndef __SETUP_H__
#define __SETUP_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// #define DO_PRINT

#define RUN_FOLDER "../outputs/" // General folder to store the results
#define RUN_CATEGORY "ref" // Sub-folder in RUN_FOLDER
#define SAVE_AP_LIST
#define SAVE_OUTPUT

// ========================
// Multi run options
// ========================

#define MULTI_RUN

#define NSUBJECTS 8
extern const char* subject_list[];

// ========================
// Input data
// ========================

#define SUBJECT "P1"

#define INPUT_DATA_TYPE 1 // 1 for 'double', 2 for 'int'

#define SAMPLING_FREQ 20000

// ========================
// Timing
// ========================
#define BUFFER_DURATION 0.5
#define BASELINE_END_IDX 700
#define BUFFER_SIZE 10000
#define N_BUFFERS 3720

// ========================
// Spike detection
// ========================

#define DETECTION_CORRELATION_THRESHOLD 0.75f
#define SPIKE_SIZE 40 // = 2 ms 
#define SPIKE_HALF_SIZE 20
#define N_INIT_TEMPLATES 12
#define DETECTION_MIN_SPIKE_DISTANCE 42 // 42 samples = 2.1 ms = 1 ms (inter-spike distance) + 1.1 ms (max spike width)
#define DETECTION_MIN_AMP_RMS_RATIO 1
#define DETECTION_MAX_AMP_RMS_RATIO 5
#define DETECTION_MAX_NSPIKES 500 // Max # spikes in a buffer
#define DETECTION_MAX_TOTAL_NSPIKES (DETECTION_MAX_NSPIKES * N_BUFFERS) // Max # spikes in whole recording

// ========================
// Template discard
// ========================
#define TEMPLATE_SORT_MIN_NSPIKES 100
#define TEMPLATE_SORT_MIN_NSPIKES_REL 0.05
#define TEMPLATE_SORT_IDX 200

// ========================
// Metric computation
// ========================
#define METRIC_FG_SIZE 20
#define METRIC_BG_SIZE 180
#define METRIC_WDW_SIZES (METRIC_FG_SIZE + METRIC_BG_SIZE)

// ========================
// Custom data structures
// ========================
// Template
typedef struct {
    float*   values;
    uint32_t    nspikes;
} template_t;

// List of spikes
typedef struct {
    float*      amplitudes;
    uint32_t    nspikes;
} spike_list_t;

// Metric window
typedef struct {
    float*      amplitude;
    float*      frequency;
    float       mean_amplitude;
    float       mean_frequency;
} metric_window_t;

// Metric
typedef struct {
    metric_window_t*    fg_window;
    metric_window_t*    bg_window;
    float*              amplitude;
    float*              frequency;
    float*              amplitude_slope;
    float*              frequency_slope;
    float               std_du_baseline;
    float               std_df_baseline;
    uint32_t            start_idx;
    float*              metric;
} metric_t;

// Algo state
typedef struct {
    template_t**        templates;
    spike_list_t*       spike_list;
    metric_t*           metric;
    float*              signal;
    float               sigRMS;
    uint8_t             ntemplates;
    uint32_t            nspikes_cumulated;
    float*              total_spike_amplitudes;
    int*                total_spike_locs;
    uint8_t             do_template_sort;
    uint8_t             phase;
    uint32_t            buffer_idx;
    const char*         subject;
    float               correlation_threshold;
} algo_t;

// ========================
// Functions
// ========================

#include "./utils.h"
#include "./signal.h"
#include "./template.h"
#include "./spike_detection.h"
#include "./metric.h"

int setup(algo_t* algo);
int reset_buffer(algo_t* algo);
int free_mem(algo_t* algo);

#endif // __SETUP_H__

