
#ifndef __AFILT_H__
#define __AFILT_H__

/**
    @brief      module to represent the analog filters
                gain
                high-pass and low-pass filtering
                saturation (clipping)
    @param[in]  in          points to the vector of the input signal
    @param[out] outp        points to the vector of the positive output signal
    @param[out] outn        points to the vector of the negative output signal
    @return     0
*/
int afiltModule(float* in, float* outp, float* outn);

#endif // __AFILT_H__

