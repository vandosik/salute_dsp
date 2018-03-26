#ifndef __LINUX_COMPILER_H
#error "Please don't include <linux/compiler-watcom.h> directly, include <linux/compiler.h> instead."
#endif

/* Optimization barrier */

#define barrier()                   __asm__ __volatile__("": : :"memory")
#define barrier_data( ptr )         __asm__ __volatile__("": :"r"(ptr) :"memory")

#ifndef __pure
#define __pure                      /* __attribute__((pure)) */
#endif
#define __aligned( x )              /* __attribute__((aligned(x))) */
#define __printf( a, b )            /* __attribute__((format(printf, a, b))) */
#define __scanf( a, b )             /* __attribute__((format(scanf, a, b))) */
#define __attribute_const__         /* __attribute__((__const__)) */
#define __maybe_unused              /* __attribute__((unused)) */
#define __always_unused             /* __attribute__((unused)) */
#define __used                      /* __attribute__((__used__)) */
#define __must_check                /* __attribute__((warn_unused_result)) */
#define __malloc                    /* __attribute__((__malloc__)) */

#define __builtin_expect( x, y )    (x)

#define __UNIQUE_ID( prefix )       __PASTE( __PASTE( __UNIQUE_ID_, prefix ), __COUNTER__ )
