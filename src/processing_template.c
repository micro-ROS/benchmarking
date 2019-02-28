/*****************************************************************
 * file: @@processingname@@
 * author: Alexandre Malki <alexandremalki@gmail.com>
 * brief: This is file is autogenrated
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
static int
@@processingname@@_data_in(@@processingname@@_obj *obj, message_obj *const msg)
{
	@@processingname@@_private_data	*pdata = 
				(@@processingname@@_private_data *) obj->pdata;

	return 0;
}

/**
 * \brief TODO
 */
static int
@@processingname@@_data_out(@@processingname@@_obj *obj, message_obj *const msg)
{
	@@processingname@@_private_data	*pdata = 
				(@@processingname@@_private_data *) obj->pdata;

	return 0;
}

/**
 * \brief TODO
 */
int @@processingname@@_init(@@processingname@@_obj *obj)
{
	return 0;
}

/**
 * \brief TODO
 */
int @@processingname@@_fini(@@processingname@@_obj *obj)
{
	return 0;
}
