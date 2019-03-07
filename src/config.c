/** 
 * @file config.c
 * 
 * @brief A generic config object. It should absract the type of configuration
 *		from the real implementation of the configuration.
 *		Currenlty one real implemenation is available using ini config
 *		file. The current implementation use the singleton model object.
 *		Now only one implementation of the configuration can exists.
 * @author	Alexandre Malki <amalki@piap.pl>
 */ 

#include <debug.h>
#include <config.h>

#include <stdbool.h>

/** This is the private structure holding
 * information for each individual configuration
 */
typedef struct {
	/** This is the path to the configuration file */
	const char * const path_cfg;

	/** Indicates if an instance is init or not */
	bool is_init;

	/** A pointer to the config_obj. As this is a singleton */
	config_obj *obj_parent;
} config_priv_data;

/**
 * Currently the allocation is static and there is only one configuration
 * available
 */
static config_priv_data cfg_pdata;

/**
 * This is the default information that will be returned if the application 
 * accesses a non initialized/de-initialized object.
 */
static cfg_param default_param[CONFIG_UNSIGNED_INT + 1] = 
{
	[CONFIG_STR] = { "none", "none", "none", CONFIG_STR, "nil"},
	[CONFIG_INT] = { "none", "none", "none", CONFIG_INT, 0},
	[CONFIG_UNSIGNED_INT] = { "none", "none", "none", CONFIG_UNSIGNED_INT, 0},
};

/**
 * @brief Default callback to get value. This callback is used to avoid 
 *		segmentation fault in case of double fini when getting 
 *		a configuration value.
 * @param obj This is the configuration abstraction object.
 * @param param This is parameter information to get.
 * @return Will return the default object corresponding to the kind type of data
 *		to retrieve.
 */
static cfg_param * const config_default_get_value(config_obj * const obj,
					   cfg_param * const param)
{
	WARNING("The config module is not loaded\n");
	return &default_param[param->type];
}

/**
 * @brief Default callback to get value. This callback is used to avoid 
 *		segmentation fault in case of double fini when setting a 
 *		configuration value.
 * @param obj This is the configuration abstraction object.
 * @param param This is parameter information to get.
 * @return Will return -1 to indicate some was wrong.
 */
static int config_default_set_value(config_obj * const obj,
						cfg_param * const param)
{
	WARNING("The config module is not loaded\n");
	return -1;
}

/**
 * @brief Default callback to get value. This callback is used to avoid 
 *		segmentation fault in case of double fini when setting a 
 *		configuration value.
 * @param obj This is the configuration abstraction object.
 * @param param This is parameter information to get.
 * @return Will return -1 to indicate some was wrong.
 */
int config_init(config_obj * const obj)
{
	config_priv_data *pdata = &cfg_pdata;

	if (pdata->is_init) {
		ERROR("configuration already initialized\n");
		return -1;
	}

	obj->pdata = (void *) pdata;
	pdata->is_init = true;
	pdata->obj_parent = obj;

	return 0;
}

/**
 * Default callback to get value. This callback is used to avoid 
 *		segmentation fault in case of double fini when setting/setting a 
 *		configuration value.
 */
static config_obj config_default_obj = {
	.get_value = config_default_get_value,
	.set_value = config_default_set_value
};

/**
 * @brief This function will retrieve a valid instance.
 * @return NULL upon error, a new instance otherwise.
 */
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

/**
 * @brief This function will release the used instance and set default
 *		callbacks.
 * @return 0 upon success, -1 otherwise.
 */
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

