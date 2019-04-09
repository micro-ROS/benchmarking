/**
 * @file decoder_swo.h
 * @brief	Source file decoding the data feed from the UART. Currenlty based
 *		on the libswo library.
 * @author	Alexandre Malki <amalki@piap.pl>
 */
#include <decoder_swo.h>
#include <common-macros.h>
#include <debug.h>

#include <libswo/libswo.h>

#include <stdbool.h>
#include <string.h>

#define DECODER_SWO_BUFFER_MAX_LEN	4096

/** Internal private data used to keep track of the number of packets decodded
 * 	and if the element was initialized or not.
 */
typedef struct {
	/** Context needed by the libswo library. */
	struct libswo_context *swo_ctx;
	/**
	 * Filter callback set upon init depending on
	 * the session's type.
	 */
	bool (*filter_cb) (const union libswo_packet *packet);
	/** Number of packet decoded on last decoding sessionl */
	unsigned int cur_packet_decoded;
	/** Number of packet decoded since the start of the applicationl */
	unsigned int tot_packet_decoded;
	/**
	 * Indicating if yes or not the object is used or not, avoid double
	 * init/fini on the same object.
	 */ 
	bool is_used;
	/**
	 * Internal message object.
	 */
	message_obj msg;
} decoder_swo_priv_data;

/** Instantiation of the only decoder that can be runned */
static decoder_swo_priv_data decoder_swo_pdata;

/** Buffer holding number of packet to hold. */
static union libswo_packet packets_decoded[DECODER_SWO_BUFFER_MAX_LEN / sizeof(union libswo_packet)];

/**
 * @brief The default packet handler, here for debug only.
 * @param packet Information about the SWD packet.
 * @param c counter value linke to the kind of packet.
 * @param s string packet name.
 * @sa libswo.h
 */
static void default_packet_handler(const union libswo_packet *packet,
				   unsigned int c, const char *s)
{
#if 0
	/** TODO implement a debug filter configurable by command line */
	if (packet->type == LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE ||
		packet->type == LIBSWO_PACKET_TYPE_DWT_PC_VALUE)

#endif
		DEBUG("Message %d, %s \n", c, s);
}


#define str(s)	#s
#define set_handler(pkt_type, handler) 		\
		[pkt_type] = { 			\
			.pkt_cb = handler, 	\
			.count = 0,		\
			.name = str(pkt_type)	\
		}
/**  This structure holds the link between the libswo packet and the callback
 *		that will called.
 *		Currently this is only used for debug
 */
static struct {
	/** Callback to call when decoded packet match */
	void (*pkt_cb) (const union libswo_packet *packet,
		       	unsigned int c, const char *s);
	/** Number of type a decoded packet type matches */
	unsigned int count;
	/** Name of the packet taken from the LIBSWO type defines */
	char *name;
} default_handlers[] = {
	set_handler(LIBSWO_PACKET_TYPE_UNKNOWN, 	default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_SYNC, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_INST, 	 	default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_OF, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_EXT, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_LTS, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_GTS1, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_GTS2, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_HW, 		default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_DWT_EVTCNT, 	default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_DWT_EXCTRC, 	default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE, 	default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_DWT_PC_VALUE, 	default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_DWT_ADDR_OFFSET, default_packet_handler),
	set_handler(LIBSWO_PACKET_TYPE_DWT_DATA_VALUE,  default_packet_handler),
};

/**
 * @brief This is the filter callback for itm message.
 * 		The filter is used for memory analysis.
 * @return false if the data does not fit the filter,
 * 		true otherwise.
 */
static bool filter_itm(const union libswo_packet *packet)
{
	if (!((1 << LIBSWO_PACKET_TYPE_INST) & (1 << packet->type)))
		return false;

	return true;
}

/**
 * @brief This is the filter callback for program counter value.
 * 		The filter is used for performance analysis.
 * @return false if the data does not fit the filter,
 * 		true otherwise.
 */
