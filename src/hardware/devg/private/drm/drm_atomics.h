#ifndef _DRM_ATOMICS_H
#define _DRM_ATOMICS_H

#ifndef likely
#define likely(x)      __builtin_expect( !!(x), 1 )
#endif
#ifndef unlikely
#define unlikely(x)    __builtin_expect( !!(x), 0 )
#endif

#ifdef __QNXNTO__
#include _NTO_CPU_HDR_(smpxchg.h)
#endif

typedef volatile unsigned atomic_t;

#if !defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)

/* copied from atomic.h */
extern unsigned atomic_add_value(volatile unsigned *__loc, unsigned __incr);
extern unsigned atomic_sub_value(volatile unsigned *__loc, unsigned __decr);

static inline void atomic_set( atomic_t *atom, unsigned value )
{
	*atom = value;
}

static inline unsigned atomic_read( atomic_t *value )
{
	return ( *value);
}

static inline unsigned atomic_xchg( atomic_t *data, unsigned value )
{
	unsigned tmp = *data;
	*data = value;
	return ( tmp);
}

static inline unsigned atomic_cmpxchg( atomic_t *data, unsigned cmp, unsigned value )
{
#ifndef __QNXNTO__
	unsigned tmp = *data;
	if ( tmp == cmp )
		*data = value;

	return ( tmp);
#else
    return _smp_cmpxchg( data, cmp, value );
#endif
}

static inline unsigned atomic_add_return( atomic_t *atom, int value )
{
	return atomic_add_value( atom, value ) + value;
}

static inline unsigned atomic_sub_return( atomic_t *atom, int value )
{
	return atomic_sub_value( atom, value ) - value;
}

#else

static inline void atomic_set( atomic_t *atom, unsigned value )
{
	__sync_synchronize();
	*atom = value;
}

static inline unsigned atomic_read( atomic_t *atom )
{
	unsigned data = *atom;
	__sync_synchronize();

	return ( data);
}

static inline unsigned atomic_xchg( atomic_t *atom, unsigned value )
{
	return __sync_lock_test_and_set(atom, value);
}

static inline unsigned atomic_cmpxchg( atomic_t *data, unsigned cmp, unsigned value )
{
	return __sync_val_compare_and_swap(data, cmp, value);
}

static inline unsigned atomic_add_return( atomic_t *atom, int value )
{
	return __sync_add_and_fetch(atom, value);
}

static inline unsigned atomic_sub_return( atomic_t *atom, int value )
{
	return __sync_add_and_fetch(atom, -value);
}

#endif

/** linux: x86/include/asm/atomic.h
 * __atomic_add_unless - add unless the number is already a given value
 * @v: pointer of type atomic_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as @v was not already @u.
 * Returns the old value of @v.
 */
static inline int __atomic_add_unless( atomic_t *v, int a, int u )
{
	int c, old;

	c = atomic_read( v );
	for ( ; ; )
	{
		if ( unlikely( c == (u) ) )
			break;

		old = atomic_cmpxchg( v, c, c + (a) );

		if ( likely( old == c ) )
			break;

		c = old;
	}

	return c;
}

/** linux: linux/atomic.h
 * atomic_add_unless - add unless the number is already a given value
 * @v: pointer of type atomic_t
 * @a: the amount to add to v...
 * @u: ...unless v is equal to u.
 *
 * Atomically adds @a to @v, so long as @v was not already @u.
 * Returns non-zero if @v was not @u, and zero otherwise.
 */
static inline int atomic_add_unless( atomic_t *v, int a, int u )
{
	return __atomic_add_unless( v, a, u ) != u;
}

static inline unsigned atomic_inc_return( atomic_t *atom )
{
	return atomic_add_return(atom, 1);
}

static inline void atomic_inc( atomic_t *atom )
{
	atomic_add_return(atom, 1);
}

static inline unsigned atomic_dec_return( atomic_t *atom )
{
	return atomic_sub_return(atom, 1);
}

static inline int atomic_dec_and_test( atomic_t *atom )
{
	return atomic_dec_return(atom) == 0;
}

#endif /* _DRM_ATOMICS_H */
