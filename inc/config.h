#ifndef __CONFIG_H__
#define __CONFIG_H__


#ifdef  MESSAGE_DYNAMIC

#else
/** This is */
#define MESSAGE_NSTANCES_CNT_MAX	32	
/** This is */
#define MESSAGE_BUFFER_SZ_MAX		1024 

#endif

#define STRING_MAX_LENGTH		255

/** Used in the uart receive callback */
#define UART_INTERNAL_BUF_LEN_MAX	1024

struct config_mgmr_st {
	unsigned int test ;
};


#endif /* __CONFIG_H__ */
