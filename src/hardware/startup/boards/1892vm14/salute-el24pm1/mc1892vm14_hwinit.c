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
	
	// Turns VPOUT / VPIN / GPU / VPU on
	out32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_CORE_REG, 
			in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_CORE_REG)  |
									//MC1892VM14_CMCTR_GATE_CORE_GPU_EN  |
									//MC1892VM14_CMCTR_GATE_CORE_VPU_EN |
									MC1892VM14_CMCTR_GATE_CORE_VPIN_EN | 
									MC1892VM14_CMCTR_GATE_CORE_VPOUT_EN);
	
	// Turns UART2 and UART3 on
	out32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG, 
		  in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG)  |
									MC1892VM14_CMCTR_GATE_SYS_SDMMC0_EN |
									MC1892VM14_CMCTR_GATE_SYS_SDMMC1_EN |
									MC1892VM14_CMCTR_GATE_SYS_EMAC_EN  |
									MC1892VM14_CMCTR_GATE_SYS_UART2_EN | 
									MC1892VM14_CMCTR_GATE_SYS_UART3_EN | 
									MC1892VM14_CMCTR_GATE_SYS_NAND_EN);

	// Turns I2C0 / I2C1 / I2C2 on
	out32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG, 
		  in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_GATE_SYS_REG) |
									MC1892VM14_CMCTR_GATE_SYS_I2C0_EN |
                                    MC1892VM14_CMCTR_GATE_SYS_I2C1_EN |
                                    MC1892VM14_CMCTR_GATE_SYS_I2C2_EN);
	

	// Switch GPIOC 14 pin to the HDMI it66121
// 	out32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_CTL, in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_CTL) | (1 << 14));
// 
// 	// Switch GPIOA pins 29,30 to the i2c-0
// 	out32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOA_CTL, in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOA_CTL) | (0x3 << 29));
	
	// Switch GPIOD pins 22,23 to the i2c-1 and pins 24,25 to the i2c-2
// 	out32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOD_CTL, in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOD_CTL) | (0xf << 22));
	kprintf("GPIOA_CTL 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOA_CTL));
	kprintf("GPIOA_DDR 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOA_DDR));
	kprintf("GPIOA_DR  0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOA_DR));
	kprintf("GPIOB_CTL 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOB_CTL));
	kprintf("GPIOB_DDR 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOB_DDR));
	kprintf("GPIOB_DR  0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOB_DR));
	kprintf("GPIOC_CTL 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_CTL));
	kprintf("GPIOC_DDR 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_DDR));
	kprintf("GPIOC_DR  0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOC_DR));
	kprintf("GPIOD_CTL 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOD_CTL));
	kprintf("GPIOD_DDR 0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOD_DDR));
	kprintf("GPIOD_DR  0x%X\n", in32(MC1892VM14_GPIO_BASE + MC1892VM14_GPIOD_DR));
}
