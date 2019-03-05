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

struct pipeline_obj_st {
	pipeline_attach_src_cb   attach_src;
	pipeline_attach_proc_cb  attach_proc;
	pipeline_stream_data_cb  stream_data;
	pipeline_is_stopped_cb	 is_stopped;

	void *pdata;
};

int pipeline_init(pipeline_obj *obj);
void pipeline_set_end_all(void);

#endif /* __PIPELINE_H__ */
