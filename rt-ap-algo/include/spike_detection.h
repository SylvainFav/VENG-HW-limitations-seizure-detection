
#ifndef __SPIKE_DETECTION_H__
#define __SPIKE_DETECTION_H__

/**
	@brief		initializes the parameters and memory linked to the spike detection module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int spike_detection_init(algo_t* algo);

/**
	@brief		performs spike detection from the signal:
                    - signal zero-padding and normalization
                    - correlation with each template 
                    - count spikes detected with each template
                    - select highest correlation among all templates
                    - detect spikes in max correlation function
                    - discard spikes below min amplitude (noise) or above max amplitude (artifacts)
                    - save detected spikes
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int buffer_spike_detection(algo_t* algo);

/**
	@brief		frees the memory linked to the spike detection module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int spike_detection_free(algo_t* algo);

#endif // __SPIKE_DETECTION_H__
