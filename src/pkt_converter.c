#include <pkt_converter.h>
#include <common-macros.h>
#include <debug.h>

#include <cJSON.h>

#include <string.h>

enum pkt_type_value {
	PKT_TYPE_STR,
	PKT_TYPE_CHAR,
	PKT_TYPE_SIZE,
	PKT_TYPE_UNSIGNED,
	PKT_TYPE_HEX,
} ;

struct packet_value_info {
	/** Name of the value */
	const char * const	name;

	/** Type of the value i.e char, string, unsigned int etc.. */
	enum pkt_type_value 	type;

	/** callback to get the value */
	void (*handle_pkt) (const union libswo_packet *pkt, void *data);
};

struct packet_type_info {
	/** Name of the packet */
	const char * const	 name;

	/** Number of value that can be taken from the packet */
	unsigned char		 count;

	/** Values representation information*/
	struct packet_value_info values[5];
};

/* Data getter */
/* HW info */
static inline void handle_hw_pkt_address(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->hw.address;
}

static inline void handle_hw_pkt_value(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->hw.value;
}

static inline void handle_hw_pkt_hw_value(const union libswo_packet *pkt,
					 void *data)
{
	char *data_c = (char *) data;
	*data_c =  (char) pkt->hw.value;
}

static inline void handle_hw_pkt_size(const union libswo_packet *pkt,
				      void *data)
{
	size_t *data_sz = (size_t *) data;
	*data_sz = (size_t) (pkt->hw.size - 1);
}

/* info instrumentation */
static inline void handle_instrumentation_pkt_address
		(const union libswo_packet *pkt, void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->hw.address;
}

static inline void handle_instrumentation_pkt_value
		(const union libswo_packet *pkt, void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->hw.value;
}

static inline void handle_instrumentation_pkt_hw_value
		(const union libswo_packet *pkt, void *data)
{
	char *data_c = (char *) data;
	*data_c =  (char) pkt->hw.value;
}

static inline void handle_instrumentation_pkt_size
		(const union libswo_packet *pkt, void *data)
{
	size_t *data_sz = (size_t *) data;
	*data_sz = (size_t) (pkt->hw.size - 1);
}

/* info EXT */
static inline void handle_extension_pkt_extension
		(const union libswo_packet *pkt, void *data)
{
	char **src = (char **) data;

	switch (pkt->ext.source) {
		case LIBSWO_EXT_SRC_ITM:
			*src = "ITM";
			break;
		case LIBSWO_EXT_SRC_HW:
			*src = "HW";
			break;
		default:
			WARNING("Extension pkt with invalid source: %u\n",
				pkt->ext.source);
			*src = "No specific ext";
			return;
		}
}

static inline void handle_extension_pkt_value
		(const union libswo_packet *pkt, void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->ext.value;
}

/* sync */
static inline void handle_sync_pkt_size(const union libswo_packet *pkt,
				      void *data)
{
	size_t *data_u = (size_t *) data;
	if (pkt->sync.size % 8)
		*data_u = (size_t)pkt->sync.size;
	else
		*data_u = (size_t)pkt->sync.size / 8;
}

/* LTS */
static inline void handle_local_timestamp_pkt_relation(
	const union libswo_packet *pkt,
	void *data)
{
	char **tc = (char **) data;

	switch (pkt->lts.relation) {
	case LIBSWO_LTS_REL_SYNC:
		*tc = "synchronous";
		break;
	case LIBSWO_LTS_REL_TS:
		*tc = "timestamp delayed";
		break;
	case LIBSWO_LTS_REL_SRC:
		*tc = "data delayed";
		break;
	case LIBSWO_LTS_REL_BOTH:
		*tc = "data and timestamp delayed";
		break;
	default:
		WARNING("Local timestamp packet with invalid relation: %u.",
			pkt->lts.relation);
		*tc = "unknown timestamp";
		return;
	}
}

static inline void handle_local_timestamp_pkt_value(
	const union libswo_packet *pkt,
	void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->lts.value;
}

