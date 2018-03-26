#ifndef __ASM_GENERIC_PAGE_H
#define __ASM_GENERIC_PAGE_H
/*
 * Generic page.h implementation, for NOMMU architectures.
 * This provides the dummy definitions for the memory management.
 */

#ifdef CONFIG_MMU
#error need to prove a real asm/page.h
#endif


/* PAGE_SHIFT determines the page size */

#define PAGE_SHIFT      12
#ifdef __ASSEMBLY__
#define PAGE_SIZE       (1 << PAGE_SHIFT)
#else
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#endif
#define PAGE_MASK       (~(PAGE_SIZE-1))

#endif /* __ASM_GENERIC_PAGE_H */
