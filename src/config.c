#include <debug.h>
#include <config.h>

#include <stdbool.h>

typedef struct {
	const char * const path_cfg;
	const char * const path_cfg;

	bool is_init;

	config_obj *obj_parent;
} config_priv_data;

static config_priv_data cfg_pdata;

static int config_get_template(config_obj *str)
{
}

int config_init(config_obj *obj)
{
	config_priv_data *pdata = (config_priv_data *)obj->pdata;

	if (pdata->is_init) {
		ERROR("configuration already initialized\n");
		return -1;
	}

	return 0;
}

static config_obj * const config_get_instance(void)
{
	return NULL;
}

static cfg_param * const config_default_get_value(config_obj * const obj,
					   cfg_param * const param)
{
	WARNING("The config module is not loaded\n");
	return NULL;
}

static void config_default_set_value(config_obj * const obj,
						cfg_param * const param)
{
	WARNING("The config module is not loaded\n");
	return NULL;
}

int config_fini(config_obj *obj)
{
	config_priv_data *pdata = (config_priv_data *)obj->pdata;

	if (pdata->is_init)  {
		WARNING("Configuration Already deinit\n");
		return -1;
	}
	
	pdata->is_init = 0;
	obj->get_value = config_default_get_value;
	obj->set_value = config_default_set_value;

	return 0;
}

