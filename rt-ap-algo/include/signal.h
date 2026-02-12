
#ifndef __SIGNAL_H__
#define __SIGNAL_H__



/**
	@brief		initializes the parameters and memory linked to the signal module
	@param[in]	algo	    points to the global algo structure
	@return		1 if memory allocation failed, else 0
*/
int signal_init(algo_t* algo);

/**
	@brief		frees the memory linked to the signal module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int signal_free(algo_t* algo);

/**
	@brief		performs all actions in the signal module:
                    - read buffer file
                    - save signal as float vector
                    - compute the RMS value of the signal in the buffer
	@param[in]	algo	    points to the global algo structure
	@return		1 if signal read failed, else 0
*/
int read_buffer_file(algo_t* algo);





#endif // __SIGNAL_H__



