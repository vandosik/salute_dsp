#ifndef _UAPI_LINUX_TYPES_H
#define _UAPI_LINUX_TYPES_H

#if !defined( __QNX__ ) || (defined( __X86__ ) && !defined( __QNX4__ ))
#include <asm/types.h>
#else
#include <asm-generic/types.h>
#endif

#ifndef __ASSEMBLY__
//#ifndef	__KERNEL__
//#ifndef __EXPORTED_HEADERS__
//#warning "Attempt to use kernel headers from user space, see http://kernelnewbies.org/KernelHeaders"
//#endif /* __EXPORTED_HEADERS__ */
//#endif

//#include <linux/posix_types.h>
#include <asm-generic/bitsperlong.h> /* from posix_types.h */

/*
 * Below are truly Linux-specific types that should never collide with
 * any application/library that wants linux/types.h.
 */

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#ifdef __CHECK_ENDIAN__
#define __bitwise __bitwise__
#else
#define __bitwise
#endif

#endif /*  __ASSEMBLY__ */
#endif /* _UAPI_LINUX_TYPES_H */
