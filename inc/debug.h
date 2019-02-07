#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>


#define ERROR(fmt, args...) do { \
       				fprintf(stderr,"erro|%s:%d->%s(): " fmt, \
 				        __FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0)

#define WARNING(fmt, args...) do { \
       				fprintf(stderr,"warn|%s:%d->%s(): " fmt, \
 				        __FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0)


#define DEBUG(fmt, args...) do { \
       				fprintf(stdout,"debg|%s:%d->%s(): " fmt, \
 				        __FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0)
#endif /* __DEBUG_H__ */
