#ifndef __CONFIG_INI_H__
#define __CONFIG_INI_H__

#include <message.h>
#include <config.h>

typedef struct config_ini_obj_st config_ini_obj;

typedef int (*config_ini_open_cfg_cb)(config_ini_obj * const obj,
				      const message_obj *msg);

struct config_ini_obj_st {
	config_obj cfg_obj;

	config_ini_open_cfg_cb open_cfg;

	void *pdata;	
} ;

int config_ini_init(config_ini_obj * const obj);
int config_ini_fini(config_ini_obj * const obj);

#endif /* __CONFIG_INI_H__ */
