/*****************************************************************
 * file: swd_ctrl.c
 * author: Alexandre Malki <amalki@piap.com>
 * @brief: This is swd_ctrl file. The swd_ctrl_obj this will fork a openocd
 * 		instance with the configuration file passed.
 *
 * 		TODO: This module shall be able to take configuration for different
 * 		kind of benchmarking (perf-execution/memory-benchmarking).
 *****************************************************************/
#include <debug.h>
#include <common-macros.h>
#include <config.h>
#include <swd_ctrl.h>

#include <openocd.h>

#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/** 
 * This is the type of configuration that has to be to openocd
 */
enum config_type {
	/** Debugger interface configuration */
	CFG_INTERFACE = 0,
	/** CPU configuration */
	CFG_CPU,
};

/**
 * This structure holds informattion related to the debug probe interface or the
 * CPU device
 */
typedef struct {
	/** Type of configuration file. */
 	enum config_type	type;
	/** configuration name */
	const char		cfg_name[STRING_MAX_LENGTH];
	/** Command to send to openocd */
	const char		cmd[STRING_MAX_LENGTH];
} swd_ctrl_config;

/**
 * SWD control Internal structure (private data).
 */
typedef struct {
	/** This is the configuration section information */
	char		cfgs[CFG_CPU+1][STRING_MAX_LENGTH];
	/** This is the openocd forked pid*/
	pid_t			fork_pid;
	/** Boolean informing  if the object is initialized or not */
	bool			is_init;
} swd_ctrl_priv_data;

/** Instance of the swd_ctr private data */
static swd_ctrl_priv_data swd_ctrl_pdata;

/**
 * @brief Retrieves the type of session from the configuration to
 * 		get the correct configuration file.
 * @return The type field name (exection or memory benchmarking session).
 */
static const char *swd_ctrl_get_script_cpu(void)
{
	const char *param_str;

	cfg_param param = {
				.section = CFG_SECTION_SESSION,
				.type = CONFIG_STR,
				.name = CFG_SECTION_SESSION_TYPE,
			  };

	param_str = CONFIG_HELPER_GET_STR(&param);
	if (!param.found) {
		return NULL;
	}

	if (!strcmp(CFG_SECTION_SESSION_TYPE_VAL_PM, param_str)) {
		param_str = CFG_SECTION_SWD_CTRL_CPU_PM;
	} else if (!strcmp(CFG_SECTION_SESSION_TYPE_VAL_PE, param_str)) {
		param_str = CFG_SECTION_SWD_CTRL_CPU_PE;
	} else {
		ERROR("Could not find the script required\n");
		param_str = NULL;
	}

	return  param_str;
}

/**
 * @brief Retrieve information from the configuration module to set the
 * 		configuration file (openocd scripts). It will check the
 * 		type of execution. 
 * @param obj swd_ctrl_obj that is the object that controls the SWD.
 * @param type the type of configuration needed CPU/INTERFACE.
 * @return 0 upon success, -1 otherwise
 */
static int swd_ctrl_set_from_gbl_cfg(swd_ctrl_obj * const obj,
					       enum config_type type)
{
	swd_ctrl_priv_data *pdata = (swd_ctrl_priv_data *) obj->pdata;
	swd_ctrl_config *swd_ctrl_cfg;
	const char *param_str;

	cfg_param param = {
				.section = CFG_SECTION_SWD_CTRL,
				.type = CONFIG_STR,
			  };

	if (CFG_INTERFACE == type)  {
		param.name = CFG_SECTION_SWD_CTRL_IF;
	} else {
		if (!(param.name = swd_ctrl_get_script_cpu())) {
			return -1;
		}
	}

	param_str = CONFIG_HELPER_GET_STR(&param);
	if (!param.found) {
		return -1;
	}
	snprintf(pdata->cfgs[type], STRING_MAX_LENGTH - 1, "-f%s", param_str);

	return 0;
}

/*
 * @brief Method called to set the INTERFACE configuration file.
 * @param obj swd_ctrl_obj that is the object that controls the SWD.
 * @return 0 upon sucess, -1 otherwise.
 */
static int swd_ctrl_set_interface_from_gbl_cfg(swd_ctrl_obj * const obj)
{
	return swd_ctrl_set_from_gbl_cfg(obj,CFG_INTERFACE);
}

/*
 * @brief Method called to set the CPU configuration file.
 * @param obj swd_ctrl_obj that is the object that controls the SWD.
 * @return 0 upon sucess, -1 otherwise.
 */
static int swd_ctrl_set_cpu_from_gbl_cfg(swd_ctrl_obj * const obj)
{
	return swd_ctrl_set_from_gbl_cfg(obj,CFG_CPU);
}

