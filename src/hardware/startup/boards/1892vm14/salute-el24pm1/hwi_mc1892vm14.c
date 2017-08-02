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


#include "startup.h"
#include "board.h"
#include "hwinfo_private.h"
#include <drvr/hwinfo.h>                // for hwi support routines in libdrvr

/*
 * Add 1892VM14 SoC devices to the hardware info section of the syspage.
*/
#define VM14_HWI_ENET	"enet"

void hwi_mc1892vm14()
{
	unsigned hwi_bus_internal = 0;

	/* add  UART */
	{
		unsigned hwi_off;
		hwiattr_uart_t uart_attr = HWIATTR_UART_T_INITIALIZER;
		/* All the UARTs operate from a  functional clock of 144 MHz only */
		struct hwi_inputclk clksrc = {.clk = MC1892VM14_UART_CLK_FREQ, .div = 16};

		/* each UART has an interrupt */
		HWIATTR_UART_SET_NUM_IRQ(&uart_attr, 1);
		HWIATTR_UART_SET_NUM_CLK(&uart_attr, 1);

		/* create uart0 */
		HWIATTR_UART_SET_LOCATION(&uart_attr, MC1892VM14_UART1_BASE, MC1892VM14_UART_IO_SIZE, 0,
																	hwi_find_as(MC1892VM14_UART1_BASE, 1));
		hwi_off = hwidev_add_uart(MC1892VM14_HWI_UART, &uart_attr, hwi_bus_internal);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		hwitag_set_ivec(hwi_off, 0, MC1892VM14_IRQ_UART1);
		hwitag_set_inputclk(hwi_off, 0, &clksrc);
	}

	/* add the WATCHDOG device */
	{
		unsigned hwi_off;
		hwiattr_timer_t attr = HWIATTR_TIMER_T_INITIALIZER;
		const struct hwi_inputclk clksrc_kick = {.clk = MC1892VM14_WDT_CLOCK_FREQ, .div = 1};
		HWIATTR_TIMER_SET_NUM_CLK(&attr, 1);
		HWIATTR_TIMER_SET_LOCATION(&attr, MC1892VM14_WDT_BASEADDR, MC1892VM14_WDT_SIZE, 0,
																	hwi_find_as(MC1892VM14_WDT_BASEADDR, 1));
		hwi_off = hwidev_add_timer(MC1892VM14_HWI_WDT, &attr,  HWI_NULL_OFF);
		ASSERT(hwi_off != HWI_NULL_OFF);
		hwitag_set_inputclk(hwi_off, 0, (struct hwi_inputclk *)&clksrc_kick);
	}

	/* add DMA controller */
	{
		unsigned hwi_off;
		hwiattr_dma_t attr = HWIATTR_DMA_T_INITIALIZER;
		HWIATTR_DMA_SET_NUM_IRQ(&attr, 1);

		/* create DMA controller */
		HWIATTR_USB_SET_LOCATION(&attr, MC1892VM14_SDMA_BASE_ADDR, MC1892VM14_SDMA_REG_SIZE, 0,
																	hwi_find_as(MC1892VM14_SDMA_BASE_ADDR, 1));
		hwi_off = hwidev_add_dma(MC1892VM14_HWI_DMA, &attr, hwi_bus_internal);
		ASSERT(hwi_find_unit(hwi_off) == 0);
		hwitag_set_avail_ivec(hwi_off, 0, MC1892VM14_IRQ_SDMA0);
	}

	/* add SPLL clock info  */
	{
		unsigned hwi_off;
		unsigned val;

		hwiattr_timer_t attr = HWIATTR_TIMER_T_INITIALIZER;
		struct hwi_inputclk clksrc_spll;
		
		HWIATTR_TIMER_SET_NUM_CLK(&attr, 1);
		clksrc_spll.clk = val = mc1892vm14_get_spll_clk();
		clksrc_spll.div = 1;
		hwi_off = hwidev_add_timer(MC1892VM14_HWI_SPLL, &attr,  HWI_NULL_OFF);
		ASSERT(hwi_off != HWI_NULL_OFF);
		hwitag_set_inputclk(hwi_off, 0, (struct hwi_inputclk *)&clksrc_spll);

		clksrc_spll.clk = val = mc1892vm14_get_l1_clk(val);
		clksrc_spll.div = 1;
		hwi_off = hwidev_add_timer(MC1892VM14_HWI_SPLL, &attr,  hwi_off);
		ASSERT(hwi_off != HWI_NULL_OFF);
		hwitag_set_inputclk(hwi_off, 0, (struct hwi_inputclk *)&clksrc_spll);

		clksrc_spll.clk = mc1892vm14_get_l2_clk(val);
		clksrc_spll.div = 1;
		hwi_off = hwidev_add_timer(MC1892VM14_HWI_SPLL, &attr,  hwi_off);
		ASSERT(hwi_off != HWI_NULL_OFF);
		hwitag_set_inputclk(hwi_off, 0, (struct hwi_inputclk *)&clksrc_spll);
	}
	
	/* add RTC */
//	hwi_add_device(HWI_ITEM_BUS_UNKNOWN, HWI_ITEM_DEVCLASS_RTC, "NONE", 0);

	/* add an ethernet device */
    {
		unsigned hwi_off;
		const uint8_t mac[6] = {'C', 0x14, 0x00, 'A', 0x03, 0x00}; // use mac as GPIO info

		hwiattr_enet_t attr = HWIATTR_ENET_T_INITIALIZER;

		HWIATTR_ENET_SET_MAC(&attr, mac);
		HWIATTR_ENET_SET_NUM_PHY(&attr, 1);
		HWIATTR_ENET_SET_DLL(&attr, "1892vm14-gemac");
		hwi_off = hwidev_add_enet(VM14_HWI_ENET, &attr, hwi_bus_internal);
		ASSERT(hwi_off != HWI_NULL_OFF);
		hwitag_set_phyaddr(hwi_off, 0, 1);
		hwi_add_synonym(hwi_find_device(VM14_HWI_ENET, 0), "arasan-gemac");
	}
	/* TODO Add peripherals */
}

