#ifndef __PROCESSING_H__
#define __PROCESSING_H__

#include <message.h>

typedef struct processing_obj_st processing_obj;

typedef size_t (*processing_data_in_cb	) (processing_obj * const obj, message_obj * const msg);
typedef size_t (*processing_data_out_cb	) (processing_obj * const obj, message_obj * const msg);

struct processing_obj_st {
	processing_data_in_cb 		data_in;
	processing_data_out_cb 		data_out;
};

int processing_init(processing_obj * const obj);
int processing_fini(processing_obj * const obj);

#endif /* __PROCESSING_H__ */
