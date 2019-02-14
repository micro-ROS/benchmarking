#ifndef __DECODER_SWO_H__
#define __DECODER_SWO_H__

#include <processing.h>

typedef struct decoder_swo_obj_st decoder_swo_obj;  

struct decoder_swo_obj_st {
	processing_obj 	proc_obj;

	void		*pdata;
};

int decoder_swo_init(decoder_swo_obj * const obj);
int decoder_swo_fini(decoder_swo_obj * const obj);

#endif /* __DECODER_SWO_H__ */
