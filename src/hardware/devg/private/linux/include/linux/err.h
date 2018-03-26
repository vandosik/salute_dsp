#ifndef _LINUX_ERR_H
#define _LINUX_ERR_H

#include <linux/compiler.h>

#ifndef __QNX__
#include <asm/errno.h>
#else
#ifdef __QNXNTO__
#include <stdbool.h>
#include <errno.h>
#else
#include <qnx4_helpers.h>
#endif
#endif

/*
 * Kernel pointers have redundant information, so we can use a
 * scheme where we can return either an error code or a normal
 * pointer with the same return value.
 *
 * This should be a per-architecture thing, to allow different
 * error and pointer decisions.
 */
#ifndef __QNX__
#define MAX_ERRNO       4095
#else
#ifdef __QNXNTO__
#define MAX_ERRNO       EENDIAN                     /* Bug? search maximal errno code in errno.h */
#else
#define MAX_ERRNO       EDSTFAULT                   /* Bug? search maximal errno code in errno.h */
#endif
#endif

#ifndef __ASSEMBLY__

#define IS_ERR_VALUE(x) unlikely((unsigned long)(void *)(x) >= (unsigned long)-MAX_ERRNO)

static inline void * __must_check ERR_PTR(long error)
{
        return (void *) error;
}

static inline long __must_check PTR_ERR(__force const void *ptr)
{
        return (long) ptr;
}

static inline bool __must_check IS_ERR(__force const void *ptr)
{
        return IS_ERR_VALUE((unsigned long)ptr);
}

/**
 * ERR_CAST - Explicitly cast an error-valued pointer to another pointer type
 * @ptr: The pointer to cast.
 *
 * Explicitly cast an error-valued pointer to another pointer type in such a
 * way as to make it clear that's what's going on.
 */
static inline void * __must_check ERR_CAST(__force const void *ptr)
{
	/* cast away the const */
	return (void *) ptr;
}

#endif /* __ASSEMBLY__ */

#endif /* _LINUX_ERR_H */
