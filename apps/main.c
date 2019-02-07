#include <stdio.h>
#include <unistd.h>

#include <debug.h>

#include <processing.h>
#include <uart.h>
#include <pipeline.h>
#include <decoder_swo.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define USAGE    \
	"%s [OPTIONS]\n" \
	"\n" \
	"OPTIONS:\n" \
	"  -T [ 1 ]		type of SWD probe (not implemented)\n" \
	"  -t [ 1 | 2 ]		type of output interface:\n" \
	"					1 - UART\n" \
	"					2 - STLINKV2" \
		"(not implemented)\n" \
	"  -o [FILE|stdout]	path where the JSON formated result will be" \
		"stored\n"\
	"  -D /dev/ttyXXX 	dev path to the UART the SWO is\n" \
	"  -g			GUI interface for configuration\n"

#define APP_ARGS_OPTIONS		"T:t:o:D:g"

typedef struct application_config_st  application_config;

typedef enum {
	UART_SWO,
	/* In long term, this will be use to get data over the debug probe
		STLINKV2 has its own UART as well */
	STLINKV2_SWO,

} SWO_link;


typedef enum {
	/* So we are able to write any information to cpu over the SWD 
	 * protocol  not implemented */
	NO_SWD,
	STLINKV2_SWD,

} SWD_link;

typedef enum {
	NO_UI, /* User cannot interact */
	CLI_UI,
	NCURSE_UI,
} ui_disp;

static struct {
	SWO_link swo;
	SWD_link swd;	
	ui_disp	 ui;
} app_cfg = {
	.swo = UART_SWO,
	.swd = NO_SWD,
	.ui = NO_UI,
};

void app_print_usage(const char *name)
{
	fprintf(stdout, USAGE, name);
}

int main(int argc, char **argv)
{
	int option_index = 0;
	uart_obj	src;
	decoder_swo_obj	decoder;
	pipeline_obj	pipeline;

	memset(&src, 0, sizeof(src));
	memset(&decoder, 0, sizeof(decoder));
	memset(&pipeline, 0, sizeof(pipeline));

	if (uart_init(&src)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("UART Initialized \n");

	if (src.uart_set_dev(&src, "/dev/ttyUSB0")) {
		exit(EXIT_FAILURE);
	}
	DEBUG("UART path set\n");

	if (src.uart_init_dev(&src)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("UART is opened\n");

	if (src.uart_set_baudrate(&src, 115200U)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("UART baudrate set\n");

	if (decoder_swo_init(&decoder)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("SWO decoder initialized \n");

	if (pipeline_init(&pipeline)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("Pipeline initialized\n");

	DEBUG("Starting Attach\n");
	pipeline.attach_src(&pipeline, (processing_obj *) &src);
	pipeline.attach_sink(&pipeline, (processing_obj *) &decoder);

	while (1) {
		if (pipeline.stream_data(&pipeline) < 0) {
			WARNING("Problem while streaming\n");
		}
	}

#if 0
	file_obj 	sink_obj;
	memset(&file_obj, 0, sizeof(sink_obj));

	if (argc > 5 || argc < 2) i{
		app_print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	while ((option_index = getopt(argc, argv, APP_ARGS_OPTIONS)) != -1) {
		switch(option_index) {
			case 'D' :
				break;
			case 't' :
				break;
			case 'T' :
				break;
			case 'o' :
				break;
			case 'g' :
				app_cfg.ui = NCURSE_UI;
				break;
			default:
				app_print_usage(argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}
#endif
	

}

