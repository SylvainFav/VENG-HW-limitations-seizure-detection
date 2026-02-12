
#ifndef __STIMULI_H__
#define __STIMULI_H__



/**
    @brief      module to generate the stimuli
                in1d and in2d come from the experimental data stored in overlapping buffers
                in1c and in2c is noise generated according to parameters specified in setup.h
    @param[out] in1d        points to the vector of input 1 differential mode
    @param[out] in2d        points to the vector of input 2 differential mode  
    @param[out] in1c        points to the vector of input 1 common mode
    @param[out] in2c        points to the vector of input 2 common mode  
    @param[in]  buffer_idx  index of the considered buffer
    @param[in]  subject     points to the name of the considered subject
    @return     1 if error during file reading, else 0
*/
int stimuliModule(float* in1d, float* in2d, float* in1c, float* in2c, int buffer_idx, char* subject);

/**
    @brief  reads a file containing the input data buffer and oversamples the signal
    @param[in]  filename    points to the name of the file
    @param[out] signal      points to the signal vector
    @return     1 if error during file reading, else 0

*/
int read_input_ffile(char* filename, float* signal);




#endif // __STIMULI_H__
