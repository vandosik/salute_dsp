#ifndef ARCH_X86_CPU_H
#define ARCH_X86_CPU_H


#ifdef __QNX__
#include <asm/cpufeature.h>
void cpu_detect(struct cpuinfo_x86 *c);
void get_model_name(struct cpuinfo_x86 *c);
#endif
extern void get_cpu_cap(struct cpuinfo_x86 *c);
extern void cpu_detect_cache_sizes(struct cpuinfo_x86 *c);
#endif /* ARCH_X86_CPU_H */
