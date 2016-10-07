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
 * Elvees Salute EL24D1 board with Cortex-A9 MPCore
 */

#include "startup.h"
#include <time.h>
#include "board.h"

extern struct callout_rtn reboot_mc1892vm14;

const struct callout_slot callouts[] = {
	{ CALLOUT_SLOT( reboot, _mc1892vm14) },
};

const struct debug_device debug_devices[] = {
	{ 	"ser1892vm14",
		/* Debug on UART 0 */
		{"0x38028000^0.115200.144000000",
		},
		init_ser1892vm14,
		put_ser1892vm14,
		{	&display_char_mc1892vm14,
			&poll_key_mc1892vm14,
			&break_detect_mc1892vm14,
		}
	},
};

/*
 * main()
 *	Startup program executing out of RAM
 *
 * 1. It gathers information about the system and places it in a structure
 *    called the system page. The kernel references this structure to
 *    determine everything it needs to know about the system. This structure
 *    is also available to user programs (read only if protection is on)
 *    via _syspage->.
 *
 * 2. It (optionally) turns on the MMU and starts the next program
 *    in the image file system.
 */
int
main(int argc, char **argv, char **envv)
{
	int		opt;
	

	add_callout_array(callouts, sizeof(callouts));

	// common options that should be avoided are:
	// "AD:F:f:I:i:K:M:N:o:P:R:S:Tvr:j:Z"
	while ((opt = getopt(argc, argv, COMMON_OPTIONS_STRING "W")) != -1) {
		switch (opt) {
			case 'W':
				/* Enable WDT */
				mc1892vm14_wdg_reload();
				mc1892vm14_wdg_enable();
				break;
			default:
				handle_common_option(opt);
				break;
		}
	}

	/*
	 * Initialize debugging output
	 */
	select_debug(debug_devices, sizeof(debug_devices));
	
	kprintf( "KPDA Neutrino startup for the Salute EL24D1 board with Cortex-A9 MPCore\n" );

	/*
	 * Collect information on all free RAM in the system
	 */
	mc1892vm14_init_raminfo(EL24D1_SDRAM_SIZE);
	
	/*
	 * Get CPU frequency
	 */
	if (cpu_freq == 0)
		cpu_freq = mc1892vm14_get_cpu_clk();

	/*
	 * Remove RAM used by modules in the image
	 */
	alloc_ram(shdr->ram_paddr, shdr->ram_size, 1);

	/*
	  * Initialize SMP
	  */
	init_smp();

	/*
	  * Initialize MMU
	  */
	if (shdr->flags1 & STARTUP_HDR_FLAGS1_VIRTUAL)
		init_mmu();

	/* Initialize the Interrupts related Information */
	init_intrinfo();

	/* Initialize the Timer related information */
	mc1892vm14_init_qtime();
	

	/* Init L2 Cache Controller */
	init_cacheattr();
	
	/* Initialize the CPU related information */
	init_cpuinfo();

	/* Initialize the Hwinfo section of the Syspage */
 	init_hwinfo();

	add_typed_string(_CS_MACHINE, "el24d1");

	/*
	 * Load bootstrap executables in the image file system and Initialise
	 * various syspage pointers. This must be the _last_ initialisation done
	 * before transferring control to the next program.
	 */
	init_system_private();

	/*
	 * This is handy for debugging a new version of the startup program.
	 * Commenting this line out will save a great deal of code.
	 */
	print_syspage();
	return 0;
}
