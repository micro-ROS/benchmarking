#include <common-macros.h>
#include <debug.h>

#include <pipeline.h>
#include <message.h>

#include <string.h>


typedef struct {
	/** Processing object that can be linked toegther */
	processing_obj *proc_objs[16];
	/** The number of in use objs */
	unsigned int count;
	message_obj parent_msg;
	message_obj child_msg;
	bool is_src_connected;
	bool is_used;
	bool stop;
} pipeline_private_data;

static pipeline_private_data pipeline_pdata;
static bool stop_pipelines = false;

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
 * \brief This function allows to get several pipeline
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

	if (message_init(&pdata->parent_msg)) {
		return -1;
	}

	if (message_init(&pdata->child_msg)) {
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
