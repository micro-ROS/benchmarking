#include <processing.h>
#include <debug.h>
#include <string.h>

typedef struct {
	bool is_init;
} processing_priv_data;

static size_t processing_data_in_default(processing_obj * const obj,
				      message_obj * const msg)
{
	WARNING("No callback defined yet\n");
	return 0;
}

static size_t processing_data_out_default(processing_obj * const obj,
				          message_obj * const msg)
{
	WARNING("No callback defined yet\n");
	return 0;
}

static int processing_reg_readder_default(processing_obj * const obj,
				             processing_obj * const el)
{
	WARNING("No callback defined yet\n");
	return 0;
}

static int processing_reg_reader(processing_obj * const obj,
				 processing_obj * const el)
{
	processing_obj *next_child, *prev_child = NULL;

	if (obj->child) {
		next_child = obj->child;
		while (next_child) {
			prev_child = next_child;
			next_child = next_child->next;
		}

		prev_child->next = el;
	} else {
		obj->child = el;
	}

	return 0;
}


static int processing_execute_out_default(processing_obj * const obj)
{
	WARNING("Not initiliazed\n");
	return -1;
}

static int processing_execute_out(processing_obj * const obj)
{
	processing_obj *next_child;
	message_obj *msg = &obj->msg;

	msg->set_length(msg, 0);
	memset(msg->ptr(msg), 0, msg->total_len(msg));
	if (obj->data_out(obj, msg) <= 0) {
		return -1;
	}

	next_child = obj->child;
	while (next_child) {
		next_child->msg.cpy(&next_child->msg, msg);
		if (next_child->data_in(next_child, &next_child->msg) < 0) {
			ERROR("error while processing data in %s\n", next_child->name);
			next_child = next_child->next;
			continue;
		}

		/* Dive deeper */
		if (next_child->execute_out(next_child) < (size_t) 0) {
			return -1;
		}

		/* Continue on the same level */
		next_child = next_child->next;
	}

	return 0;
}

int processing_init(processing_obj * const obj)
{
	DEBUG("Initializing object %p\n", obj);

	if (message_init(&obj->msg)) {
		return -1;
	}

	obj->data_in = processing_data_in_default;
	obj->data_out = processing_data_out_default;
	obj->register_element = processing_reg_reader;
	obj->execute_out = processing_execute_out;
	obj->name = "Unknown";
	obj->req_end = false;
	obj->child = NULL;
	obj->next = NULL;

	return 0;
}

int processing_fini(processing_obj * const obj)
{
	DEBUG("Denitializing object %p\n", obj);
	message_fini(&obj->msg);

	obj->data_in = processing_data_in_default;
	obj->data_out = processing_data_out_default;
	obj->register_element = processing_reg_readder_default;
	obj->execute_out = processing_execute_out_default;
	obj->req_end = true;

	return 0;
}
