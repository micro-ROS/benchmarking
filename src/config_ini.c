#include <debug.h>
#include <config_ini.h>

#include <ini.h>

#include <stdbool.h>

typedef struct {
	ini_t *ini_cfg;
	const char * path_cfg;
	bool is_init;
	bool is_cfg_init;
} config_ini_priv_data;

static config_ini_priv_data ci_pdata;

static cfg_param * const  config_ini_get_value(config_obj * const obj,
					       cfg_param * const param)
{
	config_ini_obj *cfg_ini_obj  = (config_ini_obj *)obj;
	config_ini_priv_data *pdata = (config_ini_priv_data *)cfg_ini_obj->pdata;
	ini_t *cfg = pdata->ini_cfg;

	if (!pdata->is_cfg_init) {
		ERROR("Config file not set, please provide\n");
		return NULL;
	}

	switch (param->type) {
	case CONFIG_INT:
		ini_sget(cfg, param->section, param->name, "%d",
			 &param->value.s32);
	break;
	case CONFIG_UNSIGNED_INT:
		ini_sget(cfg, param->section, param->name, "%u",
		         &param->value.u32);
	break;
	case CONFIG_STR:
		param->value.str = ini_get(cfg, param->section, param->name);
	break;
	default:
		WARNING("Unhandled type of param\n");
		return NULL;
	break;
	}

	return param;
}


static int config_ini_set_value(config_obj * const obj,
					       cfg_param * const param)
{
	DEBUG("Not possible\n");
	return 0;
}

static int config_ini_open_config(config_ini_obj * const obj,
				  const char * msg)
{
	config_ini_priv_data *pdata = (config_ini_priv_data *)obj->pdata;

	if (!pdata->is_init) {
		ERROR("Config ini not initialized\n");
		return -1;
	}

	if (pdata->is_cfg_init) {
		ERROR("Config ini already init, cannot init again\n");
		return -1;
	}

	pdata->path_cfg = msg;
	pdata->ini_cfg = ini_load(msg);

	if (!pdata->ini_cfg) {
		ERROR("Could not open cfg file %s\n", msg);
		return -1;
	}

	pdata->is_cfg_init = true;
	return 0;
}

int config_ini_init(config_ini_obj * const obj)
{
	config_obj *cfg_obj = (config_obj *) obj;
	config_ini_priv_data *pdata = &ci_pdata;

	if (config_init(cfg_obj)) {
		return -1;
	}

	obj->pdata = (void *) pdata;
	obj->open_cfg = config_ini_open_config;

	cfg_obj->get_value = config_ini_get_value;
	cfg_obj->set_value = config_ini_set_value;

	pdata->is_init = true;
	return 0;
}

int config_ini_fini(config_ini_obj * const obj)
{
	config_ini_priv_data *pdata = (config_ini_priv_data *)obj->pdata;
	config_obj *cfg_obj = (config_obj *) obj;

	if (config_fini(cfg_obj)) {
		return -1;
	}

	if (!pdata->is_init) {
		ERROR("Already De-initialized\n");
		return -1;
	}

	ini_free(pdata->ini_cfg);
	pdata->is_cfg_init = false;
	pdata->is_init = false;
	
	return 0;	
}
