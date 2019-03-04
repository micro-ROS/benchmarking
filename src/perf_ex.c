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
#include <string.h>
#include <libswo/libswo.h>
#include <malloc.h>

typedef struct exe_info_list_st exe_info_list;
typedef struct exe_line_info_st exe_line_info;

struct exe_line_info_st {
	exe_line_info	*next;

	unsigned int	addr;
	unsigned int	line;
	unsigned int	hit;
};

struct exe_info_list_st {
	exe_info_list		*next;
	exe_info_list		*prev;

	char			file_path[1024];
	char			function_name[128];

	unsigned int		hits;
	exe_line_info		*line_first;
	exe_line_info		*line_last;
};

/**
 * @brief  This is the private structure of this processing element
 * @fd_proc This is the fd pointing on the output of the popen to retrieve
 *		the value of the device
 * @is_init Checks if this module is initialized to avoid double init/fini;
 */
typedef struct {
	const char *toolchain;
	const char *elf_path;
	char path_cmd[1024];
	bool is_init;

	unsigned int  total_samples;
	exe_info_list *head;
} perf_ex_private_data;



#define PERF_EX_ADDR2LINE_CMD "%saddr2line" \
	" -e/home/amalki/prjs/microROS/soft/embedded_soft/baremetal_tests/baremetal-stm32f4/main.elf" \
	" -f 0x%x"

#define PERF_EX_SSCANF_FORMAT "%s\n%[^:]:%u"

#define PERF_EX_OUT_END_FUNC "\n]"
#define PERF_EX_OUT_BEGIN_FUNC "\nfile: %s\nfunction: %s\n" \
				"hits: %u\ndetails: [\n"
#define PERF_EX_OUT_LINE "\t{ hits: %u, line: %u, address: %x},\n"

static perf_ex_private_data perf_ex_priv_data;

static exe_line_info *
perf_ex_find_addr_info(exe_info_list *el, unsigned int addr, unsigned int line)
{
	exe_line_info *linfo = el->line_first;
	exe_line_info *linfo_prev = NULL, *linfo_new;

	while (linfo) {
		if (addr == linfo->addr) {
			return linfo;
		} else if (addr < linfo->addr) {
			break;
		}

		linfo_prev = linfo;
		linfo = linfo->next;
	}

	linfo_new = (exe_line_info *) calloc(1, sizeof(exe_line_info));
	if (!linfo_new) {
		ERROR("Could not allocate memory\n");
		return NULL;
	}

	linfo_new->line = line;
	linfo_new->addr = addr;

	/* it seems like there are no first line */
	if (!el->line_first) {
		el->line_first = linfo_new;
		el->line_last = linfo_new;
		return linfo_new;
	}

	/* In the case this is the last element of the addresses */
	if (!linfo) {
		linfo_prev->next = linfo_new;
		el->line_last = linfo_new;
	} else {
		linfo_new->next = linfo;
		if (linfo == el->line_first)
			 el->line_first = linfo_new;

		if (linfo_prev)
			linfo_prev->next = linfo_new;
	}

	return linfo_new;
}

static int
perf_ex_match_func_and_line(exe_info_list *info, const char * const function,
				const char * const path)
{
	int cmp_func = strncmp(function,info->function_name, sizeof(info->function_name) - 1);
	int cmp_file = strncmp(path, info->file_path, sizeof(info->file_path) - 1);

	return (!cmp_func && !cmp_file);
}

/** 
 * @brief This function will assign a structure to the function.
 */
static exe_info_list *
perf_ex_update_function_info(perf_ex_private_data *pdata, exe_info_list *info,
				const char * const path,
				const char * const function)
{
	exe_info_list *rinfo;

	if (perf_ex_match_func_and_line(info, function, path)) {
		return info;
	}

	if (info->next) {
		if (perf_ex_match_func_and_line(info->prev, function, path)) {
			return info->prev;
		}
	}

	rinfo = (exe_info_list *) calloc(1, sizeof(exe_info_list));
	if (!rinfo) {
		ERROR("Could not allocate memory\n");
		return NULL;
	}

	strncpy(rinfo->file_path, path, sizeof(rinfo->file_path) - 1);
	strncpy(rinfo->function_name, function, sizeof(rinfo->function_name) - 1);

	return rinfo;
}

static int
perf_ex_find_and_update_info(perf_ex_private_data *pdata, const char * const path,
			const char * const function,  unsigned int addr,
			unsigned int line)
{
	exe_info_list *pinfo, *info = pdata->head;
	exe_line_info *linfo;
	int cmp_function;
	int cmp_path;
	int rc = 0;
	bool found = false;

	while (info) {
		if (!info->line_first) {
			/* This is the first time this element is filled */
			strncpy(info->file_path, path, sizeof(info->file_path) - 1);
			strncpy(info->function_name, function, sizeof(info->function_name) - 1);
		} else if (info->line_first->addr > addr) {
			info = perf_ex_update_function_info(pdata, info, path, function);
			/* Try to find all the element that match the address and line*/
		} else {
			if (!perf_ex_match_func_and_line(info, function, path)) {
				pinfo = info;
				info = info->next;
				continue;
			}
		}

		found = true;
		break;
	}

	if (!found) {
		/** If we land here, this means that not element were matching the address.
		 *  We need to create a new one
		 */
		info = (exe_info_list *) calloc(1, sizeof(exe_info_list));
		if (!info) {
			ERROR("Could not allocate memory\n");
			return -1;
		}

		strncpy(info->file_path, path, sizeof(info->file_path) - 1);
		strncpy(info->function_name, function, sizeof(info->function_name) - 1);
		pinfo->next = info;
		info->prev = pinfo;
	}

	linfo = perf_ex_find_addr_info(info, addr, line);
	if (!linfo) {
		return -1;
	}

	info->hits++;
	linfo->hit++;

	return 0;
}

