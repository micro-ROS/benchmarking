/*****************************************************************
 * file: perf_ex.c
 * author: Alexandre Malki <amalki@piap.com>
 * @brief This is the header file of the perf_ex_obj object object.
 * 		more detailed are provided in the source file perf_ex.c.
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

/** Command line format to execute to get the line information */
#define PERF_EX_ADDR2LINE_CMD "%saddr2line" \
	" -e%s" \
	" -f 0x%x"

/** Output format of the addr2line tool */
#define PERF_EX_SSCANF_FORMAT "%s\n%[^:]:%u"

/** Output string, format representing the end of a function in a file */
#define PERF_EX_OUT_END_FUNC "\n]"

/** Output string, format representing the information of a function in a file */
#define PERF_EX_OUT_BEGIN_FUNC "\nfile: %s\nfunction: %s\n" \
				"hits: %u\ndetails: [\n"

/** Output string, format representing the informatio about a line */
#define PERF_EX_OUT_LINE "\t{ hits: %u, line: %u, address: %x},\n"

/** 
 * This structure holds information regarding the line/address 
 * of a sample
 */
struct exe_line_info_st {
	/** Link to the next element of this list. */
	exe_line_info	*next;
	/** Address of the sample */
	unsigned int	addr;
	/** Line of the corresponding address inside the source file */
	unsigned int	line;
	/** Number of occurences */
	unsigned int	hit;
};

/**
 * Performance execution linked likes. On element represent a function located
 * in specific source file. It holds a list of addresses that match this 
 * function.
 */
struct exe_info_list_st {
	/**
	 * The next element of the list, if NULL it means that
	 * the current element is the tail of the list.
	 */
	exe_info_list		*next;
	/**
	 *  The previous element of the list, if NULL it means
	 *  that the current element is the head of the list. 
	 */
	exe_info_list		*prev;
	/** The source file path containing the function. */
	char			file_path[1024];
	/** 
	 * Name of the function contained in the source file locate at
	 *  file_path
	 */
	char			function_name[128];
	/** Number of sample that refere to function_name located in file_path */
	unsigned int		hits;
	/** 
 	 * First line that a sample refers to into the function,
	 * the lines are ordered by increase number.
	 */
	exe_line_info		*line_first;
	/** Last line that  a sample referes to into the function. */
	exe_line_info		*line_last;
};

/**
 * @brief  This is the private structure of this processing element.
 * @fd_proc  * @path_cmd contains the whole command to call to retrieve the address of from
 *		an elf file.
 * @is_init Checks if this module is initialized to avoid double init/fini;
 * @head ;
 */
typedef struct {
	/**
	 * path to the toolchain
	 */
	const char *toolchain;
	/**
	 * This is the fd pointing on the output of the popen to retrieve
	 * the value of the device.
	 */
	const char *elf;
	/**
	 * Command execute by addr2line including option and path to the
	 * ELF file
	 */
	char path_cmd[1024];
	/**
	 * Check if the object is initialized to avoid double init/fini
	 */
	bool is_init;
	/**
	 *  Total number of sample, needed for the statistics
	 */
	unsigned int  total_samples;
	/**
	 *  head of the execution info list
	 */
	exe_info_list *head;
} perf_ex_private_data;

/** Private data needed as a processing object */
static perf_ex_private_data perf_ex_priv_data;

/**
 * @brief  This function is looking for the address element that 
 * 		correspond, within the info struct. If no addresses match,
 *		it will allocate a new structure.
 * @param el Function information element to look into.
 * @param addr Address to look for.
 * @param line Line corresponding to the address.
 * @return A pointer on element that correspond to the address. Or NULL if allocation
 *		fails.
 */
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

/**
 * @brief  This function checks that the function info structure matches 
 * 	the function from where belong the sample.
 * @param info Function info structure.
 * @param function Name of the function correspond to the sample address.
 * @param path Path to the file that contains the function.
 * @return if the name of the function correspond to the function_name and if 
 *		the path name corresponds to the file_path it will return 1. 0
 *		if it does not match.
 */
