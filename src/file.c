/*****************************************************************
 * @file file.c
 * @author	Alexandre Malki <amalki@piap.pl>
 * @brief	Source file of containing methods and private data related to the
 *		file processing object. The file object inherits the 
 *		processing object. Currenlty the file object only write
 *		and does not read.
 *****************************************************************/
#include <debug.h>
#include <config.h>
#include <common-macros.h>
#include <file.h>

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/** Default wronly flags: truncate, create if not exists, write only */
#define FILE_DEFAULT_WR_FLAGS 	( O_CREAT | O_TRUNC | O_RDWR)

/** Default wronly acces mode: user can read and write */
#define FILE_DEFAULT_WR_AC_MODE (S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH )

/** Default read flags: only read */
#define FILE_DEFAULT_RD_FLAGS (O_RDONLY)

/* TODO create a common interface file and uart */

/** This structure contains information regarding the file being openned */
typedef struct {
	/** Path to the file */
	char file_path[STRING_MAX_LENGTH];
	/** file descriptor */
	int fd;
	enum file_mode mode;
	/** is the file open (callback obj->file_init) */
	bool is_open;
	/** is the file used by a processing element (function file_init) */
	bool is_used;

} file_priv_data;

/**
 * This is the private file instances. Their could be maxium FILE_COUNT_DATA
 * instances 
 */
static file_priv_data file_pdata[FILE_COUNT_MAX];

/**
 * @brief This function assignes a free instance to the file_obj object.
 * @param obj File object.
 * @return 0 upon success, -1 otherwise.
 */
static int file_get_instance(file_obj * const file)
{
	unsigned int i;
	int rc = -1;

	/* TODO : Might be some concurence here */
	for (i=0; i<ARRAY_SIZE(file_pdata); i++) {
		if (!file_pdata[i].is_used) {
			file->pdata = (void *) &file_pdata[i];
			file_pdata[i].is_used = true;
			rc = 0;
			break;
		}
	}

	return rc;
}

/**
 * @brief This function frees the instance linked to the file_obj object.
 * @param obj File object.
 * @return 0 upon success, -1 otherwise.
 */
static int file_free_instance(file_obj * const obj)
{
	file_priv_data *pdata = (file_priv_data *) obj->pdata;

	if (!pdata->is_used) {
		ERROR("The object was not initialized corretly\n");
		return -1;
	}

	memset(pdata, 0, sizeof(*pdata));
	obj->pdata = NULL;

	return 0;
}

/**
 * @brief This function is assigned to the file_set_path callback of the file_obj.
 * @param obj File object.
 * @param path Path to the file to open.
 * @param mode Mode to open file file, currently only FILE_WRONLY or FILE_RDONLY.
 * @return 0 upon success, -1 otherwise.
 */
static int file_set_path(file_obj * const obj, const char * const path,
			 enum file_mode mode)
{
	file_priv_data *pdata = (file_priv_data *) obj->pdata;

	if (!pdata->is_used) {
		ERROR("The object was not initialized corretly\n");
		return -1;
	}

	strncpy(pdata->file_path, path, sizeof(pdata->file_path) - 1);
	pdata->mode = mode;

	return 0;
}


/**
 * @brief This function is assigned to the file init callback of the file_obj.
 * @param obj File object.
 * @return 0 upon success, -1 otherwise.
 */
static int file_open(file_obj * const obj)
{
	file_priv_data *pdata = (file_priv_data *) obj->pdata;
	int rc = -1;

	if (!pdata->is_used) {
		ERROR("The object was not initialized corretly\n");
		return -1;
	}

	if (pdata->is_open) {
		ERROR("The object is already openned\n");
		return -1;
	}

	switch (pdata->mode) {
	case FILE_WRONLY:
		rc = open(pdata->file_path, FILE_DEFAULT_WR_FLAGS,
			  FILE_DEFAULT_WR_AC_MODE);
	break;
	case FILE_RDONLY:
		rc = open(pdata->file_path, FILE_DEFAULT_RD_FLAGS);
	break;
	default:
		ERROR("Not handled open mode\n");
		return -1;
	}

	if (rc < 0) {
		ERROR("While opening file %s\n", strerror(errno));
		return -1;
	}

	pdata->is_open = true;
	pdata->fd = rc;

	return 0;
}

/**
 * @brief  This function is assigned to the callback file_fini of the file_obj.
 * @param obj object reference holding the file descriptor to close.
 * @return 0 upon success, -1 otherwise.
 */
static int file_close(file_obj * const obj)
{
	file_priv_data *pdata = (file_priv_data *) obj->pdata;

	if (!pdata->is_used) {
		ERROR("The object was not initialized corretly\n");
		return -1;
	}

	if (!pdata->is_open) {
		ERROR("The object is already openned\n");
		return -1;
	}

	close(pdata->fd);
	pdata->is_open = false;

	return 0;
}

/**
 * @brief This callback will is the implemenatation of the virtual function
 * 		data_in.
 * @param obj Processing obj abstraction.
 * @param msg message containing the information. The message buffer shall
 *		contains the data to write.
 * @return Number of bytes written, -1 if there is an error.
 */
static size_t file_write(processing_obj * const obj, message_obj * const msg)
{
	file_obj *f_obj = (file_obj *) obj;
	file_priv_data *pdata = (file_priv_data *) f_obj->pdata;
	char *buf = msg->ptr(msg);
	size_t length = msg->length(msg);
	size_t n, written = 0;

	while ((n = write(pdata->fd, &buf[written], length - written)) >= 0) {
		written += n;
		if (written >= length)
			break;
	}

	return written;
}

int file_init(file_obj * const obj)
{
	processing_obj *proc_obj = (processing_obj *) obj;

	memset(obj, 0, sizeof(*obj));
	if (file_get_instance(obj)) {
		goto free_instance_failed;
	}

	obj->file_init = file_open;
	obj->file_fini = file_close;
	obj->file_set_path = file_set_path;

	if (processing_init(proc_obj)) {
		goto processing_init_failed;
	}

	proc_obj->data_in = file_write;
	/** TODO File read for now leave the normal one */

	return 0;
processing_init_failed:
free_instance_failed:
	file_free_instance(obj);
	return -1;
}

int file_clean(file_obj * const obj)
{
	processing_obj *proc_obj = (processing_obj *) obj;
	file_priv_data *pdata = (file_priv_data *) obj->pdata;

	if (!pdata->is_used) {
		ERROR("Cannot clean uninitilized \n");
		return -1;
	}

	if (pdata->is_open) {
		ERROR("File attached still open\n");
		return -1;
	}

	if (processing_fini(proc_obj)) {
		return -1;
	}

	return file_free_instance(obj);
}
