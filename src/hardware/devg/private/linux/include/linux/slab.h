/*
 * Written by Mark Hemment, 1996 (markhe@nextd.demon.co.uk).
 *
 * (C) SGI 2006, Christoph Lameter
 * 	Cleaned up and restructured to ease the addition of alternative
 * 	implementations of SLAB allocators.
 * (C) Linux Foundation 2008-2013
 *      Unified interface for all slab allocators
 */

#ifndef _LINUX_SLAB_H
#define	_LINUX_SLAB_H

#ifdef __QNX__
#include <stdlib.h>
#endif
#include <linux/gfp.h>
#include <linux/types.h>
//#include <linux/workqueue.h>


/**
 * kmalloc_array - allocate memory for an array.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
#ifndef __QNX__
	if (__builtin_constant_p(n) && __builtin_constant_p(size))
		return kmalloc(n * size, flags);
	return __kmalloc(n * size, flags);
#else
	return malloc(n * size);
#endif
}

#endif	/* _LINUX_SLAB_H */
