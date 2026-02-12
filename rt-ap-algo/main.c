

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "./include/setup.h"
#include "./include/utils.h"
#include "./include/signal.h"
#include "./include/template.h"
#include "./include/spike_detection.h"
#include "./include/metric.h"

const char* subject_list[] = {"P1", "P2", "P3", "P4", "P5", "P6", "S1", "S2"};

int main(int argc, char* argv[]) {

    for (int isubject=0; isubject<NSUBJECTS; isubject++) {

        algo_t algo = {0};

        algo.subject = subject_list[isubject];

        printf("Subject %s\n", algo.subject);

        // General setup into algo structure
        int setup_res = setup(&algo);
        if (setup_res != 0) {
            fprintf(stderr, "Error at setup\n");
            return 1;
        }
        
        for (uint32_t ibuff = 0; ibuff < N_BUFFERS; ibuff++) {

            algo.buffer_idx = ibuff;

            // Read signal from buffer file
            int signal_res = read_buffer_file(&algo);
            if (signal_res != 0) {
                fprintf(stderr, "Error at reading buffer %d\n", ibuff);
                return 1;
            }
        
            // Perform spike detection
            int detection_res = buffer_spike_detection(&algo);
            if (detection_res != 0) {
                fprintf(stderr, "Error at spike detection in buffer %d\n", ibuff);
                return 1;
            }

            // At end of phase 1, update templates
            algo.do_template_sort = (algo.do_template_sort || (algo.buffer_idx == TEMPLATE_SORT_IDX));
            if (algo.do_template_sort) {
                if (algo.nspikes_cumulated > TEMPLATE_SORT_MIN_NSPIKES) {
                    #ifdef DO_PRINT
                        printf("Do template sorting\n");
                    #endif
                    int sorting_res = sort_templates(&algo);
                    if (sorting_res != 0) {
                        fprintf(stderr, "Error at template sorting at buffer %d\n", ibuff);
                        return 1;
                    }
                    algo.do_template_sort = 0;
                    algo.metric->start_idx = algo.buffer_idx+1;
                }
                else {
                    fprintf(stderr, "Cannot do template sorting at buffer %d ; only %d spikes detected in total\n", algo.buffer_idx, algo.nspikes_cumulated);
                }
            }

            // Compute metric
            int metric_res = compute_metric(&algo);
            if (metric_res != 0) {
                fprintf(stderr, "Error at metric computation at buffer %d\n", ibuff);
                return 1;
            }
        }

        // Write results to file
        float** results_to_write = (float**) malloc(3 * sizeof(float*));
        results_to_write[0] = algo.metric->amplitude;
        results_to_write[1] = algo.metric->frequency;
        results_to_write[2] = algo.metric->metric;

        int filename_max_size = 200;
        #ifdef SAVE_OUTPUT
            char* output_filename = (char*) malloc(filename_max_size * sizeof(char));
            snprintf(output_filename, filename_max_size, "%s%s/ap_out/corrThresh%d/%s/out.txt", RUN_FOLDER, RUN_CATEGORY, (int)(DETECTION_CORRELATION_THRESHOLD*100), algo.subject);
            int write_res = write_multifarray_to_file(results_to_write, 3, N_BUFFERS, output_filename);
            if (write_res != 0) {
                fprintf(stderr, "Error at output writing\n");
            }
            free(output_filename);
        #endif

        #ifdef SAVE_AP_LIST
            float** ap_results_to_write = (float**) malloc(2 * sizeof(float*));
            ap_results_to_write[0] = (float*) malloc(algo.nspikes_cumulated * sizeof(float));
            ap_results_to_write[1] = (float*) malloc(algo.nspikes_cumulated * sizeof(float));
            for (uint32_t i=0; i<algo.nspikes_cumulated; i++) {
                ap_results_to_write[0][i] = (float) algo.total_spike_locs[i];
                ap_results_to_write[1][i] = (float) algo.total_spike_amplitudes[i];
            }
            char* ap_filename = (char*) malloc(filename_max_size * sizeof(char));
            snprintf(ap_filename, filename_max_size, "%s%s/ap_out/corrThresh%d/%s/ap_list.txt", RUN_FOLDER, RUN_CATEGORY, (int)(DETECTION_CORRELATION_THRESHOLD*100), algo.subject);
            int ap_write_res = write_multifarray_to_file(ap_results_to_write, 2, algo.nspikes_cumulated, ap_filename);
            if (ap_write_res != 0) {
                fprintf(stderr, "Error at output writing\n");
            }

            free(ap_filename);
            free(ap_results_to_write[0]);
            free(ap_results_to_write[1]);
            free(ap_results_to_write);

        #endif

        free_mem(&algo);
    }

    return 0;

}

