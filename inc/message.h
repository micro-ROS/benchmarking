#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdlib.h>

typedef struct message_obj_st message_obj;

typedef size_t (*message_write_cb	)
		(message_obj * const obj, char * const buffer, size_t len);

typedef size_t (*message_read_cb	)
		(message_obj * const obj, char ** const buffer);

typedef size_t (*message_length_cb	)
		(message_obj * const obj);

typedef size_t (*message_size_cb	)
		(message_obj * const obj);

typedef size_t (*message_cpy_cb )
		(message_obj * const msg, message_obj * restrict src);

typedef char *(*message_ptr_cb ) (message_obj * const obj);

struct message_obj_st {
	/** Read from a message's buffer */
	message_read_cb		read;
	/** Write to a message's buffer, this destroy data 
	 *  within the buffer 	 **/
	message_write_cb	write;
	/** Amount of data written */
	message_length_cb	length;
	/** Total size of the underliying buffer */
	message_size_cb		total_len;
	/** Copy to one message to another */
	message_cpy_cb		cpy;
	/** Get the buffer pointer */
	message_ptr_cb		ptr;
	/** Internal data */
	void			*pdata;
};

int message_init(message_obj * const obj);
int message_fini(message_obj * const obj);

#endif /* __MESSAGE_H__ */
