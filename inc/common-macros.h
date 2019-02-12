#ifndef __COMMON_MACROS_H__
#define __COMMON_MACROS_H__

#define ARRAY_SIZE(x) (sizeof(x)/(sizeof(x[0])))

/* stringify */
#define STR(a,b) a##b
#define STRINGIFY(a,b) STR(a,b)

#endif /*  __COMMON_MACROS_H__ */
