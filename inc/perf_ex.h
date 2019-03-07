/*****************************************************************
 * file: perf_ex.h
 * author: Alexandre Malki <amalki@piap.pl>
 * brief: This is the header of the performance processing object.
 *		more details in the source file.
 *****************************************************************/

#ifndef __PERF_EX_H__
#define __PERF_EX_H__

#include <processing.h>

typedef struct perf_ex_obj_st perf_ex_obj;

typedef int (*perf_ex_set_elf_gbl_config_cb) (perf_ex_obj * const obj);
typedef int (*perf_ex_set_elf_cb) (perf_ex_obj * const obj,
					const char * const path);
typedef int (*perf_ex_set_tc_gbl_config_cb) (perf_ex_obj * const obj);
typedef int (*perf_ex_set_tc_cb) (perf_ex_obj * const obj,
					const char * const path);

/** This structure inherits from the processing object */
struct perf_ex_obj_st {
	/** Processing abstraction object */
	processing_obj	proc_obj;
	/** Method setting the toolchain path using config object*/
	perf_ex_set_elf_gbl_config_cb set_elf_gbl_config;
	/** Method setting the toolchain path using the string char*/
	perf_ex_set_elf_cb set_elf;
	/** Method setting the toolchain path using config object*/
	perf_ex_set_tc_gbl_config_cb set_tc_gbl_config;
	/** Method setting the toolchain path using the string char*/
	perf_ex_set_tc_cb set_tc;
	/** Internal data structure */
	void		*pdata;
};

/**
 * @brief This function initialize the data for a selected object. It does
 * 		not set the path to the toolchain and the elf file.
 * @param The perf_ex_obj object.
 * @return 0 if sucessfully initialized the object, -1 othewise.
 */
int perf_ex_init(perf_ex_obj * const obj);

/**
 * @brief This function de-initialize the data for a selected object 
 * @param The perf_ex_obj object.
 * @return 0 if sucessfully de-initialized the object, -1 othewise.
 */
int perf_ex_fini(perf_ex_obj * const obj);

#endif /* __PERF_EX_H__ */