static int
perf_ex_match_func_and_line(exe_info_list *info, const char * const function,
				const char * const path)
{
	int cmp_func = strncmp(function,info->function_name, sizeof(info->function_name) - 1);
	int cmp_file = strncmp(path, info->file_path, sizeof(info->file_path) - 1);

	return (!cmp_func && !cmp_file);
}

/** 
 * @brief This function will assign a structure to the sample represented by the
 *		path and the function parameter. If no function correspond, it
 *		allocate a new exe_info_list structure to get information from.
 * @param path Path to the source file that correspoond to the sample.
 * @param function Name of the function that correspond to the sample.
 * @return Structure on a exe_info_list corresponding. NULL if allocation fails.
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
		if (perf_ex_match_func_and_line(info->next, function, path)) {
			return info->next;
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

/** 
 * @brief This function is sorting sample, and classifying them into the right
 *		structure.
 * @param pdata Private data holding the head of the list of functions information
 *		structure.
 * @param path Path to the source file that correspond to the sample received.
 * @param addr PC address corresponding to the sample received.
 * @param line Number of the line in the source file that the address 
 *		corresponds to.
 * @return 0 if everything went well, otherwise -1.
 */
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
 * @brief This is the main receiving callback. This callback will receive libswo 
 *		structure samples. This samples carry the PC value. This PC value
 *		will be interpreted by addr2line and store in the corresponding
 *		structure.
 * @param obj Processing obj abstraction.
 * @param msg message containing the information. This is a table of 
 *		union libswo_packet.
 * @return The number of byte written to the output of the forked process addr2line.
 */
static size_t
perf_ex_data_in(processing_obj * const obj, message_obj *const msg)
{
	perf_ex_obj * const perf = (perf_ex_obj * const) obj;
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) perf ->pdata;
	union libswo_packet *packets = (union libswo_packet *) msg->ptr(msg);
	unsigned int pkt_count = msg->length(msg) / sizeof (union libswo_packet);
	size_t n, readd = 0;
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
				PERF_EX_ADDR2LINE_CMD, pdata->toolchain, pdata->elf,
				packets[i].pc_value.pc);

		if (!(f_popen = popen(pdata->path_cmd, "r"))) {
			ERROR("Could not execute %s\n",  pdata->path_cmd);
			return -1;
		}

		n = fscanf(f_popen, PERF_EX_SSCANF_FORMAT, function, file,
				&line);
		if (EOF == n) {
			if (ferror(f_popen)) {
				ERROR("Error while opening reading output of "
					"cmd %s\n", pdata->path_cmd);
			}
		}
		readd += n;

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

/**
 * @brief Running through all the addresses list, and write the information
 * 		to the msg object.
 * @param info function info object holding addresses.
 * @param msg buffer where the output information should be written.
 * @param pos The current position inside the buffer.
 * @return the number of char written to the buffer.
 */
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

/**
 * @brief This function will browse all the list_info function structure
 *		and write the function names/file names addressed and stats.
 * @param obj The processing object pointer abstraction.
 * @param msg buffer where the output information should be written.
 * @return the number of char written to the buffer.
 */
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
 * @brief This function will not write write anything to the output buffer until
 *		the end of processing.
 * @param obj The processing object pointer abstraction.
 * @param msg buffer where the output information should be written.
 * @return the number of char written to the buffer.
 */
static size_t
perf_ex_data_out(processing_obj * const obj, message_obj *const msg)
{
	
	if (obj->req_end) {
		return perf_ex_print(obj, msg);
	}

	return 0;
}

/**
 * @brief This function will save the name of the elf path found in the config 
 *		object. It assumes that the object was first initialized before
 *		using this function.
 * @param obj The processing object pointer abstraction.
 * @pre The config object must be initialized.
 * @return  0 if successfully found the path, otherwise.
 */
