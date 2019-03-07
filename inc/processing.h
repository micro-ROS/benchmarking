/*****************************************************************
 * file: processing.h
 * author: Alexandre Malki <amalki@piap.pl>
 * brief: This is the processing_obj's header file. More information in the
 * 		source file processing.c .
 */
#ifndef __PROCESSING_H__
#define __PROCESSING_H__

#include <message.h>

#include <stdbool.h>

typedef struct processing_obj_st processing_obj;

typedef size_t (*processing_data_in_cb)
			(processing_obj * const obj, message_obj * const msg);
typedef size_t (*processing_data_out_cb)
			 (processing_obj * const obj, message_obj * const msg);
typedef int (*processing_reg_reader_cb)
			 (processing_obj * const obj, processing_obj * const el);
typedef int (*processing_execute_out_cb) (processing_obj * const obj);

struct processing_obj_st {
	/** Callback to register to receive data */
	processing_data_in_cb 		data_in;
	/** Callback to register to receive data */
	processing_data_out_cb 		data_out;
	/** callback to register a processing element 
	 *  to the a list of readers.
	 */
	processing_reg_reader_cb	register_element;
	/** This callback will execute data_out function and 
	 *  call its child(ren) with a copy of the data produce.
	 *  this function is recurisive and execute one element at time
	 */
	processing_execute_out_cb	execute_out;
	/** Internal message */
	message_obj			msg;
	/** bool set by the pipeline obj to request an end */
	bool				req_end;
	/** This will happen when the one processing element needs to send
	 * more data than the tranmit buffer can hold.
	 */
	bool				req_send_more;
	/** Child object procssing object */
	processing_obj			*child;
	/** Same level processing objects */
	processing_obj			*next;
	/** Name of the object "Defaulting to Unknown */
	char				*name;
	/** Internal data */
	void 				*pdata;
};

/**
 * @brief This initialize the processing object. All object inheriting from the 
 * 		processing object have to call this function during their
 * 		initialization.
 * @param obj processing object to initialize.
 * @return 0 upon success, -1 otherwise.
 */
int processing_init(processing_obj * const obj);

/**
 * @brief This initialize the processing object. All object inheriting from the 
 * 		processing object have to call this function during their
 * 		de-initialization.
 * @param obj processing object to de-initialize.
 * @return 0 upon success, -1 otherwise.
 */
int processing_fini(processing_obj * const obj);

#endif /* __PROCESSING_H__ */
