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

// The watchdog timeout value should be specified in board.h.  Set default value to 10 seconds.
#if !defined(WDT_TIMEOUT)
#define WDT_TIMEOUT 10
#endif

/*
 * There are 16 possible timeout values in 0..15 where the number of
 * cycles is 2 ^ (16 + i) and the watchdog counts down.
 */
#define WDT_TOP_IN_SEC(value)	(((1U << (16 + (value))) / MC1892VM14_WDT_CLOCK_FREQ))
#define WDT_MAX_TOP				0xF

/* Enable watch dog */
void mc1892vm14_wdg_enable(void)
{
	// Enable WDT, hw reset, 8 SPLLCLK length of reset pulse
	out32(MC1892VM14_WDT_BASEADDR + MC1892VM14_WDT_CR, MC1892VM14_WDT_CR_WDEN | MC1892VM14_WDT_CR_RPL_4);
}


/* Re-load the value of watch-dog timer */
void mc1892vm14_wdg_reload(void)
{
	int 		i;
	uint32_t 	top_val = WDT_MAX_TOP;
	
	// set timeout value
	for (i = 0; i <= WDT_MAX_TOP; i++) {
		if (WDT_TOP_IN_SEC(i) >= WDT_TIMEOUT) {
			top_val = i;
			break;
		}
	}
	// load value into control register
	out32(MC1892VM14_WDT_BASEADDR + MC1892VM14_WDT_TORR, top_val | top_val << MC1892VM14_WDT_TOPINT_SHIFT);
	
	// restart the counter
	out32(MC1892VM14_WDT_BASEADDR + MC1892VM14_WDT_CRR, MC1892VM14_WDT_RESTART_KEY_VAL);
}