/** Global Timestamp 1 */
static inline void handle_gts_one_pkt_wrap(const union libswo_packet *pkt,
	void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->gts1.wrap;
}

static inline void handle_gts_one_pkt_clkch(const union libswo_packet *pkt,
	void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u= pkt->gts1.clkch;
}

static inline void handle_gts_one_pkt_value(const union libswo_packet *pkt,
	void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->gts1.value;
}

/* Global Timestamp 2 */
static inline void handle_gts_two_pkt_value(const union libswo_packet *pkt,
	void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->gts2.value;
}

/* DWT event cnt */
static inline void handle_evt_pkt_cpi(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->evtcnt.cpi;
}

static inline void handle_evt_pkt_exc(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->evtcnt.exc;
}

static inline void handle_evt_pkt_sleep(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->evtcnt.sleep;
}

static inline void handle_evt_pkt_LSU(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->evtcnt.lsu;
}

static inline void handle_evt_pkt_fold(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->evtcnt.fold;
}

/* DWT Exection Trace  */
static inline void handle_exctrc_pkt_address(const union libswo_packet *pkt,
					 void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (unsigned int) pkt->hw.value;
}

/*
 * Exception names according to section B1.5 of ARMv7-M Architecture Reference
 * Manual.
 */
static const char *exception_names[] = {
	"Thread",
	"Reset",
	"NMI",
	"HardFault",
	"MemManage",
	"BusFault",
	"UsageFault",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"SVCall",
	"Debug Monitor",
	"Reserved",
	"PendSV",
	"SysTick"
};

#define NUM_EXCEPTION_NAMES	(ARRAY_SIZE(exception_names))
static inline void handle_exctrc_pkt_function(const union libswo_packet *pkt,
					      void *data)
{
	char **func = (char **) data;

	switch (pkt->exctrc.function) {
	case LIBSWO_EXCTRC_FUNC_ENTER:
		*func = "enter";
		break;
	case LIBSWO_EXCTRC_FUNC_EXIT:
		*func = "exit";
		break;
	case LIBSWO_EXCTRC_FUNC_RETURN:
		*func = "return";
		break;
	default:
		*func = "reserved";
	}
}

static inline void handle_exctrc_pkt_exception(const union libswo_packet *pkt,
					       void *data)
{
	static char buf[32];
	unsigned short exception;
	const char **name = (const char **) data;

	exception = pkt->exctrc.exception;
	/*TODO adapting CONCUSION */
	if (exception < NUM_EXCEPTION_NAMES) {
		*name = exception_names[exception];
	} else {
		snprintf(buf, sizeof(buf) - 1, "External interrupt %lu",
			exception - NUM_EXCEPTION_NAMES);
		*name = buf;
	}
}

/** Periodic pc */
static inline void handle_pc_sample_pkt_value(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	if (!pkt->pc_sample.sleep)
		*data_u = (pkt->pc_sample.pc);
}

/** PC triggerd */
static inline void handle_pc_value_pkt_value(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (pkt->pc_value.pc);
}

static inline void handle_pc_value_pkt_comp(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	/* TODO FIX value to get pc  */
	*data_u = (pkt->pc_value.cmpn);
}

/* Addr offset */
static inline void handle_addr_off_pkt_comp(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->addr_offset.cmpn;
}

static inline void handle_addr_off_pkt_offset(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = (pkt->addr_offset.offset);
}


/* Data value */
static inline void handle_data_value_pkt_comp(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->data_value.cmpn;
}

static inline void handle_data_value_pkt_value(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->data_value.data_value;
}

static inline void handle_data_value_pkt_rw(const union libswo_packet *pkt,
					   void *data)
{
	unsigned int *data_u = (unsigned int *) data;
	*data_u = pkt->data_value.wnr;
}

static inline void handle_data_value_pkt_size(const union libswo_packet *pkt,
					   void *data)
{
	size_t *data_u = (size_t *) data;
	*data_u = (size_t) (pkt->data_value.size - 1);
}

