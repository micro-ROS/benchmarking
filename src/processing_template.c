/*****************************************************************
 * file: @@processingname@@
 * author: Alexandre Malki <alexandremalki@gmail.com>
 * brief: This file is autogenerated
 *
 *****************************************************************/

#include <message.h>
#include <@@processingname@@.h>

typedef struct {
	bool is_init;
} @@processingname@@_private_data;

static @@processingname@@_private_data @@processingname@@_priv_data;

/**
 * \brief TODO
 */
static size_t 
@@processingname@@_data_in(processing_obj * const proc_obj, message_obj *const msg)
{
	@@processingname@@_obj *obj = (@@processingname@@_obj *) proc_obj;
	@@processingname@@_private_data	*pdata = 
				(@@processingname@@_private_data *) obj->pdata;

	return 0;
}

/**
 * \brief TODO
 */
static size_t
@@processingname@@_data_out(processing_obj * const proc_obj, message_obj *const msg)
{
	@@processingname@@_obj *obj = (@@processingname@@_obj *) proc_obj;
	@@processingname@@_private_data	*pdata = 
				(@@processingname@@_private_data *) obj->pdata;

	return 0;
}

/**
 * \brief TODO
 */
int @@processingname@@_init(@@processingname@@_obj * const obj)
{
	return 0;
}

/**
 * \brief TODO
 */
int @@processingname@@_fini(@@processingname@@_obj * const obj)
{
	return 0;
}
