#ifndef _ASM_GENERIC_BITOPS_FFS_H_
#define _ASM_GENERIC_BITOPS_FFS_H_

#ifdef __QNXNTO__
#include <strings.h>
#else
#ifdef __QNX__
#include <qnx4_helpers.h>
#endif

/**
 * ffs - find first bit set
 * @x: the word to search
 *
 * This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static inline int ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}
#endif  /* __QNXNTO__ */

#endif /* _ASM_GENERIC_BITOPS_FFS_H_ */
