/**************************************************************************
 *
 * Copyright (c) 2006-2007 Tungsten Graphics, Inc., Cedar Park, TX., USA
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/
/*
 * Authors: Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */

#ifndef __QNX__
#include <linux/export.h>
#else
#include <drm/drmP.h>
#ifdef __X86__
#include <asm/cpufeature.h>
#include <asm/special_insns.h>
#endif
#include <asm-generic/barrier.h>
#ifdef __X86__
#include <kernel/cpu/cpu.h>
#endif
#endif
#include <drm/drmP.h>

void
drm_clflush_virt_range(void *addr, unsigned long length)
{
#if defined(CONFIG_X86)
#ifndef __QNX__
	if (static_cpu_has(X86_FEATURE_CLFLUSH)) {
#else
    cpu_detect( &boot_cpu_data );
    {
#endif
		const int size = boot_cpu_data.x86_clflush_size;
		void *end = addr + length;
		addr = (void *)(((unsigned long)addr) & -size);
		mb();
		for (; addr < end; addr += size)
			clflushopt(addr);
		clflushopt(end - 1); /* force serialisation */
		mb();
		return;
	}

#ifndef __QNX__
	if (wbinvd_on_all_cpus())
		printk(KERN_ERR "Timed out waiting for cache flush.\n");
#endif
#else
#ifndef __QNX__
	printk(KERN_ERR "Architecture has no drm_cache.c support\n");
	WARN_ON_ONCE(1);
#else
    fprintf( stderr, "%s(): error: Architecture has no drm_cache.c support\n", __FUNCTION__ );
#endif
#endif
}
