#include <decoder_swo.h>
#include <common-macros.h>
#include <debug.h>

#include <libswo/libswo.h>

#include <stdbool.h>
#include <string.h>

typedef struct {
	struct libswo_context *swo_ctx;
	unsigned int packet_decoded;
	bool is_used;
	message_obj msg;
} decoder_swo_priv_data;


static decoder_swo_priv_data decoder_swo_pdata;

static union libswo_packet packets_decoded[1024 / sizeof(union libswo_packet)];
static unsigned int packet_decoded = 0;

static void default_packet_handler(const union libswo_packet *packet,
				   unsigned int c, const char *s)
{
	//DEBUG("Message %d, %s \n", c, s);
}


#define str(s)	#s
#define set_handler(pkt_type, handler) 		\
		[pkt_type] = { 			\
			.pkt_cb = handler, 	\
			.count = 0,		\
			.name = str(pkt_type)	\
		}
static struct {
	void (*pkt_cb) (const union libswo_packet *packet,
		       	unsigned int c, const char *s);
	unsigned int count;
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

static int packet_cb(struct libswo_context *ctx,
		const union libswo_packet *packet, void *user_data)
{
	(void)ctx;
	(void)user_data;
	unsigned int count;
	char *name;

#if 0
	if (!(packet_type_filter & (1 << packet->type)))
		return TRUE;
#endif
	if (((packet->type < LIBSWO_PACKET_TYPE_UNKNOWN) ||
		(packet->type > LIBSWO_PACKET_TYPE_HW)) &&
		((packet->type < LIBSWO_PACKET_TYPE_DWT_EVTCNT) ||
		(packet->type > LIBSWO_PACKET_TYPE_DWT_DATA_VALUE))) {

		WARNING("Unhandled packet type value %d\n", packet->type);
		return true;
	}

	count = default_handlers[packet->type].count;
	name = default_handlers[packet->type].name;
	default_handlers[packet->type].pkt_cb(packet, count, name);

	memcpy(&packets_decoded[packet_decoded], packet, sizeof(*packet));
	packet_decoded++;

	return true;
}

static size_t decoder_swo_data_in(processing_obj * const obj,
				  message_obj * const msg)
{
	decoder_swo_obj *dec_swo = (decoder_swo_obj *) obj;
	decoder_swo_priv_data *pdata =
		(decoder_swo_priv_data *) dec_swo->pdata;
	int ret;

	DEBUG("Processing data\n");
	ret = libswo_feed(pdata->swo_ctx,
			       msg->ptr(msg), msg->length(msg));
	if (ret) {
		ERROR("Error while getting retrieving data\n");
		ERROR("%s", libswo_strerror_name(ret));
		return -1;
	}

	ret = libswo_decode(pdata->swo_ctx, 0);
	if (ret) {
		ERROR("Error while decoding data\n");
		ERROR("%s", libswo_strerror_name(ret));
		return -1;
	}

	return 0;
}

static size_t decoder_swo_data_out(processing_obj * const obj,
				   message_obj * const msg)
{
	decoder_swo_obj *dec_swo = (decoder_swo_obj *) obj;
	decoder_swo_priv_data *pdata = (decoder_swo_priv_data *) dec_swo->pdata;
	message_obj *msg_internal = &pdata->msg;


	memset(msg->ptr(msg), 0, msg->total_len(msg));
	msg->write(msg, (void *) packets_decoded,
	       packet_decoded * sizeof (union libswo_packet));
#if 0
	for (unsigned int i=0; i<len; i++) {
		DEBUG("data %d, %02x\n", i, data[i]);
	}
#endif
	packet_decoded = 0;
	return 0;
}

static decoder_swo_priv_data *decoder_swo_get_free_instance(void)
{
	if (decoder_swo_pdata.is_used) {
		return NULL;
	}

	return &decoder_swo_pdata;
}

static int decoder_swo_free_instance(decoder_swo_priv_data *pdata)
{
	if (!pdata->is_used) {
		ERROR("Freeing private data\n");
		return -1;
	}

	pdata->is_used = false;
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

	if (libswo_set_callback(pdata->swo_ctx, packet_cb, NULL)) {
		goto libswo_set_callback_failed;
	}

	libswo_log_set_level(pdata->swo_ctx, LIBSWO_LOG_LEVEL_NONE);

	return 0;
libswo_set_callback_failed:
	libswo_exit(pdata->swo_ctx);
libswo_init_failed:
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
