/*
 * $QNXLicenseC: 
 * Copyright 2011,2012  QNX Software Systems.  
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
#include <hw/inout.h>
#include <arm/mc1892vm14.h>
#include "board.h"

void mc1892vm14_hwinit(void)
{
	// Turns UART2 and UART3 on
	out32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG, 
		  in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG)  |
									MC1892VM14_CMCTR_GATE_SYS_EMAC_EN  |
									MC1892VM14_CMCTR_GATE_SYS_USBIC_EN |
									MC1892VM14_CMCTR_GATE_SYS_UART2_EN | 
									MC1892VM14_CMCTR_GATE_SYS_UART3_EN);

	// Turns I2C0 / I2C1 / I2C2 on
	out32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG, 
		  in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG) |
									MC1892VM14_CMCTR_GATE_SYS_I2C0_EN |
                                    MC1892VM14_CMCTR_GATE_SYS_I2C1_EN |
                                    MC1892VM14_CMCTR_GATE_SYS_I2C2_EN);
	
	// Set GPIOC 14 pin direction to OUT
	out32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_DDR, in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_DDR) | (1 << 14));
}
