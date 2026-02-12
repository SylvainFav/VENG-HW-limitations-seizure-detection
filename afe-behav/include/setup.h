
#ifndef __SETUP_H__
#define __SETUP_H__

///////////////////////////////////////////
//   RUN OPTIONS
///////////////////////////////////////////

// #define DO_PRINT // Print progress and info at every buffer (slows down simulation)

#define NSUBJECTS 8 // Number of subjects studied
extern const char* subject_list[];

#define VENG_DATA_FOLDER "../dummy_inputs/" // Folder where experimental data buffers are stored
#define RUN_FOLDER "../outputs/" // General folder to store the results
#define RUN_CATEGORY "ref" // Sub-folder in RUN_FOLDER

#define NOISY // Add noise in the IA (slows down simulation)
#define SATURATE // Apply saturation on AFILT outputs

///////////////////////////////////////////
//   CONSTANTS
///////////////////////////////////////////

#define FS 640000 // General time resolution for all analog functions
#define N_SAMPLES 640000 // Number of samples at FS in a buffer (1-second buffers)

#define PI 3.1415926535897932384626433
#define TWO_PI (2*PI)
#define HALF_PI (PI/2)

// Bandwidth in which to compute the noise
#define FMIN 300.0f
#define FMAX 3000.0f
#define FL (HALF_PI * FMIN)
#define FH (HALF_PI * FMAX)

///////////////////////////////////////////
//   STIMULI
///////////////////////////////////////////

#define N_BUFFERS 3720 // 31-min long recordings

// Input experimental data is sampled at 80 kS/s => oversampling necessary
#define INPUT_FS_RATIO 8 // 80 kS/s = FS/8
#define INPUT_NSAMPLES (N_SAMPLES / INPUT_FS_RATIO)

#define INPUT_CM 1e-3 // 1 mV
// Input CM power is integrated from INPUT_CM_FMIN to INPUT_CM_FMAX
#define INPUT_CM_FMIN 1.0f // 1 Hz
#define INPUT_CM_FMAX 100e3 // 100 kHz 

///////////////////////////////////////////
//   PCB
///////////////////////////////////////////

#define PCB_FL 269 // 269 Hz

///////////////////////////////////////////
//   IA
///////////////////////////////////////////

#define IA_CMRR 5623.4f // 75 dB
#define IA_NOISE 0.00000116f // 1.16 uV
#define IA_GAIN 363.1f // 51.2 dB
#define IA_FCORNER 1000.0f // 1 kHz
#define IA_FH 43000 // 43 kHz

///////////////////////////////////////////
//   ANALOG FILT
///////////////////////////////////////////

#define AFILT_GAIN 123.0f // 41.8 dB
#define AFILT_FL1 48 // 48 Hz
#define AFILT_FL2 7 // 7 Hz
#define AFILT_FH 3000 // 3 kHz
#define AFILT_DC_OUT 0.6f // 0.6 V = VDD/2 (DC common-mode voltage)
#define AFILT_DR_MAX 0.75f // 0.75 V (saturation voltage)

///////////////////////////////////////////
//   ADC
///////////////////////////////////////////

#define OUT_FS_RATIO 32 // output fs = 20 kS/s = 640 kS/s / 32
#define ADC_OSR_LOG 3 // LOG2(OSR) = LOG2(8) = 3 (ADC oversampling)
#define ADC_FREQUENCY_RATIO (OUT_FS_RATIO >> ADC_OSR_LOG)
#define ADC_NSAMPLES (N_SAMPLES / ADC_FREQUENCY_RATIO)
#define ADC_FULLSCALE 1.824f // max value - min value, in V
#define ADC_MIDRANGE 0.6 // in V
#define ADC_VMIN (ADC_MIDRANGE - ADC_FULLSCALE/4)
#define ADC_VMAX (ADC_MIDRANGE + ADC_FULLSCALE/4)
#define ADC_NBITS 12
#define ADC_INTMAX (pow(2, ADC_NBITS)-1)

///////////////////////////////////////////
//   DIGITAL FILT
///////////////////////////////////////////

#define OUT_NBITS 16 // 16 bits to fit to half a 32-bit word, no impact
#define DFILT_FL 200 // 200 Hz
#define DFILT_FH 3000 // 3 kHz

///////////////////////////////////////////
//   DECIMATION
///////////////////////////////////////////

#define OUT_NSAMPLES ((int)(N_SAMPLES / OUT_FS_RATIO))

///////////////////////////////////////////
//   MISC
///////////////////////////////////////////

#define PINK_NOISE_NSOURCES 16 // Parameter for pink noise generation

// Look-up table for IIR filter coefficients
#include "./filt_lookup.h"

#endif // __SETUP_H__

