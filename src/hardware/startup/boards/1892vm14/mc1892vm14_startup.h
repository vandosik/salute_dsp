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


#ifndef __MC1892VM14_STARTUP_H
#define __MC1892VM14_STARTUP_H

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

void mc1892vm14_init_raminfo(char *);
void mc1892vm14_set_cpu_clk( uint32_t );
void mc1892vm14_set_cpu_clk_from_fdt(void);
uint32_t mc1892vm14_get_cpu_clk(void);
void mc1892vm14_set_spll_clk( uint32_t );
void mc1892vm14_set_spll_clk_from_fdt(void);
uint32_t mc1892vm14_get_spll_clk(void);
uint32_t mc1892vm14_get_l1_clk(unsigned spll_clk);
uint32_t mc1892vm14_get_l2_clk(unsigned l1_clk);
void mc1892vm14_init_qtime(void);
void mc1892vm14_wdg_enable(void);
void mc1892vm14_wdg_reload(void);
void mc1892vm14_hwinit(void);

#endif  // __MC1892VM14_STARTUP_H
