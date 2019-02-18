#include <stdio.h>
#include <unistd.h>

#include <debug.h>

#include <config.h>
#include <config_ini.h>
#include <decoder_swo.h>
#include <form.h>
#include <file.h>
#include <pipeline.h>
#include <processing.h>
#include <uart.h>

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

#define DECODER_CONFIG_PATH_DEFAULT	"/tmp/default_decoder.ini"
void decoder_init_config(config_ini_obj *cfg)
{
	DEBUG("initializing config...\n");

	memset(cfg, 0, sizeof(*cfg));
	if (config_ini_init(cfg)) {
		exit(EXIT_FAILURE);
	}

	if (cfg->open_cfg(cfg, DECODER_CONFIG_PATH_DEFAULT)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("config initialized\n");
}

void decoder_init_uart(uart_obj *uart)
{
	cfg_param uart_cfg = {
		.section = "uart-swo",
	};

	DEBUG("initializing uart...\n");

	memset(uart, 0, sizeof(*uart));
	if (uart_init(uart)) {
		exit(EXIT_FAILURE);
	}

	uart_cfg.name = "device";
	uart_cfg.type = CONFIG_STR;
	DEBUG("\t-device used: %s\n", CONFIG_HELPER_GET_STR(&uart_cfg));
	if (uart->uart_set_dev(uart,
				  CONFIG_HELPER_GET_STR(&uart_cfg))) {
		exit(EXIT_FAILURE);
	}

	if (uart->uart_init_dev(uart)) {
		exit(EXIT_FAILURE);
	}

	uart_cfg.name = "baudrate";
	uart_cfg.type = CONFIG_UNSIGNED_INT;
	DEBUG("\t-baudrate used: %u\n", CONFIG_HELPER_GET_U32(&uart_cfg));
	if (uart->uart_set_baudrate(uart,
				       CONFIG_HELPER_GET_U32(&uart_cfg))) {
		exit(EXIT_FAILURE);
	}

	DEBUG("uart initialized\n");
}

void decoder_init_form_cjson(form_obj *form)
{
	DEBUG("initializing cjson form...\n");

	if (form_cjson_init(form)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("cjson form initialized...\n");
}

void decoder_init_decoder_swo(decoder_swo_obj *dec)
{
	DEBUG("initializing decoder swo...\n");

	memset(dec, 0, sizeof(*dec));
	if (decoder_swo_init(dec)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("decoder swo initialized...\n");
}

int decoder_init_file_sink(file_obj *file_f)
{
	cfg_param file_cfg = {
		.section = "output-file",
	};

	DEBUG("initializing file sink...\n");

	memset(file_f, 0, sizeof(*file_f));
	if (file_init(file_f)) {
		exit(EXIT_FAILURE);
	}

	file_cfg.name = "path";
	file_cfg.type = CONFIG_STR;
	if (file_f->file_set_path(file_f,
				CONFIG_HELPER_GET_STR(&file_cfg),
			        FILE_WRONLY)) {
		exit(EXIT_FAILURE);
	}

	if (file_f->file_init(file_f)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("file sink initialized.\n");
	return 0;
}

int main(int argc, char **argv)

{
	config_ini_obj	cfgini;
	uart_obj	uart_src;
	form_obj	cjson_proc;
	decoder_swo_obj	decoder_proc;
	file_obj 	file_sink;

	pipeline_obj	pipeline;

	int option_index = 0;

	decoder_init_config(&cfgini);

	decoder_init_uart(&uart_src);
	decoder_init_decoder_swo(&decoder_proc);
	decoder_init_form_cjson(&cjson_proc);
	decoder_init_file_sink(&file_sink);

	if (pipeline_init(&pipeline)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("Attaching elements\n");
	pipeline.attach_src(&pipeline, (processing_obj *) &uart_src);
	pipeline.attach_proc(&pipeline, (processing_obj *) &decoder_proc, NULL);
	pipeline.attach_proc(&pipeline, (processing_obj *) &cjson_proc, NULL);
	pipeline.attach_sink(&pipeline, (processing_obj *) &file_sink);

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

	return -1;
}