static int
perf_ex_set_elf_gbl_config(perf_ex_obj * const obj)
{
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) obj->pdata;
	cfg_param param = {
				.section = CFG_SECTION_EXT_BIN,
				.type = CONFIG_STR,
				.name = CFG_SECTION_EXT_BIN_ELF,
			  };

	pdata->elf = CONFIG_HELPER_GET_STR(&param);

	if (!param.found) {
		return -1;
	}

	return 0;
}


/**
 * @brief This function will save the path to the elf file . The
 *		elf path is taken from path parameter.
 * @param obj The processing object pointer abstraction.
 * @param path Path to the toolchain. It assumes the path is non-null.
 * @return  0 if successfully found the path, -1 otherwise.
 * @pre The path is non-null
 */
static int
perf_ex_set_elf(perf_ex_obj * const obj, const char * path)
{
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) obj->pdata;

	pdata->toolchain = path;
	return 0;
}

/**
 * @brief This function will save the name of the toolchain path+name
 * 		found in the config object. It assumes that the object was first
		initialized before using this function.
 * @param obj The processing object pointer abstraction.
 * @pre The config object must be initialized.
 * @return  0 if successfully found the path, -1 otherwise.
 */
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

	if (!param.found) {
		return -1;
	}

	return 0;
}


/**
 * @brief This function will save the name of the toolchain path+name. The
 *		toolchain path+name is taken from path parameter.
 * @param obj The processing object pointer abstraction.
 * @param path Path to the toolchain. It assumes the path is non-null.
 * @return  0 if successfully found the path, -1 otherwise.
 * @pre The path is non-null
 */
static int
perf_ex_set_tc(perf_ex_obj * const obj, const char * path)
{
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) obj->pdata;

	pdata->toolchain = path;
	return 0;
}

/**
 * @brief Default function in case of acessing a deinitialized object.
 * @param obj The processing object pointer abstraction.
 * @param path Path to the toolchain. It assumes the path is non-null.
 * @return  0.
 */
static int
perf_ex_set_elf_default(perf_ex_obj * const obj, const char * const path)
{
	WARNING("Not initialized\n");
	return 0;
}

/**
 * @brief Default function in case of acessing a deinitialized object.
 * @param obj The processing object pointer abstraction.
 * @return  0.
 */
static int perf_ex_set_elf_gbl_config_default(perf_ex_obj * const obj)
{
	WARNING("Not initialized\n");
	return 0;
}


/**
 * @brief Default function in case of acessing a deinitialized object.
 * @param obj The processing object pointer abstraction.
 * @param path Path to the toolchain. It assumes the path is non-null.
 * @return  0.
 */
static int
perf_ex_set_tc_default(perf_ex_obj * const obj, const char * const path)
{
	WARNING("Not initialized\n");
	return 0;
}

/**
 * @brief Default function in case of acessing a deinitialized object.
 * @param obj The processing object pointer abstraction.
 * @return  0.
 */
static int perf_ex_set_tc_gbl_config_default(perf_ex_obj * const obj)
{
	WARNING("Not initialized\n");
	return 0;
}

int perf_ex_init(perf_ex_obj * const obj)
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

	obj->set_elf_gbl_config = perf_ex_set_elf_gbl_config;
	obj->set_elf = perf_ex_set_elf;

	proc_obj->data_in = perf_ex_data_in;
	proc_obj->data_out = perf_ex_data_out;

	pdata->is_init = true;
	pdata->toolchain = NULL;
	pdata->head = (exe_info_list *) calloc(1, sizeof(exe_info_list));

	return 0;
}

int perf_ex_fini(perf_ex_obj * const obj)
{
	perf_ex_private_data *pdata =
				(perf_ex_private_data *) obj->pdata;

	if (!perf_ex_priv_data.is_init) {
		ERROR("Not initialized\n");
		return -1;
	}

	obj->set_tc_gbl_config = perf_ex_set_tc_gbl_config_default;
	obj->set_tc = perf_ex_set_tc_default;

	obj->set_elf_gbl_config = perf_ex_set_elf_gbl_config_default;
	obj->set_elf = perf_ex_set_elf_default;

	if (pdata->head)
		free(pdata->head);

	pdata->is_init = false;
	return 0;
}
