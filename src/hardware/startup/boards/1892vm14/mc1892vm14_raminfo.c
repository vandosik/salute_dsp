/*
 * $QNXLicenseC: 
 * Copyright 2011, QNX Software Systems.  
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



/*
 * init_raminfo.c
 *	Tell syspage about our RAM configuration
 */
#include "startup.h"
#include "fdt_startup_func.h"
#include <arm/mc1892vm14.h>

#define DDRMC_RAM_SIZE			1024
static int user_memory_setup = 0;

void mc1892vm14_init_raminfo(char *opts)
{
#if 0
	unsigned	reg;
	
	reg = in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_CORE_REG);
	
	if (reg & MC1892VM14_CMCTR_GATE_CORE_DDR0_EN) {
		add_ram(MC1892VM14_DDRMC0_BASE, MEG(DDRMC_RAM_SIZE));
	}
	
	if (reg & MC1892VM14_CMCTR_GATE_CORE_DDR1_EN) {
		add_ram(MC1892VM14_DDRMC1_BASE, MEG(DDRMC_RAM_SIZE));
	}
#else
	if ( opts != NULL ) {
		char *p;
		paddr_t	start;
		size_t	size;
		start = getsize(opts, &p);
		if(*p == ',') {
			size = getsize(p + 1, &p);
			if ( size != 0 ) {
				add_ram(start ? start : MC1892VM14_DDRMC0_BASE, size);
				user_memory_setup++;
			}
		}
	} 
	if ( !user_memory_setup ) {
		if (fdt_addr != NULL_PADDR32 && (fdt_flags & USE_FDT_MEM_CONFIG)) {
			uintptr_t       base;
			uint32_t        *fdt_data;
			int             lenp = -1;
			int             i = 0;
			
			
			base = startup_io_map(fdt_size , fdt_addr );
			
			fdt_data = (uint32_t*)recurse_deep_search((const void *)base, 0, "memory", "reg", &lenp);
			
			if ( fdt_data != NULL)
			{
				for (i = 0; i< lenp/sizeof(uint32_t); i += 2)
				{  
					add_ram( convert_fdt32(fdt_data[i]), convert_fdt32( fdt_data[i + 1] ) );
					if (debug_flag > 1) {
						kprintf( "fdt: ADD RAM addr: %x size %x\n", convert_fdt32(fdt_data[i]), convert_fdt32(fdt_data[i+1]) );
					}
				}
			}
			startup_io_unmap(base);
		}
		else {
			add_ram(MC1892VM14_DDRMC0_BASE, MEG(DDRMC_RAM_SIZE));
		}
	}
#endif
}
