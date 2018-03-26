#ifndef _LINUX_MM_H
#define _LINUX_MM_H

//#include <linux/errno.h>

#ifdef __KERNEL__

//#include <linux/mmdebug.h>
//#include <linux/gfp.h>
#include <linux/bug.h>
#include <linux/list.h>
//#include <linux/mmzone.h>
//#include <linux/rbtree.h>
//#include <linux/atomic.h>
//#include <linux/debug_locks.h>
//#include <linux/mm_types.h>
//#include <linux/range.h>
//#include <linux/pfn.h>
//#include <linux/percpu-refcount.h>
//#include <linux/bit_spinlock.h>
//#include <linux/shrinker.h>
//#include <linux/resource.h>
//#include <linux/page_ext.h>
#include <linux/err.h>
//#include <linux/page_ref.h>

//#include <asm/page.h>
#include <asm/pgtable.h>
//#include <asm/processor.h>

#define offset_in_page(p)       ((unsigned long)(p) & ~PAGE_MASK)

#endif /* __KERNEL__ */
#endif /* _LINUX_MM_H */
