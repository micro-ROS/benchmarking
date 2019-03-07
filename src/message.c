/*****************************************************************
 * file: message.c
 * author: Alexandre Malki <amalki@piap.pl>
 * brief:  This is the source of the message_object. A message object  is
 * 		as an object provide more information about a a buffer. This
 * 		object holds information such as the size of the internal buffer
 * 		the number of byte written to it (usable). And some internal
 * 		method that allow to write/append/read and access the raw pointer.
 *****************************************************************/
#include <message.h>
#include <config.h>

#include <debug.h>
#include <common-macros.h>

#include <stdbool.h>
#include <string.h>


/* For now everything is static, do not want to look for leaks right know *?
#include <malloc.c> */

/** Internal message_object structure */
typedef struct {
/* TODO create buffer size */
#ifdef MESSAGE_DYNAMIC
#error "Currently dynamic message passing is not implemented"
#else
	/** Underlying buffer object */
	char buffer[MESSAGE_BUFFER_SZ_MAX];
#endif
	/** Length of usable bytes (written) by the application */
	size_t length;
	/** Total length of the buffer */
	size_t total_len;
	/** Information if the buffer is used by a message_obj */
	bool is_used;
} message_priv_data;

/* TODO In config file create message instance */
static message_priv_data message_instances[MESSAGE_NSTANCES_CNT_MAX];


/**
 * @brief Default copy callback
 * */
static size_t message_cpy_default(message_obj * restrict obj,
			       message_obj * restrict src)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/**
 * @brief Default ptr callback
 * */
static char *message_ptr_default(const message_obj * const obj)
{
	WARNING("Message Object not initialized\n");
	return NULL;
}

/**
 * @brief Default length callback
 * */
static size_t message_length_default(const message_obj * const obj)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/**
 * @brief Default size callback
 * */
static size_t message_totlen_default(const message_obj * const obj)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/**
 * @brief Default write callback
 * */
static size_t message_write_default(message_obj * const obj,
				 char * const buffer, size_t len)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/**
 * @brief append data to the end of buffer
 */
static size_t message_append_default(message_obj * const obj,
				 char * const buffer, size_t len)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/**
 * @brief Default read callback
 * */
static size_t message_read_default(message_obj * const obj,
				  char * const buffer, size_t len)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/**
 * @brief Writing len data to message_obj's buffer from the buffer.
 * 		This funcction will make sure that the size written
 * 		does not overflow the message_obj's buffer.
 * @param obj Message_objec to write to.
 * @param buffer Buffer holding data.
 * @param len Length to write.
 * @return the len if enough space is available in the buffer, or
 * 		the maxium bytes written into the message_obj's 
 * 		buffer
 */
static size_t message_write_buffer(message_obj * const obj,
				 char * const buffer, size_t len)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	size_t min = obj->total_len(obj) < len ?
					obj->total_len(obj) : len;

	memcpy(obj->ptr(obj), buffer, min);
	pdata->length = min;

	return min;
}

/**
 * @brief Append len bytes of data to the message_obj's buffer. This
 * 		function will make sure that the size written does not
 * 		overflow the message_obj buffer.
 * @param obj Message_obj to append data to.
 * @param buffer The data to append to the message_obj
 * @param len The size in byte to append to the buffer.
 * @return  the actuall amout of byte written. If the size will overflow,
 * 		then the function will return -1
 */
static size_t message_append_buffer(message_obj * const obj,
				 char * const buffer, size_t len)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	size_t pos = obj->length(obj);

	if ((len + pos) > obj->total_len(obj)) {
		/* TODO memmove in dynamic allocation */
		ERROR("Not enought space in the buffer,"
			 "(len/max) %ld/%ld\n",
				len+pos, obj->total_len(obj));
		return -1;
	}

	memcpy(&obj->ptr(obj)[pos], buffer, len);
	obj->set_length(obj, len + pos);

	return len;
}

/**
 * @brief Read (copy) the content of the underlying message_obj's buffer to
 * 		an external buffer. This function will make sure that the
 * 		size read stay withing the message_obj's buffer boundaries.
 * @param obj Message object to read from.
 * @param buffer External buffer that will hold the data, should be big enough
 * 		to received the data read.
 * @param len Request length to read.
 * @return The number of bytes actually read could be less than the requested
 * 		size.
 */