static bool filter_pc(const union libswo_packet *packet)
{
	if (!((1 << LIBSWO_PACKET_TYPE_DWT_PC_SAMPLE |
			 1 << LIBSWO_PACKET_TYPE_DWT_PC_VALUE)	& (1 << packet->type)))
		return false;

	return true;
}

/** 
 * @brief This is the general packet callback. This callback will be called each
 *		time a packet is decoded.
 * @param ctx libswo related context.
 * @param packet The actual decoded packet.
 * @param user_data This is an abstract pointer. In our case this is the private
 *		structure.
 */
static int packet_cb(struct libswo_context *ctx,
		const union libswo_packet *packet, void *user_data)
{
	(void)ctx;
	unsigned int count;
	char *name;
	decoder_swo_priv_data *pdata =
		(decoder_swo_priv_data *) user_data;


#ifdef DEBUG
	count = default_handlers[packet->type].count;
	name = default_handlers[packet->type].name;
	default_handlers[packet->type].pkt_cb(packet, count, name);
#endif
	/** Default filters */
	if (((packet->type < LIBSWO_PACKET_TYPE_UNKNOWN) ||
		(packet->type > LIBSWO_PACKET_TYPE_HW)) &&
		((packet->type < LIBSWO_PACKET_TYPE_DWT_EVTCNT) ||
		(packet->type > LIBSWO_PACKET_TYPE_DWT_DATA_VALUE))) {

		WARNING("Unhandled packet type value %d\n", packet->type);
		return true;
	}

	if (((1 << LIBSWO_PACKET_TYPE_UNKNOWN) & (1 << packet->type)) ||
		((1 <<	LIBSWO_PACKET_TYPE_SYNC) & (1 << packet->type)))
		return true;


	/** Dedicated filter depending on the packet */
	if (!pdata->filter_cb(packet)) {
		return true;
	}

	memcpy(&packets_decoded[pdata->cur_packet_decoded], packet, sizeof(*packet));
	pdata->cur_packet_decoded++;

	return true;
}

/**
 * @brief The function that feeds the decoder.
 * @param obj The generic processing object.
 * @param msg The message data coming form the above level.
 * @return The number of written bytes written, -1 on error.
 */
static size_t decoder_swo_data_in(processing_obj * const obj,
				  message_obj * const msg)
{
	decoder_swo_obj *dec_swo = (decoder_swo_obj *) obj;
	decoder_swo_priv_data *pdata =
		(decoder_swo_priv_data *) dec_swo->pdata;
	int ret;

	ret = libswo_feed(pdata->swo_ctx,
			       msg->ptr(msg), msg->length(msg));
	if (ret) {
		ERROR("Error while getting retrieving data\n");
		ERROR("%s\n", libswo_strerror_name(ret));
		return -1;
	}

	ret = libswo_decode(pdata->swo_ctx, 0);
	if (ret) {
		ERROR("Error while decoding data\n");
		ERROR(" --> %s\n", libswo_strerror_name(ret));
		return -1;
	}
	pdata->tot_packet_decoded += pdata->cur_packet_decoded;
	DEBUG("decoded %d\n", pdata->cur_packet_decoded);
	DEBUG("total size decoded %ld\n",
			pdata->cur_packet_decoded* sizeof (union libswo_packet));
	DEBUG("Total number of decoded packet %d\n", pdata->tot_packet_decoded);
	return pdata->cur_packet_decoded * sizeof (union libswo_packet);
}

/**
 * @brief The function that feeds the decoder.
 * @param obj The generic processing object.
 * @param msg The message data coming form the above level.
 * @return The number of written bytes written, -1 on error.
 */
static size_t decoder_swo_data_out(processing_obj * const obj,
				   message_obj * const msg)
{
	decoder_swo_obj *dec_swo = (decoder_swo_obj *) obj;
	decoder_swo_priv_data *pdata = (decoder_swo_priv_data *) dec_swo->pdata;
	message_obj *msg_internal = &pdata->msg;
	unsigned int __packet_decoded ;

	msg->write(msg, (void *) packets_decoded,
	       pdata->cur_packet_decoded * sizeof (union libswo_packet));

	__packet_decoded = pdata->cur_packet_decoded;
	DEBUG("Number of data received %ld\n", __packet_decoded * sizeof (union libswo_packet));
	pdata->cur_packet_decoded = 0;

	return __packet_decoded * sizeof (union libswo_packet);
}