/**
 * @brief starting openocd software 
 * @param obj swd_ctrl_obj that is the object that controls the SWD.
 * @param prog the name of the calling program.
 * @return 0 upon succes, -1 otherwise.
 */
static int swd_ctrl_start(swd_ctrl_obj * const obj, char * const prog)
{
	swd_ctrl_priv_data *pdata = (swd_ctrl_priv_data *) obj->pdata;

	const char *argv[] = {
				prog,
#ifdef SWD_CTRL_OPENOCD_DEBUG
				"-d",
#endif
				"-s"SWD_CTRL_OPENOCD_SCRIPT_PATH,
				pdata->cfgs[CFG_INTERFACE],
			        pdata->cfgs[CFG_CPU],
				SWD_CTRL_OPENOCD_INIT,
				SWD_CTRL_OPENOCD_RESET_START
		       };
	int argc = ARRAY_SIZE(argv);
	pid_t  pid = fork();
	int rc = 0;

	if (!pid) {
		/* I am your child */
#ifndef SWD_CTRL_OPENOCD_DEBUG
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
	 	rc = openocd_main(argc, (char **) argv);
		return (rc);
	}

	/* I am your father */
	pdata->fork_pid = pid;
	if (pid < 0) {
		return -1;
	}

	return 0;
}

/**
 * @brief stop the openocd program  it send a kill signal to the child object
 * 	and waits for its return.
 * @param obj swd_ctrl_obj that is the object that controls the SWD.
 * @return 0 upon succes, -1 otherwise.
 */
static int swd_ctrl_stop(swd_ctrl_obj * const obj)
{
	swd_ctrl_priv_data *pdata = (swd_ctrl_priv_data *) obj->pdata;

	if (pdata->fork_pid < 0) {
		ERROR("No process forked\n");
		return -1;
	}

	kill(pdata->fork_pid, SIGKILL);
	waitpid(pdata->fork_pid, NULL, 0 );

	return 0;
}

/**
 * @brief default configuration method 
 */
static int swd_ctrl_default_set_cfg(swd_ctrl_obj * const obj, const char *name)
{
	WARNING("Instance not initalized\n");
	return -1;
}

/**
 * @brief default start method.
 */
static int swd_ctrl_default_start(swd_ctrl_obj * const obj, char * const prog)
{
	WARNING("Instance not initalized\n");
	return -1;
}

/**
 * @brief default stop method
 */
static int swd_ctrl_default_stop(swd_ctrl_obj * const obj)
{
	WARNING("Instance not initalized\n");
	return -1;
}

/**
 * @brief default function to set from global cfg 
 */
static int swd_ctrl_set_default_from_gbl_cfg(swd_ctrl_obj * const obj)
{
	ERROR("Instance not initialized\n");
	return -1;
}

/**
 * @brief this is the default set configuration.
 */
static int swd_ctr_set_cfg_todo(swd_ctrl_obj * const obj, const char *name)
{
	ERROR("Not implemented\n");
	return -1;
}

int swd_ctrl_init(swd_ctrl_obj * const obj)
{
	if (swd_ctrl_pdata.is_init) {
		ERROR("Maximum number fof swd obj reached\n");
		return -1;
	}

	obj->set_cfg_cpu = swd_ctr_set_cfg_todo;
	obj->set_cfg_if = swd_ctr_set_cfg_todo;
	obj->set_cfg_cpu_from_gbl_cfg = swd_ctrl_set_cpu_from_gbl_cfg;
	obj->set_cfg_if_from_gbl_cfg = swd_ctrl_set_interface_from_gbl_cfg;
	obj->start = swd_ctrl_start;
	obj->stop = swd_ctrl_stop;
	swd_ctrl_pdata.is_init = true;
	obj->pdata = &swd_ctrl_pdata;

	return 0;
}

int swd_ctrl_fini(swd_ctrl_obj * const obj)
{
	swd_ctrl_priv_data *pdata = (swd_ctrl_priv_data *) obj->pdata;

	if (!pdata->is_init) {
		ERROR("Cannot deinit this swd obj twice\n");
		return -1;
	}

	obj->set_cfg_cpu = swd_ctrl_default_set_cfg;
	obj->set_cfg_if = swd_ctrl_default_set_cfg;
	obj->set_cfg_cpu_from_gbl_cfg = swd_ctrl_set_default_from_gbl_cfg;
	obj->set_cfg_if_from_gbl_cfg = swd_ctrl_set_default_from_gbl_cfg;
	obj->start = swd_ctrl_default_start;
	obj->stop = swd_ctrl_default_stop;

	pdata->is_init = false;
	obj->pdata = NULL;

	return 0;
}
