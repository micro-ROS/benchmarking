
/*****************************************************************
 * file: @@processingname@@
 * author: Alexandre Malki <alexandremalki@gmail.com>
 * brief: This is file is autogenrated
 *
 *****************************************************************/

#ifndef __@@PROCESSINGNAME@@_H__
#define __@@PROCESSINGNAME@@_H__

#include <processing.h>

typedef struct @@processingname@@_obj_st @@processingname@@_obj ;

struct @@processingname@@_obj_st {
	processing_obj	proc_obj;

	void		*pdata;
};

int @@processingname@@_init(@@processingname@@_obj *obj);
int @@processingname@@_fini(@@processingname@@_obj *obj);

#endif /* __@@PROCESSINGNAME@@_H__ */