#define PKT_VALUE_HELPER(kind, nameval, typ) \
	{ .name = #nameval, .type = typ, \
		.handle_pkt = STRINGIFY(STRINGIFY(handle_, kind), STRINGIFY(_pkt_, nameval)) \
	}

static struct packet_type_info ptis[] = {
	[LIBSWO_PACKET_TYPE_UNKNOWN] = {
		.name = "unknown",
		.count = 0,
	},
	[LIBSWO_PACKET_TYPE_HW] = {
		.name = "hw",
		.count = 4,
		.values = {
			PKT_VALUE_HELPER(hw, address, PKT_TYPE_HEX),
			PKT_VALUE_HELPER(hw, value, PKT_TYPE_HEX),
			PKT_VALUE_HELPER(hw, hw_value, PKT_TYPE_CHAR),
			PKT_VALUE_HELPER(hw, size, PKT_TYPE_SIZE),
		}
	},
	[LIBSWO_PACKET_TYPE_INST] = {
		.name = "instrumentation",
		.count = 4,
		.values = {
			PKT_VALUE_HELPER(instrumentation, address, PKT_TYPE_HEX),
			PKT_VALUE_HELPER(instrumentation, value, PKT_TYPE_HEX),
			PKT_VALUE_HELPER(instrumentation, hw_value, PKT_TYPE_CHAR),
			PKT_VALUE_HELPER(instrumentation, size, PKT_TYPE_SIZE)
		}
	},
	[LIBSWO_PACKET_TYPE_OF] = {
		.name = "overflow",
		.count = 0,
	},
	[LIBSWO_PACKET_TYPE_EXT] = {
		.name = "extension",
		.count = 2,
		.values = {
			PKT_VALUE_HELPER(extension, extension, PKT_TYPE_STR),
			PKT_VALUE_HELPER(extension, value, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_SYNC] = {
		.name = "sync",
		.count = 1,
		.values = {
			PKT_VALUE_HELPER(sync, size, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_LTS] = {
		.name = "local_timestamp",
		.count = 2,
		.values = {
			PKT_VALUE_HELPER(local_timestamp, relation, PKT_TYPE_STR),
			PKT_VALUE_HELPER(local_timestamp, value, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_GTS1] = {
		.name = "global timestamp (gts1)",
		.count = 3,
		.values = {
			PKT_VALUE_HELPER(gts_one, wrap, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(gts_one, clkch, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(gts_one, value, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_GTS2] = {
		.name = "global timestamp (gts2)",
		.count = 1,
		.values = {
			PKT_VALUE_HELPER(gts_two, value, PKT_TYPE_HEX ),
		}
	},
	[LIBSWO_PACKET_TYPE_DWT_EVTCNT] = {
		.name = "event counter",
		.count = 5,
		.values = {
			PKT_VALUE_HELPER(evt, cpi, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(evt, exc, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(evt, sleep, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(evt, LSU, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(evt, fold, PKT_TYPE_UNSIGNED),
		}
	},
	[LIBSWO_PACKET_TYPE_DWT_EXCTRC] = {
		.name = "Exception Trace",
		.count = 2,
		.values = {
			PKT_VALUE_HELPER(exctrc, function, PKT_TYPE_STR),
			PKT_VALUE_HELPER(exctrc, exception, PKT_TYPE_STR),
		}
	},
	[LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE] = {
		.name = "pc sample",
		.count = 1,
		.values = {
			PKT_VALUE_HELPER(pc_sample, value, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_DWT_PC_VALUE] = {
		.name = "pc comp",
		.count = 2,
		.values = {
			PKT_VALUE_HELPER(pc_value, comp, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(pc_value, value, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_DWT_ADDR_OFFSET] = {
		.name = "Data Trace address offset",
		.count = 2,
		.values = {
			PKT_VALUE_HELPER(addr_off, comp, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(addr_off, offset, PKT_TYPE_HEX),
		}
	},
	[LIBSWO_PACKET_TYPE_DWT_DATA_VALUE] = {
		.name = "Data Trace address offset",
		.count = 4,
		.values = {
			PKT_VALUE_HELPER(data_value, comp, PKT_TYPE_UNSIGNED),
			PKT_VALUE_HELPER(data_value, rw, PKT_TYPE_CHAR),
			PKT_VALUE_HELPER(data_value, value, PKT_TYPE_HEX),
			PKT_VALUE_HELPER(data_value, size, PKT_TYPE_SIZE),
		}
	},
};

static size_t pkt_write_to_cjson(pkt_to_form * const ptf,
			      const union libswo_packet *pkt)
{
	struct packet_type_info  *pti = &ptis[pkt->type];
	struct packet_value_info *pvi;
	char json_char[32];
	cJSON *json_root;
	cJSON *json_value;
	size_t json_sz, total_sz = 0;
	unsigned int json_nbr;
	char *json_str = NULL;

	if (!pti->name) {
		return 0;
	}

	if (!(json_root = cJSON_AddObjectToObject(ptf->root, pti->name))) {
		ERROR("Creating Node root node packet\n");
		return -1;
	}

	if (!cJSON_AddStringToObject(json_root, "packet", pti->name)) {
		ERROR("Could not set not packet name\n");
		return -1;
	}

	for (unsigned int i=0; i<pti->count; i++) {
		pvi = &pti->values[i];

		switch (pvi->type) {
			case PKT_TYPE_CHAR:
				json_char[1] = '\0';
				pvi->handle_pkt(pkt, json_char);
				json_value =
					cJSON_CreateString(json_char);
			break;
			case PKT_TYPE_STR:
				pvi->handle_pkt(pkt, &json_str);
				json_value =
					cJSON_CreateString(json_str);
			break;
			case PKT_TYPE_SIZE:
				pvi->handle_pkt(pkt, &json_sz);
				json_value =
					cJSON_CreateNumber((float) json_sz);
			break;
			case PKT_TYPE_UNSIGNED:
				pvi->handle_pkt(pkt, &json_nbr);
				json_value =
					cJSON_CreateNumber((float) json_nbr);
			break;
			case PKT_TYPE_HEX:
				/**
				 * Hex are handled a unsigned in and then as
				 * printed out as string to keep the format
				 * 0xabcdef01
				 */
				pvi->handle_pkt(pkt, &json_nbr);
				snprintf(json_char, sizeof(json_char),
					 "0x%08x", json_nbr);
				json_value =
					cJSON_CreateString(json_char);
			break;
			default:
				ERROR("Not a valid type of pkt value\n");
				return -1;
			break;
		}

		if (!json_value) {
			ERROR("Error while creating json element %s\n",
			      pti->name);
			return -1;
		}

		cJSON_AddItemToObject(json_root, pvi->name, json_value);
		total_sz +=  sizeof(cJSON);
	}

	return 0;
}

size_t pkt_convert(pkt_to_form * const ptf, message_obj * const obj)
{
	union libswo_packet *packets = (union libswo_packet *) obj->ptr(obj);
	unsigned int pkt_count = obj->length(obj) / sizeof (union libswo_packet);


	DEBUG("Number of packet received %d\n", pkt_count);
	if (!pkt_count) {
		WARNING("Buffer underflow\n");
		return -1;
	}

	for (unsigned int i = 0; i < pkt_count; i++) {
		if ((ptf->fmt > PKT_CONVERTER_MYSQL) ||
		    (ptf->fmt < PKT_CONVERTER_CJSON)) {
			ERROR("Invalid type of output conversion\n");
			return -1;
		}

		switch (ptf->fmt) {
		case PKT_CONVERTER_CJSON:
			if (pkt_write_to_cjson(ptf, &packets[i])) {
				return -1;
			}
		break;
		default:
			WARNING("Type not implemented\n");
		break;
		}
	}

	return 0;
}