/**
 * \brief TODO
 */
static size_t
perf_ex_data_in(processing_obj * const obj, message_obj *const msg)
{
	perf_ex_obj * const perf = (perf_ex_obj * const) obj;
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) perf ->pdata;
	union libswo_packet *packets = (union libswo_packet *) msg->ptr(msg);
	unsigned int pkt_count = msg->length(msg) / sizeof (union libswo_packet);
	size_t readd = 0;
	unsigned int line;
	FILE *f_popen;
	char function[32];
	char file[512];

	if (!pdata->toolchain) {
		ERROR("Provide a valid toolchain\n");
		return -1;
	}

	if (!pkt_count) {
		WARNING("No packets \n");
		return -1;
	}

	for (unsigned int i = 0; i < pkt_count; i++) {
		snprintf(pdata->path_cmd, sizeof(pdata->path_cmd) - 1,
				PERF_EX_ADDR2LINE_CMD, pdata->toolchain,
				packets[i].pc_value.pc);

		if (!(f_popen = popen(pdata->path_cmd, "r"))) {
			ERROR("Could not execute %s\n",  pdata->path_cmd);
			return -1;
		}

		readd += fscanf(f_popen, PERF_EX_SSCANF_FORMAT, function, file,
				&line);
		if (EOF == readd) {
			if (ferror(f_popen)) {
				ERROR("Error while opening reading output of "
					"cmd %s\n", pdata->path_cmd);
			}
		}

		if (!strncmp(function, "??", sizeof(function) - 1)) {
			WARNING("Cannot find function at address %x\n",
				 packets[i].pc_value.pc);
			continue;
		}

		if (pclose(f_popen)) {
			ERROR("Error while executing %s\n", pdata->path_cmd);
			return -1;
		}

		if (perf_ex_find_and_update_info(pdata, file, function,
						 packets[i].pc_value.pc, line)) {
			return -1;
		}
		pdata->total_samples++;
	}

	return readd;
}

static size_t 
perf_ex_print_addresses(exe_info_list * info, message_obj *const msg,
				size_t pos)
{
	exe_line_info *linfo = info->line_first;
	exe_line_info *tofree;
	size_t totlen = msg->total_len(msg);
	size_t new_pos = 0;

	while (linfo) {
		new_pos += snprintf(&msg->ptr(msg)[pos + new_pos], totlen - pos,
				PERF_EX_OUT_LINE, linfo->hit,
				linfo->line, linfo->addr);

		tofree = linfo;
		linfo = linfo->next;
		free(tofree);
	}

	info->line_first = NULL;
	info->line_last = NULL;

	return new_pos;
}

static size_t
perf_ex_print(processing_obj * const obj, message_obj *const msg)
{
	perf_ex_obj * const perf = (perf_ex_obj * const) obj;
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) perf->pdata;
	exe_info_list *info, *tofree;
	size_t pos = 0;
	size_t totlen = msg->total_len(msg);
	unsigned func_hits;

	info = pdata->head;
	while(info) {
		pos += snprintf(&msg->ptr(msg)[pos], totlen - pos,
				PERF_EX_OUT_BEGIN_FUNC, info->file_path,
				info->function_name, info->hits);
		pos += perf_ex_print_addresses(info, msg, pos);
		pos += snprintf(&msg->ptr(msg)[pos], totlen - pos,
				PERF_EX_OUT_END_FUNC);

		tofree = info;
		info = info->next;
		free(tofree);
	}

	pdata->head = NULL;
	msg->set_length(msg, pos);
	ERROR("Message gotten %s\n", msg->ptr(msg));
	return pos;
}

/**
 * \brief TODO
 */
static size_t
perf_ex_data_out(processing_obj * const obj, message_obj *const msg)
{
	
	if (obj->req_end) {
		return perf_ex_print(obj, msg);
	}

	return 0;
}

static int
perf_ex_set_tc_gbl_config(perf_ex_obj * const obj)
{
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) obj->pdata;
	cfg_param param = {
				.section = CFG_SECTION_EXT_BIN,
				.type = CONFIG_STR,
				.name = CFG_SECTION_EXT_BIN_TC,
			  };

	pdata->toolchain = CONFIG_HELPER_GET_STR(&param);
	return pdata->toolchain == NULL ? -1: 0;
}

static int
perf_ex_set_tc(perf_ex_obj * const obj, const char * path)
{
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) obj->pdata;

	pdata->toolchain = path;
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

	if (processing_init(proc_obj)) {
		return -1;
	}

	pdata = &perf_ex_priv_data;

	obj->pdata = (void *) &perf_ex_priv_data;
	obj->set_tc_gbl_config = perf_ex_set_tc_gbl_config;
	obj->set_tc = perf_ex_set_tc;

	proc_obj->data_in = perf_ex_data_in;
	proc_obj->data_out = perf_ex_data_out;

	pdata->is_init = true;
	pdata->toolchain = NULL;
	pdata->head = (exe_info_list *) calloc(1, sizeof(exe_info_list));

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

	if (pdata->head)
		free(pdata->head);

	pdata->is_init = false;
	return 0;
}
