/*****************************************************************
 * file: message.h
 * author: Alexandre Malki <amalki@piap.pl>
 * brief:  This is the header of the message_object, more information
 * 		about the the message_object in source file message.c.
 *****************************************************************/

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h>

typedef struct message_obj_st message_obj;

typedef size_t (*message_write_cb	)
		(message_obj * const obj, char * const buffer, size_t len);

typedef size_t (*message_append_cb	)
		(message_obj * const obj, char * const buffer, size_t len);

typedef size_t (*message_read_cb	)
		(message_obj * const obj, char * const buffer, size_t len);

typedef size_t (*message_length_cb	)
		(const message_obj * const obj);

typedef void (*message_set_length_cb	)
		(const message_obj * const obj, size_t len);

typedef size_t (*message_size_cb	)
		(const message_obj * const obj);

typedef size_t (*message_cpy_cb )
		(message_obj * const msg, message_obj * restrict src);


typedef char *(*message_ptr_cb ) (const message_obj * const obj);

struct message_obj_st {
	/** Read from a message's buffer */
	message_read_cb		read;
	/** Write to a message's buffer, this destroy data 
	 *  within the buffer 	 **/
	message_write_cb	write;
	/** Append data to the end of the buffer */
	message_append_cb	append;
	/** Amount of data written */
	message_length_cb	length;
	/** Set length when accessing raw */
	message_set_length_cb	set_length;
	/** Total size of the underliying buffer */
	message_size_cb		total_len;
	/** Copy to one message to another */
	message_cpy_cb		cpy;
	/** Get the buffer pointer */
	message_ptr_cb		ptr;
	/** Internal data */
	void			*pdata;
};

/**
 * @brief Initialize the message object, and assign a buffer to it.
 * @param obj This is the message object to initilized. Must be
 * 		allocated before hand.
 * @return 0 upon success, -1 otherwise.
 */
int message_init(message_obj * const obj);

/**
 * @brief Initialize the message object, and remote the attached
 * 		 a buffer to it.
 * @param obj This is the message object to initilized. Must be
 * 		initilized before before hand.
 * @return 0 upon success, -1 otherwise.
 */
int message_fini(message_obj * const obj);

#endif /* __MESSAGE_H__ */
