/** 
 * @file config_ini.c
 * @brief Implementation of the virtual config_obj. This implementation only
 *		only manage ini files. Also it does not write back value to the
 *		openned file.
 * @author	Alexandre Malki <amalki@piap.pl>
 */ 
#include <debug.h>
#include <config_ini.h>

#include <ini.h>

#include <stdbool.h>

/** 
 * Internal private structure holding context information about the ini
 * implementation.
 */
typedef struct {
	/** Pointer on the ini library context. */
	ini_t *ini_cfg;
	/** Path to the configuration file. */
	const char * path_cfg;
	/**
	 * Boolean indicating if config ini module was initialized.
	 * @sa config_ini_init
	 * @sa config_ini_fini
	 */
	bool is_init;
	/**
	 * Boolean indicating if the init library successfully get information
	 * from the configuration file.
	 */
	bool is_cfg_init;
} config_ini_priv_data;

static config_ini_priv_data ci_pdata;

/** 
 * @brief This function will retrieve the information located in a configuration
 *		ini file.
 * @param obj This is the abstracted config object. 
 * @param param The parameter object containing the section, field name and type
 *		of object.
 * @return A pointer on the same structure with the update value field. NULL is
 *		return upon error.
 */
static cfg_param * const  config_ini_get_value(config_obj * const obj,
					       cfg_param * const param)
{
	config_ini_obj *cfg_ini_obj  = (config_ini_obj *)obj;
	config_ini_priv_data *pdata = (config_ini_priv_data *)cfg_ini_obj->pdata;
	ini_t *cfg = pdata->ini_cfg;
	int rc = 1;

	if (!pdata->is_cfg_init) {
		ERROR("Config file not set, please provide\n");
		return NULL;
	}

	switch (param->type) {
	case CONFIG_INT:
		rc = ini_sget(cfg, param->section, param->name, "%d",
			 &param->value.s32);
	break;
	case CONFIG_UNSIGNED_INT:
		rc = ini_sget(cfg, param->section, param->name, "%u",
		         &param->value.u32);
	break;
	case CONFIG_STR:
		param->value.str = ini_get(cfg, param->section, param->name);
		/* An error occured */
		if(!param->value.str)
			rc = 0;
	break;
	default:
		WARNING("Unhandled type of param\n");
		return NULL;
	break;
	}

	if (!rc) {
		param->found = false;
		ERROR("Could not get param from section [%s] field: %s\n",
		      param->section, param->name);
	} else {
		param->found = true;
	}

	return param;
}


/** 
 * @brief Callback setting the value in the ini file. Currently not implemented.
 * @param obj This is the abstracted config object. 
 * @param param structure containing information about the section/field to
 *		access.
 * @return Always 0.
 */
static int config_ini_set_value(config_obj * const obj,
					       cfg_param * const param)
{
	WARNING("Not possible\n");
	return 0;
}

/** 
 * @brief Callback opening the configuration ini file and initializing the ini
 * 		library.
 * @param obj Ini configuration object
 * @param msg Path to the configuration file. 
 * @return 0 upon success, -1 otherwise.
 */
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

/** 
 * @brief Initialization function in charge to set internal and virtual 
 *		callbacks and set information.
 * @param obj This is the abstracted config object. 
 * @return 0 upon success, -1 otherwise.
 */
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

/** 
 * @brief  De-Initialisation of the object.
 * @param obj This is the abstracted config object. 
 * @return 0 upon success, -1 otherwise.
 */
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
