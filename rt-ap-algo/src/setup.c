
#include "../include/setup.h"

int setup(algo_t* algo) {

    // Initialize values
    algo->buffer_idx        = 0;
    algo->phase             = 1;
    algo->do_template_sort  = 0;

    // Init (memory allocation)
    signal_init(algo);
    template_init(algo);
    spike_detection_init(algo);
    metric_init(algo);

    return 0;

}

int reset_buffer(algo_t* algo) {

    // Reset signal
    for (uint32_t i=0; i < BUFFER_SIZE; i++) {
        algo->signal[i] = 0.0f;
    }

    return 0;

}

int free_mem(algo_t* algo) {
    signal_free(algo);
    template_free(algo);
    spike_detection_free(algo);
    metric_free(algo);
    return 0;
}
