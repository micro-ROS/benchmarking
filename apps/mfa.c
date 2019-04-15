#include <stdio.h>
#include <unistd.h>

#include <debug.h>

#include <config.h>
#include <config_ini.h>
#include <decoder_swo.h>
#include <form.h>
#include <file.h>
#include <itm_to_str.h>
#include <itm2mem_info.h>
#include <pipeline.h>
#include <processing.h>
#include <uart.h>
#include <swd_ctrl.h>

#include <stdbool.h>
#include <signal.h>
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

static bool is_running;

static void app_print_usage(const char *name)
{
	fprintf(stdout, USAGE, name);
}


static void decoder_catch_signal(int signo) {
	pipeline_set_end_all();
}

#define DECODER_CONFIG_PATH_DEFAULT	BENCHMARKING_TOP_DIR \
					"/res/configs/memory_heap_config.ini"
static void decoder_init_config(config_ini_obj *cfg)
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

static void decoder_init_uart(uart_obj *uart)
{
	unsigned int uart_baudrate;
	const char *uart_dev;
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
	uart_dev = CONFIG_HELPER_GET_STR(&uart_cfg);
	if (!uart_cfg.found) {
		exit(EXIT_FAILURE);
	}

	DEBUG("\t-device used: %s\n", uart_dev);
	if (uart->uart_set_dev(uart, uart_dev)) {
		exit(EXIT_FAILURE);
	}

	if (uart->uart_init_dev(uart)) {
		exit(EXIT_FAILURE);
	}

	uart_cfg.name = "baudrate";
	uart_cfg.type = CONFIG_UNSIGNED_INT;
	uart_baudrate = CONFIG_HELPER_GET_U32(&uart_cfg);
	DEBUG("\t-baudrate used: %u\n", uart_baudrate);

	if (!uart_cfg.found) {
		exit(EXIT_FAILURE);
	}
	if (uart->uart_set_baudrate(uart, uart_baudrate)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("uart initialized\n");
}

static void decoder_init_its(itm_to_str_obj *its_obj)
{
	DEBUG("initializing itm_to_str...\n");
	if (itm_to_str_init(its_obj)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("itm_to_str object initialized...\n");
}

static void decoder_init_itm2mi(itm2mem_info_obj *it2mi_obj)
{
	DEBUG("initializing itm_to_str...\n");
	if (itm2mem_info_init(it2mi_obj)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("itm_to_str object initialized...\n");
}

static void decoder_init_decoder_swo(decoder_swo_obj *dec)
{
	DEBUG("initializing decoder swo...\n");

	memset(dec, 0, sizeof(*dec));
	if (decoder_swo_init(dec)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("decoder swo initialized...\n");
}

int decoder_init_file_raw_data(file_obj *file_f)
{
	cfg_param file_cfg = {
		.section = "output-files",
	};

	DEBUG("initializing file sink raw data...\n");

	memset(file_f, 0, sizeof(*file_f));
	if (file_init(file_f)) {
		exit(EXIT_FAILURE);
	}

	if (file_f->file_set_path(file_f,
				"test_backtrace",
			        FILE_WRONLY)) {
		exit(EXIT_FAILURE);
	}

	if (file_f->file_init(file_f)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("file sink initialized.\n");
	return 0;
}
void decoder_init_swd_ctrl(swd_ctrl_obj *swd)
{
	DEBUG("swd ctrl initializing.\n");
	if (swd_ctrl_init(swd)) {
		exit(EXIT_FAILURE);
	}

	if (swd->set_cfg_if_from_gbl_cfg(swd)) {
		exit(EXIT_FAILURE);
	}

	if (swd->set_cfg_cpu_from_gbl_cfg(swd)) {
		exit(EXIT_FAILURE);
	}
	DEBUG("swd ctrl initialized.\n");
}

static void decoder_fini_config(config_ini_obj *cfg)
{
	config_ini_fini(cfg);
}

static void decoder_fini_swd_ctrl(swd_ctrl_obj *swd_ctrl)
{
	swd_ctrl->stop(swd_ctrl);
	swd_ctrl_fini(swd_ctrl);
}


static void decoder_fini_uart(uart_obj *uart)
{
	uart->uart_fini_dev(uart);
}

static void decoder_fini_its(itm_to_str_obj *its_obj)
{
	itm_to_str_fini(its_obj);
}

static void decoder_fini_decoder_swo(decoder_swo_obj *swo)
{
	decoder_swo_fini(swo);
}

static void decoder_fini_file_raw_data(file_obj *file_p)
{
	file_p->file_fini(file_p);
}

int main(int argc, char **argv)
{
	config_ini_obj	cfgini;
	swd_ctrl_obj	swd_ctrl;
	uart_obj	uart_src;
	decoder_swo_obj	decoder_proc;
	itm_to_str_obj	its_proc;
	itm2mem_info_obj itm2mi_proc;
	file_obj 	file_raw_data;
	processing_obj *proc;

	pipeline_obj	pipeline;

	int option_index = 0;

	if (signal(SIGINT, decoder_catch_signal) == SIG_ERR) {
		exit(EXIT_FAILURE);
	}

	decoder_init_config(&cfgini);
	decoder_init_swd_ctrl(&swd_ctrl);
	decoder_init_uart(&uart_src);
	decoder_init_its(&its_proc);
	decoder_init_itm2mi(&itm2mi_proc);
	decoder_init_decoder_swo(&decoder_proc);
	decoder_init_file_raw_data(&file_raw_data);

	proc = (processing_obj *) &uart_src;
	proc->register_element(proc, (processing_obj *) &decoder_proc);

	proc = (processing_obj *) &decoder_proc;
	proc->register_element(proc, (processing_obj *) &its_proc);
	proc->register_element(proc, (processing_obj *) &itm2mi_proc);

	proc = (processing_obj *) &its_proc;
	proc->register_element(proc, (processing_obj *) &file_raw_data);

	if (swd_ctrl.start(&swd_ctrl, argv[0])) {
		exit(EXIT_FAILURE);
	}

	if (pipeline_init(&pipeline)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("Attaching elements\n");
	pipeline.attach_src(&pipeline, (processing_obj *) &uart_src);
	pipeline.attach_proc(&pipeline, (processing_obj *) &decoder_proc);
	pipeline.attach_proc(&pipeline, (processing_obj *) &itm2mi_proc);
	pipeline.attach_proc(&pipeline, (processing_obj *) &its_proc);
	pipeline.attach_proc(&pipeline, (processing_obj *) &file_raw_data);

	while (!pipeline.is_stopped(&pipeline)) {
		if (pipeline.stream_data(&pipeline) < 0) {
			WARNING("Problem while streaming\n");
		}
	}

	decoder_fini_decoder_swo(&decoder_proc);
	decoder_fini_uart(&uart_src);
	decoder_fini_swd_ctrl(&swd_ctrl);
	decoder_fini_its(&its_proc);
	decoder_fini_file_raw_data(&file_raw_data);

	DEBUG("Ending gracefully\n");
#if 0
	pipeline.fini(&pipeline);
	file_sink.fini(&file_sink);
	decoder_proc.fini(&decoder_proc);
	uart_src.fini(&uart_src);
	swd_ctrl.fini(&swd_ctrl);
	cfgini.fini(&cfgini);

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

