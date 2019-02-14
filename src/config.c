#include <debug.h>
#include <config.h>

#include <stdbool.h>

typedef struct {
	const char * const path_cfg;

	bool is_init;

	config_obj *obj_parent;
} config_priv_data;

static config_priv_data cfg_pdata;

static cfg_param default_param[CONFIG_UNSIGNED_INT + 1] = 
{
	[CONFIG_STR] = { "none", "none", "none", CONFIG_STR, "nil"},
	[CONFIG_INT] = { "none", "none", "none", CONFIG_INT, 0},
	[CONFIG_UNSIGNED_INT] = { "none", "none", "none", CONFIG_UNSIGNED_INT, 0},
};

static cfg_param * const config_default_get_value(config_obj * const obj,
					   cfg_param * const param)
{
	WARNING("The config module is not loaded\n");
	return &default_param[param->type];
}

static int config_default_set_value(config_obj * const obj,
						cfg_param * const param)
{
	WARNING("The config module is not loaded\n");
	return -1;
}


int config_init(config_obj * const obj)
{
	config_priv_data *pdata = &cfg_pdata;

	if (pdata->is_init) {
		ERROR("configuration already initialized\n");
		return -1;
	}

	obj->pdata = (void *) pdata;
	pdata->is_init = true;

	return 0;
}
static config_obj config_default_obj = {
	.get_value = config_default_get_value,
	.set_value = config_default_set_value
};

config_obj * const config_get_instance(void)
{
	config_priv_data *pdata = &cfg_pdata;

	if (!pdata) {
		ERROR("Cannot get instance: Instance not allocated\n");
		return &config_default_obj;
	}

	if (!pdata->obj_parent) {
		ERROR("No config object allocated\n");
		return &config_default_obj;
	}

	if (!pdata->is_init) {
		ERROR("Cannot get instance: Instance not initialized\n");
		return &config_default_obj;
	}

	return pdata->obj_parent;
}


int config_fini(config_obj *obj)
{
	config_priv_data *pdata = (config_priv_data *)obj->pdata;

	if (!pdata->is_init)  {
		WARNING("Configuration Already deinit\n");
		return -1;
	}

	pdata->is_init = false;
	obj->get_value = config_default_get_value;
	obj->set_value = config_default_set_value;

	return 0;
}

