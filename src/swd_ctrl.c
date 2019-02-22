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

enum config_type {
	CFG_INTERFACE = 0,
	CFG_CPU,
};

typedef struct {
	enum config_type	type;
	const char		cfg_name[STRING_MAX_LENGTH];
	const char		cmd[STRING_MAX_LENGTH];
} swd_ctrl_config;

static const char *swd_ctrl_config_gbl[CFG_CPU+1] = {
						CFG_SECTION_SWD_CTRL_IF,
						CFG_SECTION_SWD_CTRL_CPU
					};

static const swd_ctrl_config swd_ctrl_cfgs[] = {
			{
				.type = CFG_INTERFACE,
				.cfg_name = "stlink*",
				.cmd = "-f" SWD_CTRL_OPENOCD_INTERFACE_PATH"/stlink.cfg",
			},
			{
				.type = CFG_CPU,
				.cfg_name = "stm32f4*",
				.cmd = "-f" SWD_CTRL_OPENOCD_CPU_PATH"/stm32f4x.cfg",
			},
};

typedef struct {
	const char		*cfgs[CFG_CPU+1];
	char			args[STRING_MAX_LENGTH * 4];
	pid_t			fork_pid;

	bool			is_init;
} swd_ctrl_priv_data;

static swd_ctrl_priv_data swd_ctrl_pdata;

static const swd_ctrl_config *swd_ctrl_get_cfg(const char *name,
					       enum config_type type)
{
	char cfg[CONFIG_STR_LEN_MAX];
	const char *match;

	for (unsigned int i=0; i<ARRAY_SIZE(swd_ctrl_cfgs);i++) {
		if (type != swd_ctrl_cfgs[i].type)
			continue;

		strncpy(cfg, swd_ctrl_cfgs[i].cfg_name, sizeof(cfg -1));
		if (!(match = strtok(cfg,"*")))
			match = swd_ctrl_cfgs[i].cfg_name;

		if (!strncmp(match, name, strnlen(match,
			     sizeof(swd_ctrl_cfgs[i].cfg_name) - 1))) {
			DEBUG("Found configuration %s", swd_ctrl_cfgs[i].cfg_name);
			return &swd_ctrl_cfgs[i];
		}
	}

	ERROR("No configuration %s\n", name);
	return NULL;
}

static const char *swd_ctrl_find_param_gbl_cfg(cfg_param * const param,
					  enum config_type type)
{
	const swd_ctrl_config *swd_ctrl_cfg;
	const char *param_str = CONFIG_HELPER_GET_STR(param);

	if (!param_str) {
		return NULL;
	}

	if (!(swd_ctrl_cfg = swd_ctrl_get_cfg(param_str, type))) {
		return NULL;
	}

	return swd_ctrl_cfg->cmd;
}

static int swd_ctrl_set_from_gbl_cfg(swd_ctrl_obj * const obj,
					       enum config_type type)
{
	swd_ctrl_priv_data *pdata = (swd_ctrl_priv_data *) obj->pdata;
	swd_ctrl_config *swd_ctrl_cfg;
	const char *config_str;

	cfg_param param = {
				.section = CFG_SECTION_SWD_CTRL,
				.type = CONFIG_STR,
				.name = swd_ctrl_config_gbl[type],
			  };

	if (!(config_str = swd_ctrl_find_param_gbl_cfg(&param, type))) {
		return -1;
	}

	pdata->cfgs[type] = config_str;
	return 0;
}

static int swd_ctrl_set_interface_from_gbl_cfg(swd_ctrl_obj * const obj)
{
	return swd_ctrl_set_from_gbl_cfg(obj,CFG_INTERFACE);
}

static int swd_ctrl_set_cpu_from_gbl_cfg(swd_ctrl_obj * const obj)
{
	return swd_ctrl_set_from_gbl_cfg(obj,CFG_CPU);
}

static int swd_ctrl_start(swd_ctrl_obj * const obj, char * const prog)
{
	swd_ctrl_priv_data *pdata = (swd_ctrl_priv_data *) obj->pdata;

	const char *argv[] = {
				prog,
#ifdef DEBUG_OPENOCD
				"-d",
#endif
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
#ifndef DEBUG_OPENOCD
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

static int swd_ctrl_default_set_cfg(swd_ctrl_obj * const obj, const char *name)
{
	WARNING("Instance not initalized\n");
	return -1;
}

static int swd_ctrl_default_start(swd_ctrl_obj * const obj, char * const prog)
{
	WARNING("Instance not initalized\n");
	return -1;
}
static int swd_ctrl_default_stop(swd_ctrl_obj * const obj)
{
	WARNING("Instance not initalized\n");
	return -1;
}

static int swd_ctrl_set_default_from_gbl_cfg(swd_ctrl_obj * const obj)
{
	ERROR("Instance not initialized\n");
	return -1;
}

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

	if (pdata->is_init) {
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
