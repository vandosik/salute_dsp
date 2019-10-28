/*
 * (c) 2019, SWD Embedded Systems Limited, http://www.kpda.ru
 */
 
/*
 *  Copyright 2017 RnD Center "ELVEES", JSC
 */
 
/*
 *  Copyright (C) 2001 Deep Blue Solutions Ltd.
 *  Copyright (C) 2005 ARM Ltd.
 */

#include <arm/mc1892vm14.h>

.text
	
.globl	v7_invalidate_l1
v7_invalidate_l1:
	mov     r0, #0
	mcr     p15, 2, r0, c0, c0, 0
	mrc     p15, 1, r0, c0, c0, 0

	movw    r1, #0x7fff
	and     r2, r1, r0, lsr #13

	movw    r1, #0x3ff

	and     r3, r1, r0, lsr #3			@ NumWays - 1
	add     r2, r2, #1					@ NumSets

	and     r0, r0, #0x7
	add     r0, r0, #4					@ SetShift

	clz     r1, r3						@ WayShift
	add     r4, r3, #1					@ NumWays
1:     sub     r2, r2, #1				@ NumSets--
	mov     r3, r4						@ Temp = NumWays
2:     subs    r3, r3, #1				@ Temp--
	mov     r5, r3, lsl r1
	mov     r6, r2, lsl r0
	orr     r5, r5, r6					@ Reg = (Temp<<WayShift)|(NumSets<<SetShift)
	mcr     p15, 0, r5, c7, c6, 2
	bgt     2b
	cmp     r2, #0
	bgt     1b
	dsb     st
	isb
	mov     pc, lr

.globl	mcom02_secondary_startup
.align 4,0x90
mcom02_secondary_startup:
	ldr	r0, =MC1892VM14_SMCTR_BASE		@ SMCTR base address
	mov	r1, #0
	str	r1, [r0, #4]					@ remap BOOT to NORMPORT
	mrc	p15, 4, r0, c15, c0, 0			@ get SCU base address
	mrc	p15, 0, r1, c0, c0, 5			@ read MIPDR
	and	r1, r1, #3						@ mask out cpu ID
	lsl	r1, r1, #3						@ we will shift by cpu_id * 8 bits
	ldr	r2, [r0, #8]					@ SCU Power Status Register
	mov	r3, #3
	lsl	r3, r3, r1
	bic	r2, r2, r3						@ Clear bits of our CPU (Run Mode)
	str	r2, [r0, #8]					@ write back

	bl	v7_invalidate_l1
	b	smp_start

.globl	mcom02_secondary_trampoline
.align 4,0x90
mcom02_secondary_trampoline:
	ldr	pc, 1f
.globl	mcom02_secondary_boot_vector
.align 4,0x90
mcom02_secondary_boot_vector:
1:	.space	4
.globl	mcom02_secondary_trampoline_end
.align 4,0x90
mcom02_secondary_trampoline_end:

