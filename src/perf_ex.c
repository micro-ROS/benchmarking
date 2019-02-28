/*****************************************************************
 * file: perf_ex
 * author: Alexandre Malki <alexandremalki@gmail.com>
 * brief: This is file is autogenrated
 *
 *****************************************************************/

#include <debug.h>
#include <message.h>
#include <perf_ex.h>

#include <stdio.h>
#include <libswo/libswo.h>

typedef struct function_info_st function_info;

struct function_info_st {
	char name[32];
	char file[512];

	unsigned int stats;
};

/**
 * @brief  This is the private structure of this processing element
 * @fd_proc This is the fd pointing on the output of the popen to retrieve
 *		the value of the device
 * @is_init Checks if this module is initialized to avoid double init/fini;
 */
typedef struct {
	char *toolchain;
	char path_cmd[1024];
	bool is_init;
	function_info finfo[64];
} perf_ex_private_data;


#if 0
typedef struct {
	char function[32];
	char file[512];
	unsigned int line[32]
	unsigned int 
} exe_info;
#endif

#define PERF_EX_ADDR2LINE_CMD "%s" \
	" -e/home/amalki/prjs/microROS/soft/embedded_soft/baremetal_tests/baremetal-stm32f4/" \
	" -f %d"

#define PERF_EX_SSCANF_FORMAT "%s\n%*s:%*u"
static perf_ex_private_data perf_ex_priv_data;

/**
 * \brief TODO
 */
static int
perf_ex_data_int(processing_obj * const obj, message_obj *const msg)
{
	perf_ex_obj * const perf = (perf_ex_obj * const) obj;
	perf_ex_private_data *pdata = 
				(perf_ex_private_data *) perf ->pdata;
	union libswo_packet *packets = (union libswo_packet *) msg->ptr(msg);
	unsigned int pkt_count = msg->length(msg) / sizeof (union libswo_packet);
	FILE *f_popen;
	char output[128];
	char function[32];
	char file[512];

	for (unsigned int i = 0; i < pkt_count; i++) {
		snprintf(pdata->path_cmd, sizeof(pdata->path_cmd) - 1,
				 PERF_EX_ADDR2LINE_CMD, pdata->toolchain, 0x08000392);
		if (!(f_popen = popen(pdata->path_cmd, "r"))) {
			return -1;
		}

		pclose(f_popen);
		while (fread(output, 1, sizeof(output), f_popen)) {
			if (!sscanf(output, PERF_EX_SSCANF_FORMAT, function)) {
				ERROR("Invalid output");
			}
		}
	}

	return 0;
}

/**
 * \brief TODO
 */
static int
perf_ex_data_out(processing_obj * const obj, message_obj *const msg)
{
	perf_ex_obj * const perf = (perf_ex_obj * const) obj;
	perf_ex_private_data *pdata = 
				(perf_ex_private_data *) perf->pdata;

	return 0;
}

static int
perf_ex_set_tc_gbl_config(perf_ex_obj * const obj)
{
	perf_ex_private_data *pdata = 
				(perf_ex_private_data *) obj->pdata;

	return 0;
}

static int
perf_ex_set_tc(perf_ex_obj * const obj, const char * const path)
{
	perf_ex_private_data *pdata = 
				(perf_ex_private_data *) obj->pdata;
	return 0;
}

static int
perf_ex_set_tc_default(perf_ex_obj * const obj, const char * const path)
{
	WARNING("Not initialized\n");
	return 0;
}

static int perf_ex_set_tc_gbl_config_default(perf_ex_obj * const obj)
{
	WARNING("Not initialized\n");
	return 0;
}

/**
 * \brief TODO
 */
int perf_ex_init(perf_ex_obj *obj)
{
	processing_obj *proc_obj = (processing_obj *) obj;
	perf_ex_private_data *pdata;

	if (perf_ex_priv_data.is_init) {
		ERROR("Cannot initialize more than one instance\n");
		return -1;
	}

	pdata = &perf_ex_priv_data;
	obj->pdata = (void *) &perf_ex_priv_data;
	obj->set_tc_gbl_config = perf_ex_set_tc_gbl_config;
	obj->set_tc = perf_ex_set_tc;
	pdata->is_init = true;

	return 0;
}

/**
 * \brief TODO
 */
int perf_ex_fini(perf_ex_obj *obj)
{
	perf_ex_private_data *pdata = 
				(perf_ex_private_data *) obj->pdata;

	if (!perf_ex_priv_data.is_init) {
		ERROR("Not initialized\n");
		return -1;
	}

	obj->set_tc_gbl_config = perf_ex_set_tc_gbl_config_default;
	obj->set_tc = perf_ex_set_tc_default;

	pdata->is_init = false;

	return 0;
}
