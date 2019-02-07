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


#define SYNC_DWT_TRACE 		((char) 0x00)
#define SYNC_DWT_END 		((char) 0x80)

#define SYNC_NODWT_TRACE 	((char) 0xff)
#define SYNC_NODWT_END 		((char) 0x7f)

#define TIMEOUT_S		560	

#define SYNC_DWT_PACKET_SIZE	6
#define SYNC_PACKET_SIZE	5

/* Number of Max UART used */
#define UART_MAX_INSTANCE 1

typedef struct  {
	/** PATH to the char device */
	char dev_path[STRING_MAX_LENGTH];
	/** termios terminal configuraion settings */
	struct termios tty;	
	/** UART file descriptor */
	int fd;
	/** If used (init was called) */
	bool is_used;
	/** If used (open was called) */
	bool is_open;
	/** Manage multi thread concurency */
} uart_private_data;

static uart_private_data uarts_instances[UART_MAX_INSTANCE] = {0};

static int uart_get_instance(uart_obj * const uart)
{
	unsigned int i;
	int rc = -1;

	/* TODO : Might be some concurence here */
	for (i=0; i<ARRAY_SIZE(uarts_instances); i++) {
		if (!uarts_instances[i].is_used) {
			uart->pdata = (void *) &uarts_instances[i];
			uarts_instances[i].is_used = true;
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

        if (tcgetattr (uartpd->fd, &uartpd->tty) != 0) {
                ERROR("error %d from tcgetattr", errno);
                return -1;
        }
 
	cfsetspeed (&uartpd->tty, baudrate);
        if (tcsetattr (uartpd->fd, TCSANOW, &uartpd->tty) != 0)
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

	if (uartpd->is_open) {
		ERROR("Device %s: already open\n",
		      uartpd->dev_path);
		return -1;
	}

	uartpd->fd = open(uartpd->dev_path, O_RDONLY);
	if (uartpd->fd < 0) {
		ERROR("Device %s error while opening: \n",
		      uartpd->dev_path);
		return -1;
	}
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
	close(uartpd->fd);

	return 0;
}

/**
 * @brief this function allows
 */
int uart_receive(processing_obj *obj, message_obj *msg)
{
	int fd, fd_uart;
	int n, readd, written = 0;
	int rc = -1;

	DEBUG("Receiving\n");
#if 0

	while ((readd = read(fd_uart, buf, sizeof(buf))) > - 1) {
		while (written != readd) {
			n = write(fd, &buf[written], readd - written);
			if (n<0) {
				printf("Error while writing to file");
				goto fini;
			}
			written += n;
		}

		written = 0;
	}

	rc = 0;
	printf("Finished getting data\n");

fini:
	close(fd_uart);
uart_fail:
	close(fd);
file_fail:
argc_fail:
	printf("Finished application leaving\n");
#endif
	return rc;
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

