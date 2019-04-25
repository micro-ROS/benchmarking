#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <config.h>

#include <stdio.h>

#define DBG_LVL_NONE		(1 << 0)
#define DBG_LVL_ERROR		(1 << 1)
#define DBG_LVL_WARNING		(1 << 2)
#define DBG_LVL_MESSAGE		(1 << 3)
#define DBG_LVL_DEBUG		(1 << 4)
#define DBG_LVL_ALL		(DBG_LVL_ERROR|DBG_LVL_WARNING| \
				 DBG_LVL_MESG|DBG_LVL_DEBUG)

/* Default debug level all messages are displayed */
#define DEFAULT_DBG_LVL 	DBG_LVL_ALL

#ifndef DEBUG_LVL
#define DEBUG_LVL		DEFAULT_DBG_LVL
#endif /* DEBUG_LVL */

#if (DEBUG_LVL & DBG_LVL_ERROR)
#define ERROR(fmt, args...)  do { \
				fprintf(stderr,"erro|%s:%d->%s(): " fmt, \
					__FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0) 
#else /* if (DEBUG_LVL & DBG_LVL_ERROR) */
#define ERROR(fmt, args...)
#endif /* if (DEBUG_LVL & DBG_LVL_ERROR) */

#if (DEBUG_LVL & DBG_LVL_WARNING)
#define WARNING(fmt, args...) do { \
				fprintf(stderr,"warn|%s:%d->%s(): " fmt, \
					__FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0)
#else /* if (DEBUG_LVL & DBG_LVL_WARNING) */
#define WARNING(fmt, args...)
#endif /* if (DEBUG_LVL & DBG_LVL_WARNING) */

#if (DEBUG_LVL & DBG_LVL_MESG)
#define MESG(fmt, args...) do { \
				fprintf(stdout,"mesg|%s:%d->%s(): " fmt, \
					__FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0)
#else /* if (DEBUG_LVL & DBG_LVL_MESG) */
#define MESG(fmt, args...)
#endif /* if (DEBUG_LVL & DBG_LVL_MESG) */

#if (DEBUG_LVL & DBG_LVL_DEBUG)
#define DEBUG(fmt, args...) do { \
				fprintf(stdout,"debg|%s:%d->%s(): " fmt, \
					__FILE__, __LINE__, __func__, \
					## args ); \
			    } while(0)
#else /* if (DEBUG_LVL & DBG_LVL_DEBUG) */
#define DEBUG(fmt, args...)
#endif /* if (DEBUG_LVL & DBG_LVL_DEBUG) */


#endif /* __DEBUG_H__ */
