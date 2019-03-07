/*****************************************************************
 * @file file.h
 * @brief	Header of the file objects, inhererting from process.
 * @author	Alexandre Malki <amalki@piap.pl>
 *****************************************************************/
#ifndef __FILE_H__
#define __FILE_H__

#include <processing.h>

enum file_mode {
	/** Do not truncate and simply read */
	FILE_RDONLY,
	/** Truncate file */
	FILE_WRONLY,
};

typedef struct file_obj_st file_obj;

/** This callback will set the path and access mode to the file */
typedef int (*file_set_path_cb)(file_obj * const obj,
				 const char * const path, 
			     	enum file_mode);

/** This callback will open the file set to */
typedef int (*file_init_cb)(file_obj * const obj);

/** This callback will close the file set to */
typedef int (*file_fini_cb)(file_obj * const obj);


/** This structure inherits from the processing object */
struct  file_obj_st {
	/** Processing abstraction object */
	processing_obj proc_obj;
	/** callback to set the file path */
	file_set_path_cb 	file_set_path;
	/** Callback to init internal file specific info and open file */
	file_init_cb		file_init;
	/** Callback to de-init internal file specific info and close file */
	file_fini_cb		file_fini;
	/** Internal data structure */
	void *pdata;
};

/**
 * @brief This function is initializing the internal file object.
 * @param obj file object to initilize.
 * @return 0 upon success, -1 otherwize.
 */
int file_init(file_obj * const obj);

/**
 * @brief This function is de-initializing the internal file object.
 * @param obj file object to de-initilize.
 * @return 0 upon success, -1 otherwize.
 */
int file_clean(file_obj * const obj);

#endif /* __FILE_H__ */
