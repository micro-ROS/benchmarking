/*****************************************************************
 * file: pipeline.h
 * author: Alexandre Malki <amalki@piap.pl>
 * brief:  This is the header of the pipeline_obj object, more information
 * 		about the the message_object in source file pipepline.c .
 *****************************************************************/
#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <processing.h>
#include <stdbool.h>

typedef struct pipeline_obj_st pipeline_obj;

typedef int (*pipeline_attach_src_cb)(pipeline_obj * const obj, 
				      processing_obj * const src);

typedef int (*pipeline_attach_proc_cb)(pipeline_obj * const obj, 
				       processing_obj * const src);

typedef int (*pipeline_stream_data_cb)(pipeline_obj *obj);
typedef bool (*pipeline_is_stopped_cb)(pipeline_obj *obj);

/**
 * This structure holds all the processing objects used in the application.
 * It is mandatory to a processing object to be attached to the pipeline since
 * it will controle the whole processing process.
 */
struct pipeline_obj_st {
	/**
	 * Method to attache the source, this is the root element. There shall
	 * be only one element for the entire pipline
	 */
	pipeline_attach_src_cb   attach_src;
	/**
	 * Method to attach processing element used.
	 */
	pipeline_attach_proc_cb  attach_proc;
	/**
	 * Method that will execute the source pipeline and check for error.
	 */
	pipeline_stream_data_cb  stream_data;
	/* 
	 * Method that set the pipeline to stop 
	 */
	pipeline_is_stopped_cb	 is_stopped;
	/** Internal data */
	void *pdata;
};

/**
 * @brief Initialization function that sets the pipeline
 * @param obj Pipeline object to initialise.
 * @return 0 upon success, -1 otherwise.
 */
int pipeline_init(pipeline_obj *obj);

/**
 * @brief This function  stops the pipeline.
 */
void pipeline_set_end_all(void);

#endif /* __PIPELINE_H__ */
