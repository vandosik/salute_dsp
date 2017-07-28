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
#include "board.h"
#include <arm/mc1892vm14.h>


uint32_t mc1892vm14_get_cpu_clk(void)
{
	uint32_t apll_mode;

	apll_mode = in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_SEL_APLL_REG) & 0xFF;
	return (apll_mode + 1) * EL24D1_XTI_FREQ;
}

uint32_t mc1892vm14_get_spll_clk(void)
{
	uint32_t spll_mode;

	spll_mode = in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_SEL_SPLL_REG) & 0xFF;
	return (spll_mode + 1) * EL24D1_XTI_FREQ;
}

uint32_t mc1892vm14_get_l1_clk(unsigned spll_clk)
{
	uint32_t div_sys0_ctr;

	div_sys0_ctr = (in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_DIV_SYS0_REG) & 0x01) + 1;
	return spll_clk / div_sys0_ctr;
}

uint32_t mc1892vm14_get_l2_clk(unsigned l1_clk)
{
	uint32_t div_sys1_ctr;

	div_sys1_ctr = (in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_DIV_SYS1_REG) & 0x01) + 1;
	return l1_clk / div_sys1_ctr;
}
