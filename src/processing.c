#include <processing.h>
#include <debug.h>

#include <stdbool.h>

typedef struct {
	bool is_init;
} processing_priv_data;



static int processing_data_in_default(processing_obj *obj, message_obj *msg)
{
	WARNING("No callback defined yet\n");
	return -1;
}


static int processing_data_out_default(processing_obj *obj, message_obj *msg)
{
	WARNING("No callback defined yet\n");
	return -1;
}


int processing_init(processing_obj *obj)
{
	DEBUG("Initializing object %p\n", obj);

	obj->data_in = processing_data_in_default;
	obj->data_out = processing_data_out_default;

	return 0;
}


int processing_fini(processing_obj *obj)
{
	DEBUG("Denitializing object %p\n", obj);

	obj->data_in = processing_data_in_default;
	obj->data_out = processing_data_out_default;

	return 0;
}
