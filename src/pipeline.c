/*****************************************************************
 * file: pipeline.c
 * author: Alexandre Malki <amalki@piap.pl>
 * brief: This file contains method and function that will allow to control
 * 		the pipeline. The pipeline holds processing objects that are
 *		are used. Every processing_obj shall attached to the pipeline.
 *		A source processing object need also to be set. Otherwise the pipeline
 *		will not be able to start the data streaming. Data streaming means that
 *		the data are being passed from a parent processing object to his
 *		child(ren).
 *****************************************************************/
#include <common-macros.h>
#include <debug.h>

#include <pipeline.h>
#include <message.h>

#include <string.h>


/**
 * This is the pipeline's internal structure
 */
typedef struct {
	/** Processing object that can be linked toegther. */
	processing_obj *proc_objs[16];
	/** The number of processing objects in use. */
	unsigned int count;
	/** This boolean makes sure that the src is set. */
	bool is_src_connected;
	/**
	 * This boolean provide information that the pipeline
	 * initialized or not.
	 */
	bool is_used;
	/** This boolean request the pipeline to stop. */
	bool stop;
} pipeline_private_data;

/** Private instance of the pipeline, only one available */
static pipeline_private_data pipeline_pdata;

/** Global pipeline stop */
static bool stop_pipelines = false;

/**
 * @brief This callback set the source processing object. If a processing
 * 		object is already set, then it will fail.
 * @param obj pipeline object the processing object will be attached to.
 * @param src processing_obj to attach to the pipeline.
 * @return 0 upon success, -1 otherwise.
 */
static int pipeline_attach_src(pipeline_obj * const obj, processing_obj *const src)
{
	pipeline_private_data *const pdata =
	       		(pipeline_private_data * const) obj->pdata;

	if (pdata->is_src_connected) {
		ERROR("Src already connected\n");
		return -1;
	}

	pdata->proc_objs[pdata->count] = src;
	pdata->is_src_connected = true;
	pdata->count++;

	return 0;
}

/**
 * @brief This callback set all processing object except the source.
 * @param obj pipeline object the processing object will be attached to.
 * @param src processing_obj to attach to the pipeline.
 * @return 0 upon success, -1 otherwise.
 */
static int pipeline_attach_proc(pipeline_obj * const obj,
			   processing_obj * const src)
{
	pipeline_private_data * const pdata =
	       		(pipeline_private_data * const) obj->pdata;

	if (!pdata->is_src_connected) {
		ERROR("Source not connected, please connect source first\n");
		return -1;
	}
	pdata->proc_objs[pdata->count] = src;
	pdata->count++;
	return 0;
}

/**
 * @brief This pipeline streams data. It will call the source element and start
 * 		it. Upon stop request, it will set the req_end to gracefully stop
 * 		all  processing object.
 * @param obj The pipeline that will stream the data.
 * @return 0 upon success, -1 otherwise.
 */
static int pipeline_stream_data(pipeline_obj * const obj)
{
	pipeline_private_data * const pdata =
			(pipeline_private_data * const ) obj->pdata;

	processing_obj *el = pdata->proc_objs[0];

	if (pdata->count < 2) {
		ERROR("The pipeline is incomplete\n");
		return -1;
	}

	if (stop_pipelines) {
		DEBUG("Requesting to end pipeline and processing elements\n");
		for (unsigned int j = 0; j < pdata->count; j++) {
			pdata->proc_objs[j]->req_end = true;
		}
		pdata->stop = true;
	}

	return pdata->proc_objs[0]->execute_out(pdata->proc_objs[0]);
}

/**
 * @brief This function will return if the pipeline is stopped or not.
 * @param obj pipeline object.
 * @return true if the pipeline is fully stopped (all processing element
 * 		as well) and false otherwise.
 */
static bool pipeline_get_stop(pipeline_obj *obj)
{
	pipeline_private_data * const pdata =
			(pipeline_private_data * const) obj->pdata;

	return pdata->stop;
}

void pipeline_set_end_all(void) {

	stop_pipelines = true;
}

/**
 * @brief This function allows to get several pipeline
 * @return Pipeline private data clean.
 */
static pipeline_private_data *pipeline_get_instance()
{
	memset(&pipeline_pdata, 0, sizeof(pipeline_pdata));
	return  &pipeline_pdata;
}

int pipeline_init(pipeline_obj *obj)
{
	pipeline_private_data *pdata = pipeline_get_instance();

	if (pdata->is_used) {
		WARNING("Pipeline already in use\n");
		return -1;
	}

	memset(obj, 0, sizeof(*obj));
	obj->attach_src = pipeline_attach_src;
	obj->attach_proc = pipeline_attach_proc;
	obj->stream_data = pipeline_stream_data;
	obj->is_stopped = pipeline_get_stop;

	obj->pdata = pdata;
	pdata->is_used = true;
	pdata->stop = false;
	DEBUG("Pipeline initialized\n");

	return 0;
}
