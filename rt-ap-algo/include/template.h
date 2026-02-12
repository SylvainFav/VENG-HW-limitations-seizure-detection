
#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

/**
	@brief		initializes the parameters and memory linked to the template module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int template_init(algo_t* algo);

/**
	@brief		at the end of phase 1, discards the templates that detected too few spikes
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int sort_templates(algo_t* algo);

/**
	@brief		frees the memory linked to the template module
	@param[in]	algo	    points to the global algo structure
	@return		0
*/
int template_free(algo_t* algo);

#endif // __TEMPLATE_H__
