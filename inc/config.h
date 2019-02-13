#ifndef __CONFIG_H__
#define __CONFIG_H__


#ifdef  MESSAGE_DYNAMIC

#else
/** This is */
#define MESSAGE_NSTANCES_CNT_MAX	32
/** This is */
#define MESSAGE_BUFFER_SZ_MAX		1024

#endif

#define STRING_MAX_LENGTH		255

/** Used in the uart receive callback */
#define UART_INTERNAL_BUF_LEN_MAX	1024

/** The maximum length of the string */
#define CONFIG_STR_LEN_MAX		512

/** Max number of file that can be opened */
#define FILE_COUNT_MAX			16

/** Max number of uart device that can be openned */
#define UART_COUNT_MAX			1


/** This configuration will be taken either from a .INI file like
 *  or parsed from the command line arguments.
 *  As for now the parameter cannot be changed while applicaiton is running
 *  TODO: Add a reset cb to allow on the fly changes
 */

typedef enum {
	CONFIG_STR,
	CONFIG_INT,
	CONFIG_UNSIGNED_INT,
} config_type_value;

typedef struct {
	/** Not in use currently might be in the future */
	char * const parent_section;

	/** Name of the section where to look for the param */
	char * const section;

	/** Name of the parameter to get the value from*/
	char * const name;

	/** Type of parameter */
	config_type_value type;

	/** Values stored as union depending on the value of it */
	union {
		const char	*str;
		int		s32;
		unsigned int	u32;
	} value;
} cfg_param;

#define CONFIG_HELPER_CREATE(section_cfg, name_cfg, type_cfg)	\
		{  						\
		 .section = section_cfg,	   		\
		 .name = name_cfg,			   	\
		 .type = type_cfg,		   		\
		}

typedef struct config_obj_st config_obj;

typedef cfg_param * const (*config_get_value_cb)
			  (config_obj * const obj, cfg_param * const param);

typedef int		  (*config_set_value_cb)
			  (config_obj * const obj, cfg_param * const param);

struct config_obj_st {
	config_get_value_cb 	get_value;
	config_set_value_cb 	set_value;

	void 			*pdata;
};


int config_init(config_obj * const obj);
config_obj * const config_get_instance(void);
int config_fini(config_obj * const obj);

#define CONFIG_HELPER_GET_STR(param_info) \
		config_get_instance()->get_value(config_get_instance(), param_info)->value.str

#define CONFIG_HELPER_GET_U32(param_info) \
		config_get_instance()->get_value(config_get_instance(), param_info)->value.u32

#define CONFIG_HELPER_GET_S32(param_info) \
		config_get_instance()->get_value(config_get_instance(), param_info)->value.s32

#endif /* __CONFIG_H__ */
