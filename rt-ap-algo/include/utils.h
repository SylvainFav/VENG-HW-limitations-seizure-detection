

#ifndef __UTILS_H__
#define __UTILS_H__



/**
	@brief		searches for a value in a vector of ints
	@param[in]	value	    value to search
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		1 if the value is in the array, else 0
*/
int isinarray(uint32_t value, uint32_t* array, uint32_t arraySize);

/**
	@brief		searches for a value in a vector of floats
	@param[in]	value	    value to search
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		1 if the value is in the array, else 0
*/
int isinfarray(float value, float* array, uint32_t arraySize);

/**
	@brief		finds the maximum value in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		max of vector
*/
float max_farray(float* array, uint32_t arraySize);

/**
	@brief		finds the minimum value in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		min of vector
*/
float min_farray(float* array, uint32_t arraySize);

/**
	@brief		computes the sum of all elements in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		sum of vector
*/
float sum_farray(float* array, uint32_t arraySize);

/**
	@brief		computes the mean of all elements in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		mean of vector
*/
float mean_farray(float* array, uint32_t arraySize);

/**
	@brief		computes the sum of all non-NAN  elements in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		NAN if only NAN values in the input vector, else the sum of all non-NAN values
*/
float nansum_farray(float* array, uint32_t arraySize);

/**
	@brief		computes the mean of all non-NAN  elements in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		NAN if only NAN values in the input vector, else the mean of all non-NAN values
*/
float nanmean_farray(float* array, uint32_t arraySize);

/**
	@brief		computes the standard deviation of all non-NAN  elements in a vector of floats
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
	@return		NAN if only NAN values in the input vector, else the standard deviation of all non-NAN values
*/
float nanstd_farray(float* array, uint32_t arraySize);

/**
	@brief		writes a vector to a file
	@param[in]	array	    points to the input vector
	@param[in]	arraySize	number of samples in the input vector
    @param[in]  filename    points to the name of the file
	@return		0 if the vector is correctly written, else 1
*/
int write_farray_to_file(float* array, uint32_t arraySize, char* filename);

/**
	@brief		writes the elements of a 2D float vector to a file, coma-separated
	@param[in]	array		points to the input 2D vector
	@param[in]	nArrays		number of columns to write
	@param[in]	size		number of rows to write
	@param[in]	filename	points to the name of the file
	@return		0 if the vector is correctly written, else 1
*/
int write_multifarray_to_file(float** array, uint32_t nArrays, uint32_t arraySize, char* filename);






#endif // __UTILS_H__


