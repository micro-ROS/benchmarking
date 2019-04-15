/*****************************************************************
 * file: itm2mem_info
 * author: Alexandre Malki <alexandremalki@gmail.com>
 * brief: This file gather data from the decoded SWO (ITM trace)
 *	  sort and print the information to a json file format. 
 *
 *****************************************************************/

#include <debug.h>
#include <config.h>
#include <info.h>
#include <itm2mem_info.h>
#include <json.h>
#include <libswo/libswo.h>
#include <message.h>
#include <memory_info.h>
#include <string.h>
#include <tree.h>

#define SSCANF_ALLOC "alloc %ld %x"
#define SSCANF_ADDR_FUNC "%x %s"

typedef struct {
	/** Temporary mem_info buffer. */
	struct mem_info mi[2];
	/** Temp buffer index switch. */
	size_t mi_count;
	/** 
	 * Buffer holding characters from 
	 * the buffer from the ITM trace 
	 * this is used as a circular buffer.
	 */
	char buffer[MESSAGE_BUFFER_SZ_MAX];
	/** Head of the circular buffer. */
	size_t buffer_head;
	/** Tail of the circular buffer. */
	size_t buffer_tail;
	/** Tail of the circular buffer. */
	node_file *nf_head;
	/**
	 * Informing if the structure is initialized to
	 * protect from double init/de-init.
	 */
	bool is_init;
} itm2mem_info_private_data;

static itm2mem_info_private_data itm2mem_info_priv_data;


/**
 * \brief Function splitting the received data into lines and
 * 	processing them and/storing then to the circular buffer.
 * \param char line (between \n) received from the ITM trace.
 * \return a pointer on the begigning of a line. NULL pointer
 * 	returned if an error occured.
 */
static char *
itm2mem_get_next_line(itm2mem_info_private_data * const pdata)
{
	char *pos;
	size_t buffer_head;
	size_t buffer_tail;
	size_t prev_buffer_head;
	size_t move_sz;

	prev_buffer_head = buffer_head = pdata->buffer_head;
	buffer_tail = pdata->buffer_tail;
	buffer_head = pdata->buffer_head;

	pos = strchr(&pdata->buffer[buffer_head], '\n');
	if (!pos && buffer_tail < buffer_head) {
		move_sz = (sizeof(pdata->buffer) - 1) - buffer_head;
		memmove(&pdata->buffer[move_sz], pdata->buffer, pdata->buffer_tail + 1); 
		memmove(pdata->buffer, &pdata->buffer[buffer_head], move_sz); 

		pdata->buffer_head  = 0;
		pdata->buffer_tail += move_sz ;

		return itm2mem_get_next_line(pdata);
	} else if (!pos) {
		return NULL;
	}

	*pos = '\0';
	pos++;
	
	pdata->buffer_head += (size_t) ((uintptr_t)pos - (uintptr_t)&pdata->buffer[buffer_head]);
	pdata->buffer_head %= sizeof(pdata->buffer) - 1;

	return (char *) &pdata->buffer[prev_buffer_head];
}

/**
 * \brief Function parsing the string char received.
 * \param minfo is the memory information (size, ptr etc..).
 * \param char line (between \n) received from the ITM trace.
 */
static void
itm2mem_create_new_mem_info(struct mem_info *minfo, const char *line)
{
	sscanf(line, SSCANF_ALLOC, (size_t *) &minfo->blk_sz,
					 (unsigned int *) &minfo->ptr);
}

/**
 * \brief this function will add append the filled mem_info structure according
 * 		to the depth.
 * \param pdata itm2mem_info private structure.
 */
static void 
itm2mem_add_to_backtrace(struct mem_info *minfo, const char *line, unsigned int depth)
{
	char *strbacktrace = NULL;
	sscanf(line, SSCANF_ADDR_FUNC, (unsigned int *)&minfo->backtrace[depth],
				 		strbacktrace);
	strncpy(minfo->strbacktrace[depth], line, BACKTRACE_STR_MAX_SIZE - 1);

}


/**
 * \brief this function will add append the filled mem_info structure according
 * 		to the depth.
 * \param pdata itm2mem_info private structure.
 * \return the amount of char processed from the ITM trace. 
 * 	-1 if an error occured.
 */
static int
itm2mem_add_to_list(itm2mem_info_private_data *const pdata, unsigned int depth)
{
	if (pdata->mi_count > 0) {
		pdata->mi[pdata->mi_count - 1].size = depth;
		if (mem_info_append_to_list(&pdata->mi[pdata->mi_count - 1])) {
			return -1;	
		}
		memset(&pdata->mi[pdata->mi_count - 1], 0, sizeof(struct mem_info));
		/* Switch to the next buffer */
		pdata->mi_count ^=3;
			
	} else {
		pdata->mi_count = 1;
	}

	return 0;
}

/**
 * \brief this function will fill the mem_info structure according
 * 	to what is received from the ITM trace.
 * \param pdata itm2mem_info private structure.
 * \return the amount of char processed from the ITM trace. 
 * 	-1 if an error occured.
 */
