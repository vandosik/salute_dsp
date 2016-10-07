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
#include <arm/mc1892vm14.h>

void mc1892vm14_init_raminfo(unsigned ram_size)
{
	if (MEG(ram_size) <= MEG(1024))
		add_ram(MC1892VM14_DDRMC0_BASE, MEG(ram_size));
	else {
		if (MEG(ram_size) < MEG(2048)) {
			add_ram(MC1892VM14_DDRMC0_BASE, MEG(1024));
			add_ram(MC1892VM14_DDRMC1_BASE, MEG(ram_size) - MEG(1024));
		}
		else {
			add_ram(MC1892VM14_DDRMC0_BASE, MEG(1024));
			add_ram(MC1892VM14_DDRMC1_BASE, MEG(1024));
		}
	}
}
