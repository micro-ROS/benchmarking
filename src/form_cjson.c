#include <debug.h>
#include <config.h>
#include <common-macros.h>
#include <form.h>
#include <pkt_converter.h>

#include <cJSON.h>

#include <stdbool.h>


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

	JSON_CONFIG_INIT("board_info", "name", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board_info", "cpu", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board_info", "cpu", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board_info", "cortex", CONFIG_STR, board_info),
	JSON_CONFIG_INIT("board_info", "hardware_module", CONFIG_STR, board_info),

	JSON_CONFIG_INIT("soft_info", "name", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft_info", "version_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft_info", "commitid_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft_info", "config_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft_info", "version_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft_info", "commitid_nuttx", CONFIG_STR, soft_info),
	JSON_CONFIG_INIT("soft_info", "config_nuttx", CONFIG_STR, soft_info),
};

static size_t form_json_send_data(processing_obj * const obj,
				  message_obj * const msg)
{
	size_t rc = -1;
	form_json_priv_data *jpdata = (form_json_priv_data *)
						((form_obj *) obj)->pdata;
	if (!jpdata->is_nfirstrun) {
		rc = cJSON_PrintPreallocated(jpdata->root_benchmarking,
				       msg->ptr(msg), msg->length(msg), true);
		jpdata->is_nfirstrun = 1;
	} else {
		rc = cJSON_PrintPreallocated(jpdata->ptf->root->child,
				       msg->ptr(msg), msg->length(msg), true);
	}

	cJSON_Delete(jpdata->ptf->root->child);
	return rc;
}

static size_t form_json_receive_data(processing_obj * const obj,
				     message_obj * const msg)
{
	form_json_priv_data *jpdata = (form_json_priv_data *)
						((form_obj *) obj)->pdata;
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
	char *name;
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

static int form_json_init_session(form_obj *obj)
{
	form_json_priv_data *pdata = (form_json_priv_data *) obj->pdata;
	json_config *json_cfgs = pdata->json_cfgs;

	for (unsigned int i=0; i < ARRAY_SIZE(json_cfgs); i++) {
		if (form_json_init_nodes(obj)) {
			return -1;
		}
	}

	return 0;
}

static int form_json_fini_session(form_obj *obj)
{
	DEBUG("To Be implemented\n");
	return 0;
}

int form_json_init(form_obj * const obj)
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