static size_t message_read_buffer(message_obj * const obj, char * const buffer,
				  size_t len)
{
	size_t min = obj->length(obj) > len ?
					obj->length(obj) : len;

	memcpy(buffer, obj->ptr(obj), min);

	return min;
}

/**
 * @brief Return the number of byte written using write/append to the
 * 		message_obj's buffer or the value set by using set_length method.
 * @param obj Messsage object.
 * @return The number of usable byte in the buffer.
 */
static size_t message_length_buffer(const message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	return pdata->length;
}

/**
 * @brief This function will set the length of underlying buffer. This function
 * 		is used in case of a writing to the buffer directly instead
 * 		of using the write methods.
 * @param obj Message object.
 * @param len The size to set.
 */
static void message_set_length_buffer(const message_obj * const obj, size_t len)
{
	message_priv_data *pdata = (message_priv_data *) obj->pdata;
	if (len > pdata->total_len)
		WARNING("set length bigger than the actual buffer allocating %ld/%ld\n", len, pdata->total_len);

	pdata->length = len;
}

/**
 * @brief Return the total length usable of the underlying buffer attached
 * 		to a specific message object.
 * @param obj Message object.
 * @return The total length of the the message object's buffer. 
 */
static size_t message_totlen_buffer(const message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	return pdata->total_len;
}

/**
 * @brief To retrieve the pointer on the underlying buffer that the
 * 		message holds.
 * @param obj message object containing the buffer.
 * @return a pointer on the buffer
 */
static char *message_ptr_buffer(const message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	return pdata->buffer;
}

/**
 * @brief Copy a message from src into obj. If obj's total_len is lower
 *  		than the the total_len of src, then only the total_len's obj
 * 		is copied.
 * @param obj Message object to copy to.
 * @param sr Message object to copy from.
 * @return the number of byte copied.
 */
static size_t message_cpy_buffer(message_obj * const obj,
			         message_obj * const src)
{
	return message_write_buffer(obj, src->ptr(src), src->length(src));
}

/**
 * @brief Retrieve an unused private instance of message.
 * @return An available private instance of message.
 */
static message_priv_data *message_get_free_instance(void)
{
#ifdef MESSAGE_DYNAMIC
	/*TODO if needed do malloc */
	return NULL;
#else
	unsigned int i = 0;
	message_priv_data *pdata = NULL;

	while (i < ARRAY_SIZE(message_instances)) {
		if (!message_instances[i].is_used) {
			pdata = &message_instances[i];
			pdata->is_used = true;
			break;
		}
		i++;
	}

	return pdata;
#endif
}

/**
 * @brief Set default values to the private instance.
 * @param pdata private instance.
 * @return 0 upon success.
 */
int message_internal_init(message_priv_data *pdata)
{
	pdata->length = 0;
	pdata->total_len = MESSAGE_BUFFER_SZ_MAX;

	return 0;
}

int message_init(message_obj * const obj)
{
	message_priv_data *pdata = NULL;
	if (!(pdata = message_get_free_instance())) {
		ERROR("Could not get a free message instance\n");
		goto no_free_instance;
	}

	if (message_internal_init(pdata)){
		goto internal_init_failed;
	}

	obj->pdata = (void *) pdata;
	/** For now we use buffer only */
	obj->read = message_read_buffer;
	obj->write = message_write_buffer;
	obj->append = message_append_buffer;
	obj->length = message_length_buffer;
	obj->set_length = message_set_length_buffer;
	obj->total_len = message_totlen_buffer;
	obj->cpy = message_cpy_buffer;
	obj->ptr = message_ptr_buffer;

	return 0;
internal_init_failed:
no_free_instance:
	return -1;
}

int message_fini(message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *) obj->pdata;

	if (!pdata) {
		WARNING("Message NULL\n");
		return -1;
	}

	if (!pdata->is_used) {
		WARNING("Message already freed\n");
		return -1;
	}

	obj->read = message_read_default;
	obj->write = message_write_default;
	obj->append = message_append_default;
	obj->length = message_length_default;
	obj->total_len = message_totlen_default;
	obj->cpy = message_cpy_default;
	obj->ptr = message_ptr_default;
	obj->pdata = NULL;

	memset(pdata, 0, sizeof(*pdata));
	return 0;
}
