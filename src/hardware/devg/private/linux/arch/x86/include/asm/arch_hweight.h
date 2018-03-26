#ifndef _ASM_X86_HWEIGHT_H
#define _ASM_X86_HWEIGHT_H

#ifdef __QNX__
#include <drm/drm.h>
#define CONFIG_64BIT
#endif
#include <asm/cpufeatures.h>

#ifdef CONFIG_64BIT
/* popcnt %edi, %eax */
#define POPCNT32 ".byte 0xf3,0x0f,0xb8,0xc7"
/* popcnt %rdi, %rax */
#define POPCNT64 ".byte 0xf3,0x48,0x0f,0xb8,0xc7"
#define REG_IN "D"
#define REG_OUT "a"
#else
/* popcnt %eax, %eax */
#define POPCNT32 ".byte 0xf3,0x0f,0xb8,0xc0"
#define REG_IN "a"
#define REG_OUT "a"
#endif

#ifndef __QNX__
#define __HAVE_ARCH_SW_HWEIGHT
#else
unsigned int __sw_hweight32(unsigned int w);
unsigned long __sw_hweight64(__u64 w);
#endif

static __always_inline unsigned int __arch_hweight32(unsigned int w)
{
#ifndef __QNX__
	unsigned int res;

	asm (ALTERNATIVE("call __sw_hweight32", POPCNT32, X86_FEATURE_POPCNT)
			 : "="REG_OUT (res)
			 : REG_IN (w));

	return res;
#else
    return __sw_hweight32( w );
#endif
}

static inline unsigned int __arch_hweight16(unsigned int w)
{
	return __arch_hweight32(w & 0xffff);
}

static inline unsigned int __arch_hweight8(unsigned int w)
{
	return __arch_hweight32(w & 0xff);
}

#ifdef CONFIG_X86_32
static inline unsigned long __arch_hweight64(__u64 w)
{
	return  __arch_hweight32((u32)w) +
		__arch_hweight32((u32)(w >> 32));
}
#else
static __always_inline unsigned long __arch_hweight64(__u64 w)
{
#ifndef __QNX__
	unsigned long res;

	asm (ALTERNATIVE("call __sw_hweight64", POPCNT64, X86_FEATURE_POPCNT)
			 : "="REG_OUT (res)
			 : REG_IN (w));

	return res;
#else
    return __sw_hweight64( w );
#endif
}
#endif /* CONFIG_X86_32 */

#endif
