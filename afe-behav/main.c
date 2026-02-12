

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "./include/setup.h"
#include "./include/utils.h"
#include "./include/stimuli.h"
#include "./include/pcb.h"
#include "./include/ia.h"
#include "./include/afilt.h"
#include "./include/adc.h"
#include "./include/dfilt.h"
#include "./include/decim.h"

const char* subject_list[] = {"P1", "P2", "P3", "P4", "P5", "P6", "S1", "S2"};

int main(int argc, char* argv[]) {
        
    // Memory allocation

    float* in1d = (float*)malloc(N_SAMPLES * sizeof(float));
    float* in2d = (float*)malloc(N_SAMPLES * sizeof(float));
    float* in1c = (float*)malloc(N_SAMPLES * sizeof(float));
    float* in2c = (float*)malloc(N_SAMPLES * sizeof(float));

    float* pcbOut1d = (float*)malloc(N_SAMPLES * sizeof(float));
    float* pcbOut2d = (float*)malloc(N_SAMPLES * sizeof(float));
    float* pcbOut1c = (float*)malloc(N_SAMPLES * sizeof(float));
    float* pcbOut2c = (float*)malloc(N_SAMPLES * sizeof(float));

    float* iaOut = (float*)malloc(N_SAMPLES * sizeof(float));

    float* afiltOutp = (float*)malloc(N_SAMPLES * sizeof(float));
    float* afiltOutn = (float*)malloc(N_SAMPLES * sizeof(float));

    int** adcOut = (int**)malloc(ADC_NBITS * sizeof(int*));
    for (int n=0; n<ADC_NBITS; n++) {
        adcOut[n] = (int*)malloc(ADC_NSAMPLES * sizeof(int));
    }

    int* dfiltOut = (int*)malloc(ADC_NSAMPLES * sizeof(int));

    int* out = (int*)malloc(OUT_NSAMPLES * sizeof(int));

    char* output_filename = (char*)malloc(100 * sizeof(char));

    char* subject;

    for (int n=0; n<NSUBJECTS; n++) {

        subject = (char*) subject_list[n];

        printf("Running for subject %s\n", subject);

        // Runs
        for (int i=0; i<N_BUFFERS; i++) {
            #ifdef DO_PRINT
                printf("Buffer %d\n", i+1);
            #endif
            stimuliModule(in1d, in2d, in1c, in2c, i, subject);
            #ifdef DO_PRINT
                printf("Generated stimuli\n");
            #endif
            pcbModule(in1d, in2d, in1c, in2c, pcbOut1d, pcbOut2d, pcbOut1c, pcbOut2c);
            #ifdef DO_PRINT
                printf("Applied PCB filtering\n");
            #endif
            iaModule(pcbOut1d, pcbOut2d, pcbOut1c, pcbOut2c, iaOut);
            #ifdef DO_PRINT
                printf("Applied IA module\n");
            #endif
            afiltModule(iaOut, afiltOutp, afiltOutn);
            #ifdef DO_PRINT
                printf("Applied analog filters module\n");
            #endif
            adcModule(afiltOutp, afiltOutn, adcOut);
            #ifdef DO_PRINT
                printf("Applied ADC module\n");
            #endif
            dfiltModule(adcOut, dfiltOut);
            #ifdef DO_PRINT
                printf("Applied digital filters module\n");
            #endif
            decimModule(dfiltOut, out);
            #ifdef DO_PRINT
                printf("Applied decimation module\n");
            #endif

            sprintf(output_filename, "%s%s/behav_out/%s/buffer%d.txt", RUN_FOLDER, RUN_CATEGORY, subject, i+1);
            write_intarray_to_file(out, OUT_NSAMPLES, output_filename);
            #ifdef DO_PRINT
                printf("Wrote output to file %s\n", output_filename);
            #endif
        }
    }
    printf("Done\n");

    // Free memory
    free(in1d);
    free(in2d);
    free(in1c);
    free(in2c);
    free(pcbOut1d);
    free(pcbOut2d);
    free(pcbOut1c);
    free(pcbOut2c);
    free(iaOut);
    free(afiltOutp);
    free(afiltOutn);
    for (int n=0; n<ADC_NBITS; n++) {
        free(adcOut[n]);
    }
    free(adcOut);
    free(dfiltOut);
    free(out);
    free(output_filename);

    return 0;

}


