
/*****************************************************************
 * file: perf_ex
 * author: Alexandre Malki <alexandremalki@gmail.com>
 * brief: This is file is autogenrated
 *
 *****************************************************************/

#ifndef __PERF_EX_H__
#define __PERF_EX_H__

#include <processing.h>

typedef struct perf_ex_obj_st perf_ex_obj ;

typedef int (*perf_ex_set_tc_gbl_config_cb) (perf_ex_obj * const obj);
typedef int (*perf_ex_set_tc_cb) (perf_ex_obj * const obj, const char * const path);

struct perf_ex_obj_st {
	processing_obj	proc_obj;
	perf_ex_set_tc_gbl_config_cb set_tc_gbl_config;
	perf_ex_set_tc_cb set_tc;

	void		*pdata;
};

int perf_ex_init(perf_ex_obj *obj);
int perf_ex_fini(perf_ex_obj *obj);

#endif /* __PERF_EX_H__ */
