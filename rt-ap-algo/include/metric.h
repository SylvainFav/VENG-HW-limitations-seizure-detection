
#ifndef __METRIC_H__
#define __METRIC_H__

/**
	@brief		initializes the parameters and memory linked to the seizure metric module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int metric_init(algo_t* algo);

/**
	@brief		computes the seizure metric:
                    - updates the algo phase as a function of the buffer idx
                    - computes the mean spike amplitude and the number of spikes in the buffer
                    - fills and updates the foreground/background windows
                    - computes the variations in spike amplitude / frequency
                    - computes the seizure metric from the normalized variations
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int compute_metric(algo_t* algo);

/**
	@brief		frees the memory linked to the seizure metric module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int metric_free(algo_t* algo);

#endif // __METRIC_H__



