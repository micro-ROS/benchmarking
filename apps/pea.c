#include <stdio.h>
#include <unistd.h>

#include <debug.h>

#include <config.h>
#include <config_ini.h>
#include <decoder_swo.h>
#include <form.h>
#include <file.h>
#include <perf_ex.h>
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

void app_print_usage(const char *name)
{
	fprintf(stdout, USAGE, name);
}


static void decoder_catch_signal(int signo) {
	pipeline_set_end_all();
}

#define DECODER_CONFIG_PATH_DEFAULT	BENCHMARKING_TOP_DIR \
					"/res/configs/execution_config.ini"

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

void decoder_init_perf_ex(perf_ex_obj *perf)
{
	memset(perf, 0, sizeof(*perf));
	if (perf_ex_init(perf)) {
		exit(EXIT_FAILURE);
	}

	perf->set_tc_gbl_config(perf);
	perf->set_elf_gbl_config(perf);
}

int decoder_init_file_perf(file_obj *file_f)
{
	cfg_param file_cfg = {
		.section = "output-files",
	};

	DEBUG("initializing file sink perf...\n");

	memset(file_f, 0, sizeof(*file_f));
	if (file_init(file_f)) {
		exit(EXIT_FAILURE);
	}

	file_cfg.name = "path-perf";
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

int decoder_init_file_json(file_obj *file_f)
{
	cfg_param file_cfg = {
		.section = "output-files",
	};

	DEBUG("initializing file sink json...\n");

	memset(file_f, 0, sizeof(*file_f));
	if (file_init(file_f)) {
		exit(EXIT_FAILURE);
	}

	file_cfg.name = "path-json";
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

static void decoder_fini_decoder_swo(decoder_swo_obj *swo)
{
	decoder_swo_fini(swo);
}

static void decoder_fini_form_cjson(form_obj *cjson)
{
	form_cjson_fini(cjson);
}

static void decoder_fini_file_json(file_obj *file_p)
{
	file_p->file_fini(file_p);
}

static void decoder_fini_file_perf(file_obj *file_p)
{
	file_p->file_fini(file_p);
}

static void decoder_fini_perf_ex(perf_ex_obj *perf)
{
	perf_ex_fini(perf);
}

int main(int argc, char **argv)
{
	config_ini_obj	cfgini;
	swd_ctrl_obj	swd_ctrl;
	uart_obj	uart_src;
	form_obj	cjson_proc;
	perf_ex_obj	perf_proc;
	decoder_swo_obj	decoder_proc;
	file_obj 	file_json;
	file_obj 	file_perf;
	processing_obj *proc;

	pipeline_obj	pipeline;

	int option_index = 0;

	if (signal(SIGINT, decoder_catch_signal) == SIG_ERR) {
		exit(EXIT_FAILURE);
	}

	decoder_init_config(&cfgini);
	decoder_init_swd_ctrl(&swd_ctrl);
	decoder_init_uart(&uart_src);
	decoder_init_decoder_swo(&decoder_proc);
	decoder_init_form_cjson(&cjson_proc);
	decoder_init_perf_ex(&perf_proc);
	decoder_init_file_json(&file_json);
	decoder_init_file_perf(&file_perf);

	proc = (processing_obj *) &uart_src;
	proc->register_element(proc, (processing_obj *) &decoder_proc);

	proc = (processing_obj *) &decoder_proc;
	proc->register_element(proc, (processing_obj *) &cjson_proc);
	proc->register_element(proc, (processing_obj *) &perf_proc);

	proc = (processing_obj *) &perf_proc;
	proc->register_element(proc, (processing_obj *) &file_perf);

	proc = (processing_obj *) &cjson_proc;
	proc->register_element(proc, (processing_obj *) &file_json);

	if (swd_ctrl.start(&swd_ctrl, argv[0])) {
		exit(EXIT_FAILURE);
	}

	if (pipeline_init(&pipeline)) {
		exit(EXIT_FAILURE);
	}

	DEBUG("Attaching elements\n");
	pipeline.attach_src(&pipeline, (processing_obj *) &uart_src);
	pipeline.attach_proc(&pipeline, (processing_obj *) &decoder_proc);
	pipeline.attach_proc(&pipeline, (processing_obj *) &cjson_proc);
	pipeline.attach_proc(&pipeline, (processing_obj *) &perf_proc);
	pipeline.attach_proc(&pipeline, (processing_obj *) &file_perf);
	pipeline.attach_proc(&pipeline, (processing_obj *) &file_json);

	while (!pipeline.is_stopped(&pipeline)) {
		if (pipeline.stream_data(&pipeline) < 0) {
			WARNING("Problem while streaming\n");
		}
	}

	decoder_fini_form_cjson(&cjson_proc);
	decoder_fini_decoder_swo(&decoder_proc);
	decoder_fini_uart(&uart_src);
	decoder_fini_swd_ctrl(&swd_ctrl);
	decoder_fini_perf_ex(&perf_proc);
	decoder_fini_file_perf(&file_perf);
	decoder_fini_file_json(&file_json);
	decoder_fini_config(&cfgini);

	DEBUG("Ending gracefully\n");
#if 0
	pipeline.fini(&pipeline);
	file_sink.fini(&file_sink);
	cjson_proc.fini(&cjson_proc);
	decoder_proc.fini(&decoder_proc);
	uart_src.fini(&uart_src);
	swd_ctrl.fini(&swd_ctrl);
	cfgini.fini(&cfgini);
#endif
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

