#ifndef __CONFIG_INI_H__
#define __CONFIG_INI_H__

#include <config.h>


typedef struct {
	config_obj cfg_obj;

	void *pdata;	
} config_ini_obj;

int config_ini_init(config_ini_obj *obj);
int config_ini_fini(config_ini_obj *obj);

#endif /* __CONFIG_INI_H__ */
