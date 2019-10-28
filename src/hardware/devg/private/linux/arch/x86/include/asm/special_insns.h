#ifndef _ASM_X86_SPECIAL_INSNS_H
#define _ASM_X86_SPECIAL_INSNS_H


#ifdef __KERNEL__

#ifdef __QNX__
#include <asm/alternative.h>
#include <linux/stringify.h>
#endif
//#include <asm/nops.h>

#ifndef  __QNX4__
static inline void clflushopt(volatile void *__p)
{
	alternative_io(".byte " __stringify(NOP_DS_PREFIX) "; clflush %P0",
		       ".byte 0x66; clflush %P0",
		       X86_FEATURE_CLFLUSHOPT,
		       "+m" (*(volatile char __force *)__p));
}
#endif  /* __QNX4__ */

#endif /* __KERNEL__ */

#endif /* _ASM_X86_SPECIAL_INSNS_H */
