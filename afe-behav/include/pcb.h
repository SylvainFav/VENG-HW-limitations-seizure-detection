
#ifndef __PCB_H__
#define __PCB_H__

/**
    @brief      module to represent the off-chip components of the front end
                high-pass filtering function
    @param[in]  in1d        points to the vector of input 1 differential mode
    @param[in]  in2d        points to the vector of input 2 differential mode  
    @param[in]  in1c        points to the vector of input 1 common mode
    @param[in]  in2c        points to the vector of input 2 common mode  
    @param[out] out1d       points to the vector of output 1 differential mode
    @param[out] out2d       points to the vector of output 2 differential mode  
    @param[out] out1c       points to the vector of output 1 common mode
    @param[out] out2c       points to the vector of output 2 common mode  
    @return     0
*/
int pcbModule(float* in1d, float* in2d, float* in1c, float* in2c, float* out1d, float* out2d, float* ou1c, float* out2c);

#endif // __PCB_H__

