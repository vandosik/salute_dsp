#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

//#define __EXPORTED_HEADERS__
#ifdef __QNXNTO__
#include <stdbool.h>
#endif

#include <uapi/linux/types.h>

#ifndef __ASSEMBLY__

#define DECLARE_BITMAP(name,bits) \
        unsigned long name[BITS_TO_LONGS(bits)]
	
typedef unsigned __bitwise__ gfp_t;

struct list_head {
	struct list_head *next, *prev;
};

#endif /*  __ASSEMBLY__ */
#endif /* _LINUX_TYPES_H */
