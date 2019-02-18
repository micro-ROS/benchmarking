#include <debug.h>
#include <config.h>
#include <common-macros.h>
#include <form.h>
#include <pkt_converter.h>

#include <cJSON.h>

#include <stdbool.h>
#include <string.h>
#include <unistd.h>


typedef struct {
	cfg_param param;
	/**
	 * Using the address of the pointer in case of  changes
	 */
	cJSON **parent;
} json_config;

typedef struct {
	cJSON	*root_benchmarking;
	cJSON	*session_info;
	cJSON	*board_info;
	cJSON	*soft_info;
	cJSON	*packets;
	cJSON	*raw_packets;
	cJSON	*last_packet;

	bool	is_used;
	bool	is_nfirstrun;
	json_config *json_cfgs;
	pkt_to_form *ptf;
} form_json_priv_data;


/* TODO Stringify */


static pkt_to_form cjson_ptf = {
		.fmt = PKT_CONVERTER_CJSON,
	};


static form_json_priv_data json_priv_data = {
	.ptf = &cjson_ptf,
};

#define JSON_CONFIG_INIT(section, name, type, ROOT_CJSON)			\
		{	 							\
			.param = CONFIG_HELPER_CREATE(section, name, type),	\
			.parent = &json_priv_data.ROOT_CJSON,			\
		}
