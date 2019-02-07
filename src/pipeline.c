#include <common-macros.h>
#include <debug.h>

#include <pipeline.h>
#include <message.h>

#include <stdbool.h>
#include <string.h>


typedef struct {
	/** Processing object that can be linked toegther */
	processing_obj *proc_objs[16];
	/** The number of in use objs */
	unsigned int count;
	message_obj msg;
	bool is_sink_connected;
	bool is_src_connected;
	bool is_used;
} pipeline_private_data;

static pipeline_private_data pipeline_pdata;

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
			   processing_obj * const src,
	       		   processing_obj * const dst)
{
	pipeline_private_data * const pdata =
	       		(pipeline_private_data * const) obj->pdata;

	if (!pdata->is_src_connected) {
		ERROR("Source not connected, please connect source first\n");
		return -1;
	}

	if (pdata->is_sink_connected) {
		ERROR("Sink is connected , please remove sink first\n");
		return -1;
	}

	pdata->proc_objs[pdata->count] = src;
	pdata->count++;
	return 0;
}

static int pipeline_attach_sink(pipeline_obj * const obj, processing_obj * const sink)
{
	pipeline_private_data * const pdata =
	       		(pipeline_private_data * const) obj->pdata;

	if (pdata->count == ARRAY_SIZE(pdata->proc_objs)) {
		ERROR("Cannot add sink, too many proc obj");
		return -1;
	}

	pdata->proc_objs[pdata->count] = sink;
	pdata->is_sink_connected = true;
	pdata->count++;
}

static int pipeline_stream_data(pipeline_obj * const obj)
{
	pipeline_private_data * const pdata =
	       		(pipeline_private_data * const ) obj->pdata;
	message_obj *msg = &pdata->msg;
	int rc = 0;


	if (pdata->count < 2) {
		ERROR("The pipeline is incomplete\n");
		return -1;
	}

	for (unsigned int i=0; i<(pdata->count-1); i++) {
		DEBUG("Data incoming from element\n");
		if (pdata->proc_objs[i]->data_out(pdata->proc_objs[i], msg) < 0) {
			rc = -1;
			break;
		}

		DEBUG("Data passed to the next element\n");
		if (pdata->proc_objs[i+1]->data_in(pdata->proc_objs[i+1], msg) < 0) {
			rc = -1;
			break;
		}
	}

	return rc;
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

	if (message_init(&pdata->msg)) {
		return -1;
	}


	memset(obj, 0, sizeof(*obj));
	obj->attach_src = pipeline_attach_src;
	obj->attach_sink = pipeline_attach_sink;
	obj->attach_proc = pipeline_attach_proc;
	obj->stream_data = pipeline_stream_data;

	obj->pdata = pdata;
	pdata->is_used = true;
	DEBUG("Pipeline initialized\n");

	return 0;
}
