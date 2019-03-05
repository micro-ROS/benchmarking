/**
 *
 *
 * Author Alexandre MALKI (amalki@piap.pl)
 * This file is the uart communication intented to be used
 * in conjunction with the libswo to retrieve debug information
 * for later use
 */


#include <stdio.h>

/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* read */
#include <unistd.h>

/* perror */
#include <errno.h>

/*fcntl fnctl */
#include <fcntl.h>

/* time for timeout (could use poll or select) */
#include <sys/time.h>

#include <uart.h>

/* Common macros */
#include <common-macros.h>
/* Configuration */
#include <config.h>
/* debug messages */
#include <debug.h>

/* tty commands */
#include <termios.h>

/* boolean */
#include <stdbool.h>

/* memset */
#include <string.h>

#include <inttypes.h>

#include <poll.h>

#define SYNC_DWT_TRACE 		((char) 0x00)
#define SYNC_DWT_END 		((char) 0x80)

#define SYNC_NODWT_TRACE 	((char) 0xff)
#define SYNC_NODWT_END 		((char) 0x7f)

#define TIMEOUT_S		560

#define SYNC_DWT_PACKET_SIZE	6
#define SYNC_PACKET_SIZE	5

#define ADD_BAUDRATE(a,b) { .term_val = a, .user_val = b }
/**
 * This structure holds information regarding the baudrate translation
 * between termios and user 
 */
struct {
	/** This is the take from the GLIBC library termios */
	unsigned int term_val;
	/** User value taken from configuration file */
	unsigned int user_val;
} uart_baudrate_tbl[] = {
	ADD_BAUDRATE(B115200, 115200),
};

/** TODO create a common interface file and uart */
/* Number of Max UART used */

typedef struct  {
	/** PATH to the char device */
	char dev_path[UART_COUNT_MAX];
	/** termios terminal configuraion settings */
	struct termios tty;
	/** UART file descriptor */
	struct pollfd pfd;
	/** If used (init was called) */
	bool is_used;
	/** If open (open was called) */
	bool is_open;
	/** Manage multi thread concurency */
} uart_private_data;

static uart_private_data uarts_instances[UART_DEV_COUNT_MAX] = {0};

static int uart_get_instance(uart_obj * const uart)
{
	unsigned int i;
	int rc = -1;

	/* TODO : Might be some concurence here */
	for (i=0; i<ARRAY_SIZE(uarts_instances); i++) {
		if (!uarts_instances[i].is_used) {
			uart->pdata = (void *) &uarts_instances[i];
			uarts_instances[i].is_used = true;
			memset(&uarts_instances[i].tty, 0, sizeof(struct termios));
			rc = 0;
			break;
		}
	}

	/* TODO : Might be some concurence here */
	return rc;
}

static void uart_free_instance(uart_obj * const uart)
{
	uart_private_data *uartpd = (uart_private_data *) uart->pdata;
	memset(uartpd, 0, sizeof(*uartpd));
}

/* As for now, it only set the baudrate to be enhanced */
static int uart_set_baudrate(uart_obj * const uart, unsigned int baudrate)
{
	uart_private_data * const uartpd = uart->pdata;
	unsigned int default_br = uart_baudrate_tbl[0].term_val;
	DEBUG("Setting baudrate %d \n", baudrate);
        if (tcgetattr (uartpd->pfd.fd, &uartpd->tty) != 0) {
                ERROR("error %d from tcgetattr", errno);
                return -1;
        }

	for (unsigned int i = 0; i < ARRAY_SIZE(uart_baudrate_tbl); i++) {
		if (baudrate == uart_baudrate_tbl[i].user_val) {
			default_br = uart_baudrate_tbl[i].term_val;
			break;
		}	
	}

	cfmakeraw(&uartpd->tty);
	/** We are setting the baudrate to 8N1 */
	uartpd->tty.c_cflag= default_br|CS8|CREAD|CLOCAL; 
	uartpd->tty.c_cflag &= ~PARENB;
	uartpd->tty.c_cflag &= ~CSTOPB;
	uartpd->tty.c_cflag &= ~CSIZE;
	uartpd->tty.c_cflag |= CS8;

        if (tcsetattr (uartpd->pfd.fd, TCSANOW, &uartpd->tty) != 0)
        {
                ERROR("error %d from tcsetattr", errno);
                return -1;
        }

	return 0;
}

