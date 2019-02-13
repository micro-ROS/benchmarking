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
				message_obj * const path, 
			     	enum file_mode);

/** This callback will open the file set to */
typedef int (*file_init_cb)(file_obj * const obj);

/** This callback will close the file set to */
typedef int (*file_fini_cb)(file_obj * const obj);


struct  file_obj_st {
	processing_obj proc_obj;

	file_set_path_cb 	file_set_path;
	file_init_cb		file_init; 
	file_fini_cb		file_fini; 

	void *pdata;
};

int file_init();
int file_clean();

#endif /* __FILE_H__ */
