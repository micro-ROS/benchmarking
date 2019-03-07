/** 
 * @file config_ini.h
 * @brief	Header file implementing the virutal config_obj. 
 * @author	Alexandre Malki <amalki@piap.pl>
 */ 
#ifndef __CONFIG_INI_H__
#define __CONFIG_INI_H__

#include <message.h>
#include <config.h>

typedef struct config_ini_obj_st config_ini_obj;

typedef int (*config_ini_open_cfg_cb)(config_ini_obj * const obj,
				      const char * const msg);

/** INI Configuration implementation of the config_obj generic */
struct config_ini_obj_st {
	/** This is the generic object this object inherit froms */
	config_obj cfg_obj;
	/** Callback to the configuration file to open */
	config_ini_open_cfg_cb open_cfg;
	/** Internal private data  */
	void *pdata;	
};

/** 
 * @brief Initialization function in charge to set internal and virtual 
 *		callbacks and set information.
 * @param obj This is the abstracted config object. 
 * @return 0 upon success, -1 otherwise.
 */
int config_ini_init(config_ini_obj * const obj);

/** 
 * @brief  De-Initialisation of the object.
 * @param obj This is the abstracted config object. 
 * @return 0 upon success, -1 otherwise.
 */
int config_ini_fini(config_ini_obj * const obj);

#endif /* __CONFIG_INI_H__ */
