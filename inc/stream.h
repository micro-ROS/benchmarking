#ifndef __LIBCOM_H__
#define __LIBCOM_H__

#include <message.h>
#include <processing.h>

typedef struct stream_obj_st stream_obj;

typedef int (*processing_read_cb		) (stream_obj *obj,
	      					   message_obj *msg);

typedef int (*processing_write_cb		) (stream_obj *obj,
						   message_obj *msg);

typedef int (*processing_open_cb		) (stream_obj * obj); 

typedef int (*processing_close_cb		) (stream_obj *obj);

typedef int (*processing_attach_output_cb	) (stream_obj *obj, processing_obj *proc);

typedef int (*processing_run_cb				) (stream_obj *obj);

struct stream_obj_st {
	processing_read_cb		read;
 	processing_write_cb		write;
	processing_open_cb		open;
	processing_close_cb		close;
	processing_attach_output_cb	attach_output;
	processing_run_cb		run;

	void *pdata;
};



#endif /* __LIBCOM_H__ */
