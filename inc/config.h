/**
 * @file config.h
 * @brief This file contains all definition needed by all source file. Also it
 *		contains definitions of the generic config object.
 * @author	Alexandre Malki <amalki@piap.pl>
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__


#include <config_lib.h>

#include <stdbool.h>

#ifdef  MESSAGE_DYNAMIC

#else
/** This is */
#define MESSAGE_NSTANCES_CNT_MAX	32
/** This is */
#define MESSAGE_BUFFER_SZ_MAX		16384

#endif

#define STRING_MAX_LENGTH		255

/** Used in the uart receive callback */
#define UART_INTERNAL_BUF_LEN_MAX	128

/** The maximum length of the string */
#define CONFIG_STR_LEN_MAX		512

/** Max number of file that can be opened */
#define FILE_COUNT_MAX			16

/** Max number of uart device that can be openned */
#define UART_DEV_COUNT_MAX		1

#define UART_COUNT_MAX			STRING_MAX_LENGTH
#define UART_TIMEOUT_MS			100U
#define UART_MAX_POLL_RETRIES		10

/* Configuration */
#ifdef CONFIG_LIBINI

/* Section Session */
#define CFG_SECTION_SESSION		"session"
#define CFG_SECTION_SESSION_TYPE	"type"
#define CFG_SECTION_SESSION_TYPE_VAL_PE	"execution-performance"
#define CFG_SECTION_SESSION_TYPE_VAL_PM	"memory-performance"

/* Section SWD CTRL */
#define CFG_SECTION_SWD_CTRL		"swd-ctrl"

/* Configuration regarding the SWD module */
#define CFG_SECTION_SWD_CTRL_IF		"script_interface"
#define CFG_SECTION_SWD_CTRL_CPU_PE	"script_cpu_perf_ex"
#define CFG_SECTION_SWD_CTRL_CPU_PM	"script_cpu_perf_mem"

/* Section EXT BINS */
#define CFG_SECTION_EXT_BIN		"ext-bins"

/* Nedded by the execution perfomance module (perf_ex.c) */
#define CFG_SECTION_EXT_BIN_TC		"path_toolchain"
#define CFG_SECTION_EXT_BIN_ELF		"path_elf"

/* configuration section */
#define CFG_SECTION_OUTPUT_FILE		"output-files"
#define CFG_SECTION_OUTPUT_FILE_PM	"path-mem"
#define CFG_SECTION_OUTPUT_FILE_PE	"path-perf"

#else /* CONFIG_LIBINI */

#error "Not other configuration library than libinit defined"

#endif /* CONFIG_LIBINI */

#ifdef SWD_CTRL_OPENOCD /* SWD_CTRL_OPENOCD */

#define SWD_CTRL_OPENOCD_INTERFACE_PATH SWD_CTRL_OPENOCD_SCRIPT_PATH"/interface/"
#define SWD_CTRL_OPENOCD_CPU_PATH	SWD_CTRL_OPENOCD_SCRIPT_PATH"/target/"
#define SWD_CTRL_OPENOCD_INIT		"-c init"
#define SWD_CTRL_OPENOCD_RESET_START	"-c reset run"
#define SWD_CTRL_OPENOCD_DEBUG

#else /* SWD_CTRL_OPENOCD */

#error "Not other swd ctrl library than openOCD exists"

#endif /* SWD_CTRL_OPENOCD */

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
	const char * parent_section;

	/** Name of the section where to look for the param */
	const char * section;

	/** Name of the parameter to get the value from*/
	const char * name;

	/** Type of parameter */
	config_type_value type;

	/** Values stored as union depending on the value of it */
	union {
		const char	*str;
		int		s32;
		unsigned int	u32;
	} value;

	/** Set to true if the value is found, to false otherwise */
	bool found;
} cfg_param;

/** This is a helper to create a structure structure */
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

/** This is the parent object of all configs. Meaning that all implementation of
 * a configuration module should inherite from this object*/
struct config_obj_st {
	/** Callback to retrieve value of a field in a configuration */
	config_get_value_cb 	get_value;
	/** Callback to set the value of a field in a configuration */
	config_set_value_cb 	set_value;
	/** This a pointer to a private data */
	void 			*pdata;
};

/**
 * @brief Default callback to get value. This callback is used to avoid 
 *		segmentation fault in case of double fini when setting a 
 *		configuration value.
 * @param obj This is the configuration abstraction object.
 * @param param This is parameter information to get.
 * @return Will return -1 to indicate some was wrong.
 */
int config_init(config_obj * const obj);

/**
 * @brief This function will retrieve a valid instance.
 * @return NULL upon error, a new instance otherwise.
 */
config_obj * const config_get_instance(void);

/**
 * @brief This function will release the used instance and set default
 *		callbacks.
 * @return 0 upon success, -1 otherwise.
 */
int config_fini(config_obj * const obj);

/**
 * Helper to get the string from a cfg_param object 
 * @warning This helper assume that the configuration was already initialized.
 */
#define CONFIG_HELPER_GET_STR(param_info) \
		config_get_instance()->get_value(config_get_instance(), param_info)->value.str

/**
 * Helper to get the unsigned 32 bits integer from a cfg_param object
 * @warning This helper assume that the configuration was already initialized.
 */
#define CONFIG_HELPER_GET_U32(param_info) \
		config_get_instance()->get_value(config_get_instance(), param_info)->value.u32

/**
 * Helper to get the signed 32 bits integer from a cfg_param object
 * @warning This helper assume that the configuration was already initialized.
 */
#define CONFIG_HELPER_GET_S32(param_info) \
		config_get_instance()->get_value(config_get_instance(), param_info)->value.s32

#endif /* __CONFIG_H__ */