/**
 * @brief Checks if the instance is used. And return it if it available
 * @return The pointer on the private data, NULL if unavailable.
 */
static decoder_swo_priv_data *decoder_swo_get_free_instance(void)
{
	if (decoder_swo_pdata.is_used) {
		return NULL;
	}

	return &decoder_swo_pdata;
}

/**
 * @brief This function will set the private data as free to be used again.
 * @param pdata Pointer to the private data.
 * @return The number of written bytes written, -1 on error.
 */
static int decoder_swo_free_instance(decoder_swo_priv_data * const pdata)
{
	if (!pdata->is_used) {
		ERROR("Freeing private data\n");
		return -1;
	}

	pdata->is_used = false;
	return 0;
}

/**
 * @brief Funciton setting the filter callback necessary
 * 		depending on the type of session being run.
 * @param obj decoder object.
 * @return 0 upon success, -1 otherwise.
 */
static int decoder_swo_set_filter(decoder_swo_obj * const obj)
{
	const char *param_str;
	decoder_swo_priv_data *pdata = 
			(decoder_swo_priv_data *) obj->pdata;

	cfg_param param = {
				.section = CFG_SECTION_SESSION,
				.type = CONFIG_STR,
				.name = CFG_SECTION_SESSION_TYPE,
			  };

	param_str = CONFIG_HELPER_GET_STR(&param);
	if (!strcmp(CFG_SECTION_SESSION_TYPE_VAL_PE, param_str)) {
		pdata->filter_cb = filter_pc;
	} else if (!strcmp(CFG_SECTION_SESSION_TYPE_VAL_PM, param_str)) {
		pdata->filter_cb = filter_itm;
	} else {
		ERROR("Could not set correct filter\n");
		return -1;
	}

	return 0;
}

int decoder_swo_init(decoder_swo_obj * const obj)
{
	processing_obj *proc_obj = (processing_obj *) obj;
	decoder_swo_priv_data *pdata;
	message_obj *msg;

	if (processing_init(proc_obj)) {
		goto processing_init_failed;
	}

	if (!(pdata = decoder_swo_get_free_instance())) {
		goto get_free_instance_failed;
	}

	obj->pdata = pdata;

	if (decoder_swo_set_filter(obj)) {
		goto filter_setup_failed;
	}

	if (message_init(&pdata->msg)) {
		goto message_init_failed;
	}

	msg = &pdata->msg;

	proc_obj->data_in  = decoder_swo_data_in;
	proc_obj->data_out = decoder_swo_data_out;

	
	if (libswo_init(&pdata->swo_ctx, NULL,
			(size_t) (2U * msg->total_len(msg))) != LIBSWO_OK) {
		goto libswo_init_failed;
	}

	if (libswo_set_callback(pdata->swo_ctx, packet_cb, pdata)) {
		goto libswo_set_callback_failed;
	}

	libswo_log_set_level(pdata->swo_ctx, LIBSWO_LOG_LEVEL_NONE);

	return 0;
libswo_set_callback_failed:
	libswo_exit(pdata->swo_ctx);
libswo_init_failed:
filter_setup_failed:
message_init_failed:
get_free_instance_failed:
	processing_fini(proc_obj);
processing_init_failed:
	return -1;
}

int decoder_swo_fini(decoder_swo_obj * const obj)
{
	processing_obj *proc_obj = (processing_obj *) obj;
	decoder_swo_priv_data *pdata = (decoder_swo_priv_data *) obj->pdata;
	message_obj *msg = &pdata->msg;

	message_fini(msg);
	processing_fini(proc_obj);

	DEBUG("Decoder Swo uninitialized\n");
	return 0;
}

