/**
 * @file form.h
 * @brief This file contains all definition needed by all source file. Also it
 *		contains definitions of the generic config object.
 * @author	Alexandre Malki <amalki@piap.pl>
 */
#ifndef __FORM_H__
#define __FORM_H__

#include <processing.h>
#include <pipeline.h>


typedef struct form_obj_st form_obj;

/** This structure inherits from the processing object */
struct form_obj_st {
	/** Processing abstraction object */
	processing_obj		proc_obj;
	/** Internal data structure */
	void *pdata;
};

/**
 * @brief Initialization of the cjson object 
 * @param obj form_obj Object to initialize.
 * @return 0 upon sucess, -1 otherwise.
 */
int form_cjson_init(form_obj * const obj);

/**
 * @brief Initialization of the cjson object 
 * @param obj form_obj Object to de-initialize.
 * @return 0 upon sucess, -1 otherwise.
 */
int form_cjson_fini(form_obj * const obj);

#endif /*  __FORM_H__ */