static size_t
itm2mem_parse(itm2mem_info_private_data *const pdata, const char *line)
{
	static unsigned int depth = 0;
	size_t total_sz = -1;

	if (!strncmp(line, "alloc ", sizeof("alloc ") - 1)) {
		if (itm2mem_add_to_list(pdata, depth)) {
			ERROR("Error while adding itm memory info to list\n");
			goto failed_adding_to_list;
		}
		depth = 0;

		itm2mem_create_new_mem_info(&pdata->mi[pdata->mi_count -1],
					    line);
	} else {
		/** case alloc was not met yet */
		if (pdata->mi_count - 1 < 0 ) {
			goto no_alloc_yet;
		}

		itm2mem_add_to_backtrace(&pdata->mi[pdata->mi_count - 1],
					 line, depth);
		depth++;
	}

	total_sz += strlen(line);
failed_adding_to_list:
no_alloc_yet:
	return total_sz;
}

/**
 * \brief this function will fill the mem_info structure according
 * 	to what is received from the ITM trace.
 * \param pdata itm2mem_info private structure.
 * \return the amount of char processed from the ITM trace. 
 * 	-1 if an error occured.
 */
static size_t
itm2mem_fill_info(itm2mem_info_private_data * const pdata)
{
	const char *line;
	size_t res;
	size_t total_sz = 0;

	while ((line = itm2mem_get_next_line(pdata))) {
		if ((res = itm2mem_parse(pdata, line)) < 0) {
			return res;
		}

		total_sz += res;
	}

	return total_sz;
}

/**
 * \brief Function receiving data from the decoder. 
 */
static size_t 
itm2mem_info_data_in(processing_obj * const proc_obj, message_obj *const msg)
{
	itm2mem_info_obj *obj = (itm2mem_info_obj *) proc_obj;
	itm2mem_info_private_data	*pdata = 
				(itm2mem_info_private_data *) obj->pdata;

	union libswo_packet *packets = (union libswo_packet *) msg->ptr(msg);
	unsigned int pkt_count = msg->length(msg) / sizeof (union libswo_packet);
	unsigned int i;

	for (i = 0; i < pkt_count; i++) {
		DEBUG("Size of payload %ld\n", packets[i].inst.size);
		pdata->buffer[pdata->buffer_tail] = (char) packets[i].inst.value;
		pdata->buffer_tail = 
				(pdata->buffer_tail + 1) % (sizeof(pdata->buffer) - 1);
	}

	return itm2mem_fill_info(pdata);
}

/**
 * \brief this funcion is called before the application is stopped.
 * 		it sort the data and print it to json format.
 */
static size_t
itm2_mem_get_list()
{
	node_file *nf = NULL;
	if (!(nf = tree_create(memory_info_get_head()))) {
		return -1;
	}

	if (json_translate_and_write_to_file(nf)) {
		return -1;
	}

	return 1;
}

/**
 * \brief Function to printout the data into the json file. 
 */
static size_t
itm2mem_info_data_out(processing_obj * const proc_obj, message_obj *const msg)
{
	itm2mem_info_obj *obj = (itm2mem_info_obj *) proc_obj;
	itm2mem_info_private_data *pdata = 
				(itm2mem_info_private_data *) obj->pdata;

	if (proc_obj->req_end) {
		return itm2_mem_get_list();

	}
			
	return 0;
}

/**
 * \brief  Initializing the  the processing element.
 */
int itm2mem_info_init(itm2mem_info_obj * const obj)
{
	processing_obj *proc_obj = (processing_obj *) obj;
	itm2mem_info_private_data *pdata;
	cfg_param param = {
			 .section = CFG_SECTION_OUTPUT_FILE,
			 .name = CFG_SECTION_OUTPUT_FILE_PM,
			 .type = CONFIG_STR
	};

	if (processing_init(proc_obj)) {
		return -1;
	}

	proc_obj->data_in  = itm2mem_info_data_in;
	proc_obj->data_out = itm2mem_info_data_out;

	pdata = &itm2mem_info_priv_data;

	memset(pdata->buffer, 0, sizeof(pdata->buffer));
	pdata->mi_count = 0;
	pdata->buffer_head = 0;
	pdata->buffer_tail = 0;
	obj->pdata = (void *) pdata;

	CONFIG_HELPER_GET_STR(&param);
	
	if (!param.found) {
		return -1;
	}

	json_set_output_path(param.value.str);

	param.section = CFG_SECTION_EXT_BIN;
	param.name = CFG_SECTION_EXT_BIN_ELF;
	CONFIG_HELPER_GET_STR(&param);
	
	if (!param.found) {
		return -1;
	}

	tree_set_elf_path(param.value.str);
	tree_set_platform(PLATFORM_NUTTX);

	return 0;
}

/**
 * \brief  De-Initializing the  the processing element.
 */
int itm2mem_info_fini(itm2mem_info_obj * const obj)
{
	return 0;
}


