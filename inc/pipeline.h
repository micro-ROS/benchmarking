#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <processing.h>

typedef struct pipeline_obj_st pipeline_obj;

typedef int (*pipeline_attach_src_cb)(pipeline_obj * const obj, 
				      processing_obj * const src);

typedef int (*pipeline_attach_proc_cb)(pipeline_obj * const obj, 
				       processing_obj * const src,
	       			       processing_obj * const dst);

typedef int (*pipeline_attach_sink_cb)(pipeline_obj * const obj, 
				       processing_obj * const sink);

typedef int (*pipeline_stream_data_cb)(pipeline_obj *obj);

struct pipeline_obj_st {
	pipeline_attach_src_cb   attach_src;
	pipeline_attach_proc_cb  attach_proc;
	pipeline_attach_sink_cb  attach_sink;
	pipeline_stream_data_cb  stream_data;

	void *pdata;
};

int pipeline_init(pipeline_obj *obj);

#endif /* __PIPELINE_H__ */
