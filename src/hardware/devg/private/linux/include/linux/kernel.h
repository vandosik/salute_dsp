#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

//#include <stdarg.h>
//#include <linux/linkage.h>
//#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/bitops.h>
//#include <linux/log2.h>
//#include <linux/typecheck.h>
//#include <linux/printk.h>
//#include <asm/byteorder.h>
#include <uapi/linux/kernel.h>

#ifndef SIZE_MAX
#define SIZE_MAX        (~(size_t)0)
#endif

#define U64_MAX         ((uint64_t)~0ULL)
#define S64_MAX         ((int64_t)(U64_MAX>>1))
#define S64_MIN         ((int64_t)(-S64_MAX - 1))

#define ALIGN(x, a)             __ALIGN_KERNEL((x), (a))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*
* This looks more complex than it should be. But we need to
* get the type for the ~ right in round_down (it needs to be
* as wide as the result!), and we want to evaluate the macro
* arguments just once each.
*/
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define DIV_ROUND_UP __KERNEL_DIV_ROUND_UP

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#ifndef __WATCOMC__
#define DIV_ROUND_CLOSEST(x, divisor)(                  \
{                                                       \
        typeof(x) __x = x;                              \
        typeof(divisor) __d = divisor;                  \
        (((typeof(x))-1) > 0 ||                         \
         ((typeof(divisor))-1) > 0 || (__x) > 0) ?      \
                (((__x) + ((__d) / 2)) / (__d)) :       \
                (((__x) - ((__d) / 2)) / (__d));        \
}                                                       \
)
#else
static inline uint64_t DIV_ROUND_CLOSEST( uint64_t x, uint64_t divisor ) 
{
    /* Bug: signed types not supported */
    return (x + (divisor / 2)) / divisor;
}
#endif
/*
 * Same as above but for u64 dividends. divisor must be a 32-bit
 * number.
 */
#define DIV_ROUND_CLOSEST_ULL(x, divisor)(              \
{                                                       \
        typeof(divisor) __d = divisor;                  \
        unsigned long long _tmp = (x) + (__d) / 2;      \
        do_div(_tmp, __d);                              \
        _tmp;                                           \
}                                                       \
)

/*
 * Multiplies an integer by a fraction, while avoiding unnecessary
 * overflow or loss of precision.
 */
#define mult_frac(x, numer, denom)(                     \
{                                                       \
        typeof(x) quot = (x) / (denom);                 \
        typeof(x) rem  = (x) % (denom);                 \
        (quot * (numer)) + ((rem * (numer)) / (denom)); \
}                                                       \
)

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((uint32_t)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((uint32_t)(n))

/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#ifdef __QNX__
#undef __min
#endif
#define __min(t1, t2, min1, min2, x, y) ({              \
        t1 min1 = (x);                                  \
        t2 min2 = (y);                                  \
        (void) (&min1 == &min2);                        \
        min1 < min2 ? min1 : min2; })

#ifdef __QNX__
#undef __max
#endif
#define __max(t1, t2, max1, max2, x, y) ({              \
        t1 max1 = (x);                                  \
        t2 max2 = (y);                                  \
        (void) (&max1 == &max2);                        \
        max1 > max2 ? max1 : max2; })

/**
 * clamp - return a value clamped to a given range with strict typechecking
 * @val: current value
 * @lo: lowest allowable value
 * @hi: highest allowable value
 *
 * This macro does strict typechecking of lo/hi to make sure they are of the
 * same type as val.  See the unnecessary pointer comparisons.
 */
#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y)                               \
        __min(type, type,                               \
              __UNIQUE_ID(min1_), __UNIQUE_ID(min2_),   \
              x, y)

#define max_t(type, x, y)                               \
        __max(type, type,                               \
              __UNIQUE_ID(min1_), __UNIQUE_ID(min2_),   \
              x, y)

/**
 * clamp_t - return a value clamped to a given range using a given type
 * @type: the type of variable to use
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of type
 * 'type' to make all the comparisons.
 */
#define clamp_t(type, val, lo, hi) min_t(type, max_t(type, val, lo), hi)

/**
 * clamp_val - return a value clamped to a given range using val's type
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of whatever
 * type the input argument 'val' is.  This is useful when val is an unsigned
 * type and min and max are literals that will otherwise be assigned a signed
 * integer type.
 */
#define clamp_val(val, lo, hi) clamp_t(typeof(val), val, lo, hi)


/*
 * swap - swap value of @a and @b
 */
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#endif  /* _LINUX_KERNEL_H */
