#ifndef __ASM_GENERIC_BITS_PER_LONG
#define __ASM_GENERIC_BITS_PER_LONG

#ifdef __QNX__
#if __LONG_BITS__-0 == 64
#define CONFIG_64BIT
#endif
#endif

#ifdef CONFIG_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif /* CONFIG_64BIT */

#endif /* __ASM_GENERIC_BITS_PER_LONG */
