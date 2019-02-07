#ifndef __STREAM_H__
#define __STREAM_H__

#include <message.h>
#include <processing.h>

typedef struct stream_obj_st stream_obj;
	int (*write) (char *buffer, size_t size);

/** Open the stream, this functionis virtual, the inherited object
	shall actually open the device/file */
int (*stream_src_open		) (stream_obj *obj); 

/** Close the stream, this functionis virtual, the inherited object
	shall actually close the device/file */
int (*stream_src_close		) (stream_obj *obj);

/** Provide the maximum attachable output for a specific stream */
int (*stream_src_max_attachable	) (stream_obj *obj);

/** This processing output is just a kind of sink.
	multiple output can be attached.
*/
int (*stream_src_attach_output	) (stream_obj *obj, processing_obj *proc);


struct stream_src_obj_st {
	stream_src_open			open;
	stream_src_close		close;
	stream_src_attach_output	attach_output;
	stream_src_max_attachable	max_attachable;
};      

#endif /* __STREAM_H__ */
