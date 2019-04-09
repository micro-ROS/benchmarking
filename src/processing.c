/*****************************************************************
 * file: processing.c
 * author: Alexandre Malki <amalki@piap.pl>
 * brief: A processing object is an object intented to be part of a pipeline.
 * 		Processing objects are intented to be queued in a hierachical
 * 		chains of multiple processing object. One processing object
 * 		can have only one parent, but can have multiple children.
 * 		If a new processing object needs to be added it should inherit
 * 		the public structure. A template create is located at
 * 		tools/gen-processing.sh .
 *
 * 		Processing objects from the same parent are on the same level.
 * 		In the current implementation, the processing objects one the
 * 		same level are not execute prior to their own respective
 * 		child(ren). Each element on the same level one after another will
 * 		process its children.
 *
 * 		Children (also called readers) have to be registered to the 
 * 		parent that will provide the data.
 *****************************************************************/
#include <processing.h>
#include <debug.h>
#include <string.h>

/**
 * Processing obj Internal structure.
 */
typedef struct {
	/** Boolean checking if the if the processing_objec is initialized */
	bool is_init;
} processing_priv_data;

/**
 * @brief data in default function
 */
static size_t processing_data_in_default(processing_obj * const obj,
				      message_obj * const msg)
{
	WARNING("No callback defined yet\n");
	return 0;
}

/**
 * @brief data out default function
 */
static size_t processing_data_out_default(processing_obj * const obj,
				          message_obj * const msg)
{
	WARNING("No callback defined yet\n");
	return 0;
}

/**
 * @brief register reader default function
 */
static int processing_reg_readder_default(processing_obj * const obj,
				             processing_obj * const el)
{
	WARNING("No callback defined yet\n");
	return 0;
}

/**
 * @brief Register a reader to a a processing object. The registered object
 * 		is a child.
 * @param obj parent object.
 * @param el reader object (children).
 */
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

/**
 * @brief Default execution object.
 */
static int processing_execute_out_default(processing_obj * const obj)
{
	WARNING("Not initiliazed\n");
	return -1;
}

/**
 * @brief execute object. This is recursive function that will loop through
 * 		all children and then go to next processing object on the same
 * 		level.
 * @param obj processing object to execute.
 * @return 0 upon success, -1 otherwise.
 */
static int processing_execute_out(processing_obj * const obj)
{
	processing_obj *next_child;
	message_obj *msg = &obj->msg;

	msg->set_length(msg, 0);
	memset(msg->ptr(msg), 0, msg->total_len(msg));
	if ((obj->data_out(obj, msg) <= 0) && (!obj->req_end)) {
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