/* TODO: Think of a more flexible way to present it */
static json_config form_json_cfgs[] = {
	JSON_CONFIG_INIT("session", "id", CONFIG_STR, session_info),
	JSON_CONFIG_INIT("session", "date", CONFIG_STR, session_info),
	JSON_CONFIG_INIT("session", "type", CONFIG_STR, session_info),

	JSON_CONFIG_INIT("board-info", "name", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board-info", "cpu", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board-info", "cortex", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board-info", "hardware_module", CONFIG_STR, board_info),

	JSON_CONFIG_INIT("soft-info", "name", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft-info", "version_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft-info", "commit-id_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft-info", "config_nuttx", CONFIG_STR, soft_info),
};

static void form_json_format_init_output(message_obj * const msg)
{
	char *brace = strrchr(msg->ptr(msg), ']');
	char *end = strrchr(msg->ptr(msg), '}');


	brace[0] = ',';
	brace[1] = '\n';
	msg->set_length(msg, msg->length(msg) - ((uintptr_t) end - (uintptr_t)brace));
}

static void form_json_format_child_output(message_obj * const msg)
{
	char *brace, *lf, *old_lf;
	unsigned int shifts = 0, written = 0;
	char temp[MESSAGE_BUFFER_SZ_MAX];

	memcpy(temp, msg->ptr(msg), msg->length(msg));
	lf = strtok(temp, "\n");
	while (lf) {
		written += snprintf(&msg->ptr(msg)[written], msg->total_len(msg) - written,
				    "\t\t%s\n", lf);
		lf = strtok(NULL, "\n");
	}

	msg->ptr(msg)[written - 1] = ',';
	msg->ptr(msg)[written] = '\n';
	msg->set_length(msg, written + 1);
}

static size_t form_json_print_nodes(cJSON *pnode, message_obj * const msg)
{

	size_t rc = -1;
	char *data = NULL;

	data = cJSON_Print(pnode);

	if (!data) {
		ERROR("could not print to json\n");
		return -1;
	}

	if ((rc = strnlen(data, msg->total_len(msg) * 2)) > msg->total_len(msg)) {
		ERROR("Data to write is bigger than the msg's buffer, "
		      "%ld/%ld\n", rc, msg->total_len(msg));
		goto buffer_failed;
	}

	msg->write(msg, data, rc + 1);
	/* DEBUG ("json:%s\n", msg->ptr(msg));*/
	rc = msg->length(msg);
buffer_failed:
	free(data);
	return rc;

}

static size_t form_json_send_data(processing_obj * const obj,
				  message_obj * const msg)
{
	form_json_priv_data *jpdata = (form_json_priv_data *)
						((form_obj *) obj)->pdata;
	cJSON *node;
	int rc=0, err;

	/* TODO clean this function */
	if (!jpdata->is_nfirstrun) {
		node = jpdata->root_benchmarking;
		rc = form_json_print_nodes(node, msg);
		jpdata->is_nfirstrun = 1;
		form_json_format_init_output(msg);
	} else {
		node = jpdata->ptf->root->child;
		while (node) {
			if ( err = form_json_print_nodes(node, msg) < 0) {
				return -1;
			}

			form_json_format_child_output(msg);

			rc += err;
			node = node->next;
		}
	}

	cJSON_Delete(jpdata->ptf->root->child);
	jpdata->ptf->root->child = NULL;
	return rc;
}

static size_t form_json_receive_data(processing_obj * const obj,
				     message_obj * const msg)
{
	form_json_priv_data *jpdata = (form_json_priv_data *)
						((form_obj *) obj)->pdata;
	jpdata->ptf->root = jpdata->packets;

	return pkt_convert(jpdata->ptf, msg);
}

static int form_json_create_str_node(cJSON *parent, const char * const name,
				      char const * const value)
{
	if (!cJSON_AddStringToObject(parent, name, value)) {
		ERROR("Error while creating string node\n");
		return -1;
	}

	return 0;
}


static int form_json_create_num_node(cJSON *parent, const char * const name,
				     float value)
{
	if (!cJSON_AddNumberToObject(parent, name, value)) {
		ERROR("Error while creating float node\n");
		return -1;
	}

	return 0;
}

static int form_json_init_nodes(form_obj * const obj)
{
	form_json_priv_data *jpdata = (form_json_priv_data *) obj->pdata;
	cfg_param *cfg;
	cJSON *parent;
	const char *name;
	float value_fl;
	char const * value_str;
	int rc = -1;

	for (unsigned int i=0; i<ARRAY_SIZE(form_json_cfgs);i++) {
		cfg = &jpdata->json_cfgs[i].param;
		parent = *jpdata->json_cfgs[i].parent;
		name = cfg->name;

		switch(cfg->type) {
			case CONFIG_STR:
				value_str = CONFIG_HELPER_GET_STR(cfg);
				rc = form_json_create_str_node(parent, name,
							       value_str);
			break;
			case CONFIG_INT:
				value_fl = (float) CONFIG_HELPER_GET_S32(cfg);
				rc = form_json_create_num_node(parent, name,
							       value_fl);
			break;
			case CONFIG_UNSIGNED_INT:
				value_fl = (float) CONFIG_HELPER_GET_U32(cfg);
				rc = form_json_create_num_node(parent, name,
							       value_fl);
			break;
			default:
				ERROR("Invalid type of configuration\n");
			break;
		}

		if (rc)
			return rc;
	}

	/* Creating packet table which will hold the debugging information */
	jpdata->packets = cJSON_AddArrayToObject(jpdata->root_benchmarking,
						   "packet");
	if (!jpdata->packets) {
		ERROR("Error while creating string node\n");
		return -1;
	}

	return 0;
}

static int form_json_init_session(form_obj * const obj)
{
	form_json_priv_data *pdata = (form_json_priv_data *) obj->pdata;
	json_config *json_cfgs = pdata->json_cfgs;

	/* create root nodes 8 */
	pdata->session_info = cJSON_AddObjectToObject(pdata->root_benchmarking, "session");
	if (!pdata->session_info)
		return -1;

	pdata->board_info =  cJSON_AddObjectToObject(pdata->root_benchmarking, "board_info");
	if (!pdata->board_info)
		return -1;

	pdata->soft_info =  cJSON_AddObjectToObject(pdata->root_benchmarking, "soft_info");
	if (!pdata->soft_info)
		return -1;

	pdata->session_info =  cJSON_AddObjectToObject(pdata->root_benchmarking, "session_info");
	if (!pdata->session_info)
		return -1;

	if (form_json_init_nodes(obj)) {
		return -1;
	}

	return 0;
}

int form_cjson_init(form_obj * const obj)
{
	processing_obj * const proc_obj = (processing_obj *)obj;
	form_json_priv_data *pdata;

	if (processing_init(proc_obj)) {
		return -1;
	}

	if (json_priv_data.is_used) {
		ERROR("JSON from object already used\n");
		goto no_free_instance;
	}

	json_priv_data.is_used = true;
	json_priv_data.json_cfgs = form_json_cfgs;

	obj->pdata = (void *) &json_priv_data;
	pdata = obj->pdata;

	/**
	 * Creating root JSON object here the root only is needed
	 * to be freed
	 */
	pdata->root_benchmarking = cJSON_CreateObject();
	if (!pdata->root_benchmarking) {
		ERROR("JSON root object creation failed\n");
		goto json_failed;
	}

	/* Create other default object */
	if (form_json_init_session(obj)) {
		goto init_session_failed;
	}

	/* Set up processing callbacks that will translate the
	 * decoder information to json style element*/
	proc_obj->data_in = form_json_receive_data;
	proc_obj->data_out = form_json_send_data;

	return 0;
init_session_failed:
	cJSON_Delete(json_priv_data.root_benchmarking);
json_failed:
no_free_instance:
	processing_fini((processing_obj *) obj);
	return -1;
}

int form_cjson_fini(form_obj * const obj)
{
	DEBUG("To Be implemented\n");
	return 0;
}