static int uart_set_dev(uart_obj * const uart, const char * const dev)
{
	uart_private_data * const uartpd = (uart_private_data *) uart->pdata;
	strncpy(uartpd->dev_path, dev, sizeof(uartpd->dev_path) - 1);

	return 0;
}

static int uart_open(uart_obj * const uart)
{
	uart_private_data * const uartpd =
	       			(uart_private_data * const) uart->pdata;
	int flags;

	if (uartpd->is_open) {
		ERROR("Device %s: already open\n",
		      uartpd->dev_path);
		return -1;
	}

	uartpd->pfd.fd = open(uartpd->dev_path, O_RDONLY);
	if (uartpd->pfd.fd < 0) {
		ERROR("Device %s error while opening: %s\n",
		      uartpd->dev_path, strerror(errno));
		return -1;
	}

	uartpd->pfd.events = POLLIN|POLLPRI;
	uartpd->is_open = 1;

	return 0;
}


static int uart_close(uart_obj * const uart)
{
	uart_private_data * const uartpd =
	       			(uart_private_data * const) uart->pdata;

	if (!uartpd->is_open) {
		ERROR("Device %s, already open\n",
		      uartpd->dev_path);
		return -1;
	}
	close(uartpd->pfd.fd);

	return 0;
}

static int uart_debug_table(const char * const buffer, size_t len,
			     unsigned int bpc)
{
	DEBUG("Table: data received %p size=%"PRId64"\n", buffer, len);
	for (unsigned int i=0;i<len;i++) {
		if (!(i%bpc))
			printf("\n");

		printf("0x%02X ", (unsigned char) buffer[i]);
	}

	putchar('\n');
	DEBUG("Table: end of buffer\n");
}

/**
 * \brief this function receives information from the UART device
 * \param obj: UART object reference.
 * \param msg: message object where data is going to be receive.
 */
static size_t uart_receive(processing_obj * const obj, message_obj * const msg)
{
	uart_obj * const uart = (uart_obj * const) obj;
	uart_private_data * const pdata = (uart_private_data *) uart->pdata;
	char ptr[UART_INTERNAL_BUF_LEN_MAX];
	size_t readd, n = 0;
	unsigned int retries = UART_MAX_POLL_RETRIES;
	int rc;
	while (sizeof(ptr) - n) {
		rc = poll(&pdata->pfd, 1, UART_TIMEOUT_MS);
		if (-1 == rc) {
			ERROR("Error while polling %s\n", strerror(errno));
			return -1;
		}

		if (!rc) {
			if (retries--)
				continue;

			WARNING("UART timed out no data !\n");
			return -1;
		}

		if (!(pdata->pfd.revents & POLLIN)) {
			WARNING("Invalid event!\n");
			return -1;
		}

		readd = read(pdata->pfd.fd, &ptr[n], sizeof(ptr) - n);
		if (readd < 0) {
			return -1;
		}
		n+=readd;
	}

	msg->write(msg, ptr, n);

	return n;
}

int uart_init(uart_obj * const uart)
{
	processing_obj *proc_obj = (processing_obj *) uart;

	memset(uart, 0, sizeof(*uart));
	if (uart_get_instance(uart)) {
		goto free_instance_failed;
	}

	uart->uart_set_baudrate = uart_set_baudrate;
	uart->uart_set_dev = uart_set_dev;
	uart->uart_init_dev = uart_open;
	uart->uart_fini_dev = uart_close;
	if (processing_init(proc_obj)) {
		goto processing_init_failed;
	}

	proc_obj->data_out = uart_receive;
	return 0;
processing_init_failed:
free_instance_failed:
	uart_free_instance(uart);
	return -1;
}

void uart_fini(uart_obj * const uart)
{
	processing_obj *proc_obj = (processing_obj *) uart;
	uart_private_data *uartpd = (uart_private_data *)uart->pdata;

	if (!uartpd->is_used) {
		WARNING("UART not initialized\n");
		return;
	}

	if (processing_fini(proc_obj)) {
		return;
	}

	uart_free_instance(uart);
}

