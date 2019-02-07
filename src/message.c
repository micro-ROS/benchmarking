#include <message.h>
#include <config.h>

#include <debug.h>
#include <common-macros.h>

#include <stdbool.h>
#include <string.h>


/* For now everything is static, do not want to look for leaks right know *?
#include <malloc.c> */

typedef struct {
/* TODO create buffer size */
#ifdef MESSAGE_DYNAMIC
#error "Currently dynamic message passing is not implemented"
#else
	char buffer[MESSAGE_BUFFER_SZ_MAX];
#endif
	size_t length;
	size_t total_len;
	bool is_used;
} message_priv_data;

/* TODO In config file create message instance */
static message_priv_data message_instances[MESSAGE_NSTANCES_CNT_MAX];


/** 
 * \brief Default copy callback 
 * */
static size_t message_cpy_default(message_obj * restrict obj,
			       message_obj * restrict src)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/** 
 * \brief Default ptr callback 
 * */
static char *message_ptr_default(message_obj * const obj)
{
	WARNING("Message Object not initialized\n");
	return NULL;
}

/** 
 * \brief Default length callback 
 * */
static size_t message_length_default(message_obj * const obj)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/** 
 * \brief Default size callback 
 * */
static size_t message_totlen_default(message_obj * const obj)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/** 
 * \brief Default write callback 
 * */
static size_t message_write_default(message_obj * const obj,
				 char * const buffer, size_t len)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

/** 
 * \brief Default read callback 
 * */
static size_t message_read_default(message_obj * const obj,
	       			  char ** const buffer)
{
	WARNING("Message Object not initialized\n");
	return -1;
}

static size_t message_write_buffer(message_obj * const obj,
				 char * const buffer, size_t len)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	size_t min = obj->length(obj) > len ? 
					obj->length(obj) : len;

	memcpy(obj->ptr(obj), buffer, min);
	pdata->length = min;

	return min;
}

static size_t message_read_buffer(message_obj * const obj, char ** const buffer)
{
	*buffer = obj->ptr(obj);
	return obj->length(obj);
}

static size_t message_length_buffer(message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	return pdata->length;
}

static size_t message_totlen_buffer(message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	return pdata->total_len;
}

static char *message_ptr_buffer(message_obj * const obj)
{
	message_priv_data *pdata = (message_priv_data *)obj->pdata;
	return pdata->buffer;
}

/**
 * \brief Copy a message from src into obj. If obj's total_len is lower
 *  		than the the total_len of src, then only the total_len's obj
 * 		is copied. 
 */
static size_t message_cpy_buffer(message_obj * const obj,
			       message_obj * const src)
{
	return message_write_buffer(obj, src->ptr(src), src->length(src));
}

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
	obj->length = message_length_buffer;
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
	obj->length = message_length_default;
	obj->total_len = message_totlen_default;
	obj->cpy = message_cpy_default;
	obj->ptr = message_ptr_default;
	obj->pdata = NULL;

	memset(pdata, 0, sizeof(*pdata));
	return 0;
}
