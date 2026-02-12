
#ifndef __UTILS_H__
#define __UTILS_H__



///////////////////////////////////////////
//   IIR filters
///////////////////////////////////////////

/**
    @brief          applies 1st-order IIR filtering with the equation alpha[0]*y[n] + alpha[1]*y[n-1] = beta[0]*x[n] + beta[1]*x[n-1]
    @param[in]      sig_in      points to the input vector
    @param[out]     sig_out     points to the output vector
    @param[in]      size        number of samples in the input vector
    @param[in]      alpha       points to the vector of alpha coefficients (numerator), size 2
    @param[in]      beta        points to the vector of beta coefficients (denominator), size 2
	@return			0
*/
int iir_order_1(float* sig_in, float* sig_out, int size, float* alpha, float* beta);

/**
    @brief          applies 2nd-order IIR filtering with the equation alpha[0]*y[n] + alpha[1]*y[n-1] + alpha[2]*y[n-2] = beta[0]*x[n] + beta[1]*x[n-1] + beta[2]*x[n-2]
    @param[in]      sig_in      points to the input vector
    @param[out]     sig_out     points to the output vector
    @param[in]      size        number of samples in the input vector
    @param[in]      alpha       points to the vector of alpha coefficients (numerator), size 3
    @param[in]      beta        points to the vector of beta coefficients (denominator), size 3
	@return			0
*/
int iir_order_2(float* sig_in, float* sig_out, int size, float* alpha, float* beta);

/**
    @brief          applies 2nd-order IIR filtering using integer representation for input/output vectors,
                    with the equation alpha[0]*y[n] + alpha[1]*y[n-1] + alpha[2]*y[n-2] = beta[0]*x[n] + beta[1]*x[n-1] + beta[2]*x[n-2]
    @param[in]      sig_in      points to the input vector (int)
    @param[out]     sig_out     points to the output vector (int)
    @param[in]      size        number of samples in the input vector
    @param[in]      alpha       points to the vector of alpha coefficients (numerator), size 3
    @param[in]      beta        points to the vector of beta coefficients (denominator), size 3
	@return			0
*/
int iir_order_2_int(int* sig_in, int* sig_out, int size, float* alpha, float* beta);




///////////////////////////////////////////
//   Noise generators
///////////////////////////////////////////

/**
    @brief      generates one unique sample of white gaussian noise using the Box-Muller transform
    @param[in]  scale   standard devation of the noise generated
    @return     white noise sample (float)
*/
float white_noise_sample_generator(float scale);

/**
    @brief      generates a vector of band-illimited white gaussian noise using the Box-Muller transform
	@param[out]	white_noise		points to the output vector of white gaussian noise
    @param[in]  size   			number of samples in the output vector
	@param[in]	power			noise power in V^2
	@param[in]	power_band		points to the vector defining the bandwidth in which the noise power is computed
    @return     0
*/
int white_noise_generator(float* white_noise, int size, float power, float* power_band);

/**
	@brief		generates a vector of band-illimited pink noise using the Voss-McCartney algorithm
				calls white_noise_generator
	@param[out]	pink_noise		points to the output vector of pink noise
	@param[in]	size			number of samples in the output vector
	@param[in]	power			noise power in V^2
	@param[in]	power_band		points to the vector defining the bandwidth in which the noise power is computed
	@return		0	
*/
int pink_noise_generator(float* pink_noise, int size, float power, float* power_band);

/**
	@brief		generates a vector of band-illimited white and pink noise
				calls white_noise_generator() and pink_noise_generator()
	@param[out]	noise			points to the output vector of mixed noise, size N_SAMPLES
	@param[in]	power			noise power in V^2
	@param[in]	fcorner			noise corner frequency in Hz
	@param[in]	power_band		points to the vector defining the bandwidth in which the noise power is computed
	@return		0	
*/
int mixed_noise_generator_nsamples(float* noise, float power, float fcorner, float* power_band);



///////////////////////////////////////////
//   Math functions on arrays
///////////////////////////////////////////

/**
	@brief		computes the sum of all elements in a vector of floats
	@param[in]	array	points to the input vector
	@param[in]	size	number of samples in the input vector
	@return		sum of vector
*/
float sumf(float* array, int size);

/**
	@brief		computes the maximum of all elements in a vector of floats
	@param[in]	array	points to the input vector
	@param[in]	size	number of samples in the input vector
	@return		max of vector
*/
float maxf(float* array, int size);

/**
	@brief		computes the minimum of all elements in a vector of floats
	@param[in]	array	points to the input vector
	@param[in]	size	number of samples in the input vector
	@return		min of vector
*/
float minf(float* array, int size);

/**
	@brief		computes the mean of all elements in a vector of floats
	@param[in]	array	points to the input vector
	@param[in]	size	number of samples in the input vector
	@return		mean of vector
*/
float meanf(float* array, int size);

/**
	@brief		computes the rms values of a vector of floats
	@param[in]	array	points to the input vector
	@param[in]	size	number of samples in the input vector
	@return		rms value
*/
float rmsf(float* array, int size);

/**
	@brief		computes the rms values of a vector of floats from which the mean is subtracted
	@param[in]	array	points to the input vector
	@param[in]	size	number of samples in the input vector
	@return		rms value
*/
float rmsf_nomean(float* array, int size);



///////////////////////////////////////////
//   Write to files
///////////////////////////////////////////

/**
	@brief		writes the elements of a float vector to a file, one value on each line
	@param[in]	array		points to the input vector
	@param[in]	size		number of samples in the input vector
	@param[in]	filename	points to the name of the file
	@return		0 if the vector is correctly written, else 1
*/
int write_farray_to_file(float* array, int size, char* filename);

/**
	@brief		writes the elements of an int vector to a file, one value on each line
	@param[in]	array		points to the input vector
	@param[in]	size		number of samples in the input vector
	@param[in]	filename	points to the name of the file
	@return		0 if the vector is correctly written, else 1
*/
int write_intarray_to_file(int* array, int size, char* filename);

/**
	@brief		writes the elements of a 2D float vector to a file, coma-separated
	@param[in]	array		points to the input 2D vector
	@param[in]	nArrays		number of columns to write
	@param[in]	size		number of rows to write
	@param[in]	filename	points to the name of the file
	@return		0 if the vector is correctly written, else 1
*/
int write_multifarray_to_file(float** array, uint32_t nArrays, uint32_t arraySize, char* filename);

/**
	@brief		writes the elements of a 2D int vector to a file, coma-separated
	@param[in]	array		points to the input 2D vector
	@param[in]	nArrays		number of columns to write
	@param[in]	size		number of rows to write
	@param[in]	filename	points to the name of the file
	@return		0 if the vector is correctly written, else 1
*/
int write_multiintarray_to_file(int** array, uint32_t nArrays, uint32_t arraySize, char* filename);





#endif // __UTILS_H__

