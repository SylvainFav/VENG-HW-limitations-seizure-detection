
#ifndef __ADC_H__
#define __ADC_H__

/**
    @brief      module to represent the analog-to-digital converter
                saturation (clipping) within full scale
                number of bits
                sampling rate
    @param[in]  inp        points to the float vector of the input positive signal
    @param[in]  inn        points to the float vector of the input negative signal
    @param[out] out        points to the boolean 2D vector (one per bit) of the output signal
    @return     0
*/
int adcModule(float* inp, float* inn, int** out);

#endif // __ADC_H__