#ifndef __UART_H__
#define __UART_H__

#include <message.h>
#include <processing.h>

typedef struct uart_obj_st uart_obj;

typedef int (*uart_set_baudrate_cb)(uart_obj * const obj, 
				    unsigned int baudrate);
typedef int (*uart_set_dev_cb)(uart_obj * const obj, 
				    const char * const dev);
typedef int (*uart_init_dev_cb)(uart_obj * const obj);

typedef int (*uart_fini_dev_cb)(uart_obj * const obj);

struct uart_obj_st {
	processing_obj		proc_obj;

	/** UART function ops. Set up the baudrate */
	uart_set_baudrate_cb 	uart_set_baudrate;	

	uart_set_dev_cb 	uart_set_dev;

	uart_init_dev_cb	uart_init_dev;
	uart_fini_dev_cb	uart_fini_dev;
	/** private data */
	void 	   *pdata;		
};

int uart_init(uart_obj * const obj);
int uart_clean(uart_obj * const obj);

#endif /* __UART_H__ */
