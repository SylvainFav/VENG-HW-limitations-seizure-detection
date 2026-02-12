
#ifndef __IA_H__
#define __IA_H__

/**
    @brief      module to represent the instrumentation 
                gain
                summation of the two input channels
                finite CMRR
                addition of white and pink noise
                low-pass filtering at the output
    @param[in]  in1d        points to the vector of input 1 differential mode
    @param[in]  in2d        points to the vector of input 2 differential mode  
    @param[in]  in1c        points to the vector of input 1 common mode
    @param[in]  in2c        points to the vector of input 2 common mode  
    @param[out] out         points to the vector of the output signal
    @return     0
*/
int iaModule(float* in1d, float* in2d, float* in1c, float* in2c, float* out);

#endif // __IA_H__

