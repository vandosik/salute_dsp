#ifndef __LINUX_BITMAP_H
#define __LINUX_BITMAP_H

#ifndef __ASSEMBLY__

#include <linux/types.h>
#include <linux/bitops.h>
//#include <linux/string.h>
#include <linux/kernel.h>
#ifdef __QNX__
/* To avoid including string.h header */
void* memset( void* dst, int c, size_t length );
#endif

/*
 * bitmaps provide bit arrays that consume one or more unsigned
 * longs.  The bitmap interface and available operations are listed
 * here, in bitmap.h
 *
 * Function implementations generic to all architectures are in
 * lib/bitmap.c.  Functions implementations that are architecture
 * specific are in various include/asm-<arch>/bitops.h headers
 * and other arch/<arch> specific files.
 *
 * See lib/bitmap.c for more details.
 */

/*
 * The available bitmap operations and their rough meaning in the
 * case that the bitmap is a single unsigned long are thus:
 *
 * Note that nbits should be always a compile time evaluable constant.
 * Otherwise many inlines will generate horrible code.
 *
 * bitmap_zero(dst, nbits)			*dst = 0UL
 * bitmap_fill(dst, nbits)			*dst = ~0UL
 * bitmap_copy(dst, src, nbits)			*dst = *src
 * bitmap_and(dst, src1, src2, nbits)		*dst = *src1 & *src2
 * bitmap_or(dst, src1, src2, nbits)		*dst = *src1 | *src2
 * bitmap_xor(dst, src1, src2, nbits)		*dst = *src1 ^ *src2
 * bitmap_andnot(dst, src1, src2, nbits)	*dst = *src1 & ~(*src2)
 * bitmap_complement(dst, src, nbits)		*dst = ~(*src)
 * bitmap_equal(src1, src2, nbits)		Are *src1 and *src2 equal?
 * bitmap_intersects(src1, src2, nbits) 	Do *src1 and *src2 overlap?
 * bitmap_subset(src1, src2, nbits)		Is *src1 a subset of *src2?
 * bitmap_empty(src, nbits)			Are all bits zero in *src?
 * bitmap_full(src, nbits)			Are all bits set in *src?
 * bitmap_weight(src, nbits)			Hamming Weight: number set bits
 * bitmap_set(dst, pos, nbits)			Set specified bit area
 * bitmap_clear(dst, pos, nbits)		Clear specified bit area
 * bitmap_find_next_zero_area(buf, len, pos, n, mask)	Find bit free area
 * bitmap_find_next_zero_area_off(buf, len, pos, n, mask)	as above
 * bitmap_shift_right(dst, src, n, nbits)	*dst = *src >> n
 * bitmap_shift_left(dst, src, n, nbits)	*dst = *src << n
 * bitmap_remap(dst, src, old, new, nbits)	*dst = map(old, new)(src)
 * bitmap_bitremap(oldbit, old, new, nbits)	newbit = map(old, new)(oldbit)
 * bitmap_onto(dst, orig, relmap, nbits)	*dst = orig relative to relmap
 * bitmap_fold(dst, orig, sz, nbits)		dst bits = orig bits mod sz
 * bitmap_parse(buf, buflen, dst, nbits)	Parse bitmap dst from kernel buf
 * bitmap_parse_user(ubuf, ulen, dst, nbits)	Parse bitmap dst from user buf
 * bitmap_parselist(buf, dst, nbits)		Parse bitmap dst from kernel buf
 * bitmap_parselist_user(buf, dst, nbits)	Parse bitmap dst from user buf
 * bitmap_find_free_region(bitmap, bits, order)	Find and allocate bit region
 * bitmap_release_region(bitmap, pos, order)	Free specified bit region
 * bitmap_allocate_region(bitmap, pos, order)	Allocate specified bit region
 * bitmap_from_u32array(dst, nbits, buf, nwords) *dst = *buf (nwords 32b words)
 * bitmap_to_u32array(buf, nwords, src, nbits)	*buf = *dst (nwords 32b words)
 */

/*
 * Also the following operations in asm/bitops.h apply to bitmaps.
 *
 * set_bit(bit, addr)			*addr |= bit
 * clear_bit(bit, addr)			*addr &= ~bit
 * change_bit(bit, addr)		*addr ^= bit
 * test_bit(bit, addr)			Is bit set in *addr?
 * test_and_set_bit(bit, addr)		Set bit and return old value
 * test_and_clear_bit(bit, addr)	Clear bit and return old value
 * test_and_change_bit(bit, addr)	Change bit and return old value
 * find_first_zero_bit(addr, nbits)	Position first zero bit in *addr
 * find_first_bit(addr, nbits)		Position first set bit in *addr
 * find_next_zero_bit(addr, nbits, bit)	Position next zero bit in *addr >= bit
 * find_next_bit(addr, nbits, bit)	Position next set bit in *addr >= bit
 */

/*
 * The DECLARE_BITMAP(name,bits) macro, in linux/types.h, can be used
 * to declare an array named 'name' of just enough unsigned longs to
 * contain all bit positions from 0 to 'bits' - 1.
 */

/*
 * lib/bitmap.c provides these functions:
 */

extern void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
                        const unsigned long *bitmap2, unsigned int nbits);
extern int __bitmap_weight(const unsigned long *bitmap, unsigned int nbits);

extern void bitmap_set(unsigned long *map, unsigned int start, int len);

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define small_const_nbits(nbits) \
        (__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)

static inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
        if (small_const_nbits(nbits))
                *dst = 0UL;
        else {
                unsigned int len = BITS_TO_LONGS(nbits) * sizeof(unsigned long);
                memset(dst, 0, len);
        }
}

static inline void bitmap_or(unsigned long *dst, const unsigned long *src1,
                        const unsigned long *src2, unsigned int nbits)
{
        if (small_const_nbits(nbits))
                *dst = *src1 | *src2;
        else
                __bitmap_or(dst, src1, src2, nbits);
}

static inline int bitmap_empty(const unsigned long *src, unsigned nbits)
{
        if (small_const_nbits(nbits))
                return ! (*src & BITMAP_LAST_WORD_MASK(nbits));

        return find_first_bit(src, nbits) == nbits;
}

static __always_inline int bitmap_weight(const unsigned long *src, unsigned int nbits)
{
        if (small_const_nbits(nbits))
                return hweight_long(*src & BITMAP_LAST_WORD_MASK(nbits));
        return __bitmap_weight(src, nbits);
}

#endif /* __ASSEMBLY__ */

#endif /* __LINUX_BITMAP_H */
