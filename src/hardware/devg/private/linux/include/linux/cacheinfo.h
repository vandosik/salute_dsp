#ifndef _LINUX_CACHEINFO_H
#define _LINUX_CACHEINFO_H

#include <linux/bitops.h>
//#include <linux/cpumask.h>
//#include <linux/smp.h>

enum cache_type {
	CACHE_TYPE_NOCACHE = 0,
	CACHE_TYPE_INST = BIT(0),
	CACHE_TYPE_DATA = BIT(1),
	CACHE_TYPE_SEPARATE = CACHE_TYPE_INST | CACHE_TYPE_DATA,
	CACHE_TYPE_UNIFIED = BIT(2),
};

#endif /* _LINUX_CACHEINFO_H */
