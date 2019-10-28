/*
 * $QNXLicenseC: 
 * Copyright 2012, QNX Software Systems.  
 * Copyright 2013, Adeneo Embedded.
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $
 */


#include "startup.h"
#include "arm/mpcore.h"
#include <arm/mc1892vm14.h>

/*
 * Board-specific cold boot code
 */
unsigned	startup_smp_start;
unsigned	startup_reset_vec;
unsigned	startup_reset_vec_addr;
unsigned	board_smp_new = 0;

unsigned board_smp_num_cpu()
{
	unsigned num;

	num = in32(mpcore_scu_base + MPCORE_SCU_CONFIG);
	if (debug_flag) {
		kprintf("SCU_CONFIG = %x, %d cpus\n", num, (num & 0xf) + 1);
	}
	return (num & 0xf) + 1;
}

void board_smp_init(struct smp_entry *smp, unsigned num_cpus)
{
	smp->send_ipi = (void *) &sendipi_gic;
	/*
	 * Use smp_spin_pl310 instead of the default smp_spin routine
	 */
	smp_spin_vaddr = (void(*)(void)) &smp_spin_pl310;
	callout_register_data(&smp_spin_vaddr, (void *) MC1892VM14_PL310_BASE);
}

#define SPRAM_MAGIC_OFFSET				0xFFF8
#define SPRAM_ADDR_OFFSET				0xFFF4
#define SMP_START_MAGIC					0xDEADBEEF

extern char mcom02_secondary_trampoline;
extern char mcom02_secondary_trampoline_end;
extern unsigned long mcom02_secondary_boot_vector;
extern void mcom02_secondary_startup(void);
int board_smp_start(unsigned cpu, void(*start)(void))
{
	
	if (debug_flag > 1) {
		kprintf("board_smp_start: cpu%d -> %x\n", cpu, start);
	}

	if ( !board_smp_new ) {
		/* 
		* FIXME: Support hack used in u-boot.
		* Core1 always poll for magic value in SPRAM and when it will 
		* receive it jumps to the start addr stored in SPRAM.
		* So we have to write magic and core1 start addr to SPRAM.
		*
		*/
		out32(MC1892VM14_SPRAM_START + SPRAM_MAGIC_OFFSET, SMP_START_MAGIC);
		out32(MC1892VM14_SPRAM_START + SPRAM_ADDR_OFFSET, start);
		return 1;
	}

	if (debug_flag > 1) {
		kprintf("board_smp_start: cpu%d -> %x\n", cpu, start);
	}
	uint32_t trampoline_sz = &mcom02_secondary_trampoline_end -
			    &mcom02_secondary_trampoline;
	if (debug_flag > 1) {
		kprintf("board_smp_start: trampoline_sz %d\n", trampoline_sz);
		kprintf("board_smp_start: mcom02_secondary_boot_vector %d\n", mcom02_secondary_boot_vector);
	}
	mcom02_secondary_boot_vector = (unsigned long)mcom02_secondary_startup;
	if (debug_flag > 1) {
		kprintf("board_smp_start: mcom02_secondary_boot_vector %d\n", mcom02_secondary_boot_vector);
	}
	void *mc1892vm14_spram_start = (void *)MC1892VM14_SPRAM_START;
	memcpy(mc1892vm14_spram_start, &mcom02_secondary_trampoline, trampoline_sz);

	/* Remap BOOT so that CPU1 boots from SPRAM where the boot
	 * vector is stored */
	if (debug_flag > 1) {
		kprintf("board_smp_start: Remap BOOT\n");
	}
	out32(MC1892VM14_SMCTR_BASE + SMCTR_BOOT_REMAP, SMCTR_BOOT_REMAP_SPRAM);

	/* Turn on power domain for CPU1 */
	if (debug_flag > 1) {
		kprintf("board_smp_start: Turn on power domain for CPU1 %d\n", (1 << (cpu + 1)));
	}
	out32(MC1892VM14_PMCTR_BASE + MC1892VM14_PMCTR_SYS_PWR_UP, (1 << (cpu + 1)));
	return 1;
}

unsigned board_smp_adjust_num(unsigned cpu)
{
	return cpu;
}
