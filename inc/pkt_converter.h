/**
 * This file will provide a generic way to translate 
 * packets received from any decoder to any form output
 */
#ifndef __PKT_CONVERTER_H__
#define __PKT_CONVERTER_H__

#include <message.h>
#include <libswo/libswo.h>
#include <cJSON.h>

typedef enum {
	PKT_CONVERTER_CJSON = 0,
	PKT_CONVERTER_MYSQL = 1,
	PKT_CONVERTER_MAX   = 2,
} pkt_format_output;

typedef struct {
	pkt_format_output fmt;
	union {
		cJSON *root;
	};
} pkt_to_form;

size_t pkt_convert(pkt_to_form * const ptf, message_obj * const obj);

#endif /* __PKT_CONVERTER_H__*/
