
#ifndef __DFILT_H__
#define __DFILT_H__

/**
    @brief      module to represent the digital filters
                low-pass and high-pass filtering
    @param[in]  in         points to the boolean 2D vector (one per bit) of the input signal
    @param[out] out        points to the integer vector of the output signal 
    @return     0
*/
int dfiltModule(int** in, int* out);

#endif // __DFILT_H__

