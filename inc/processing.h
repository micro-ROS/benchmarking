#ifndef __PROCESSING_H__
#define __PROCESSING_H__

#include <message.h>

#include <stdbool.h>

typedef struct processing_obj_st processing_obj;

typedef size_t (*processing_data_in_cb	) (processing_obj * const obj, message_obj * const msg);
typedef size_t (*processing_data_out_cb	) (processing_obj * const obj, message_obj * const msg);

struct processing_obj_st {
	/** Callback to register to receive data */
	processing_data_in_cb 		data_in;

	/** Callback to register to receive data */
	processing_data_out_cb 		data_out;

	bool				req_end;
};

int processing_init(processing_obj * const obj);
int processing_fini(processing_obj * const obj);

#endif /* __PROCESSING_H__ */
