#ifndef _LINUX_MATH64_H
#define _LINUX_MATH64_H

#ifdef __QNX__
#include <stdint.h>
#include <asm-generic/div64.h>
#endif

/**
 * div_u64_rem - unsigned 64bit divide with 32bit divisor with remainder
 *
 * This is commonly provided by 32bit archs to provide an optimized 64bit
 * divide.
 */
static inline uint64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
        *remainder = dividend % divisor;
        return dividend / divisor;
}

/**
 * div64_u64_rem - unsigned 64bit divide with 64bit divisor and remainder
 */
static inline u64 div64_u64_rem(u64 dividend, u64 divisor, u64 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/**
 * div64_u64 - unsigned 64bit divide with 64bit divisor
 */
static inline uint64_t div64_u64(uint64_t dividend, uint64_t divisor)
{
        return dividend / divisor;
}

/**
 * div64_s64 - signed 64bit divide with 64bit divisor
 */
static inline s64 div64_s64(s64 dividend, s64 divisor)
{
	return dividend / divisor;
}

/**
 * div_u64 - unsigned 64bit divide with 32bit divisor
 *
 * This is the most common 64bit divide and should be used if possible,
 * as many 32bit archs can optimize this variant better than a full 64bit
 * divide.
 */
#ifndef div_u64
static inline uint64_t div_u64(uint64_t dividend, uint32_t divisor)
{
        uint32_t remainder;
        return div_u64_rem(dividend, divisor, &remainder);
}
#endif

#endif /* _LINUX_MATH64_H */
