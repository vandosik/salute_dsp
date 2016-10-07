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

int board_smp_start(unsigned cpu, void(*start)(void))
{
	
	if (debug_flag > 1) {
		kprintf("board_smp_start: cpu%d -> %x\n", cpu, start);
	}

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

unsigned board_smp_adjust_num(unsigned cpu)
{
	return cpu;
}
