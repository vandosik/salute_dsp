/*
 * $QNXLicenseC: 
 * Copyright 2012 QNX Software Systems.  
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
 * 1892VM14 private timer support.
 */

#include "startup.h"
#include "board.h"
#include <arm/mc1892vm14.h>

#define TIMER_INTERRUPT				29


extern struct callout_rtn timer_load_mc1892vm14;
extern struct callout_rtn timer_value_mc1892vm14;
extern struct callout_rtn timer_reload_mc1892vm14;

extern struct callout_rtn clock_cycles_mc1892vm14;

extern void arm_add_clock_cycles(struct callout_rtn *callout, int incr_bit);

static uintptr_t timer_base = NULL;

static const struct callout_slot timer_callouts[] = {
		{CALLOUT_SLOT(timer_load, _mc1892vm14)},
		{CALLOUT_SLOT(timer_value, _mc1892vm14)},
		{CALLOUT_SLOT(timer_reload, _mc1892vm14)},
	};

/*
 * These functions are used to calibrate the inline delay loop functions.
 * They aren't used after the kernel comes up.
 */
static uint32_t get_timer(void)
{
	return in32(timer_base + MC1892VM14_PRIVATE_TIMER_COUNTER);
}

static unsigned timer_start_mc1892vm14(void)
{
	return (uint32_t) get_timer();
}

static unsigned timer_diff_mc1892vm14(unsigned start)
{
	unsigned now = (uint32_t) get_timer();
	return (now - start);
}

void mc1892vm14_init_qtime(void)
{
	struct qtime_entry *qtime = alloc_qtime();
	uint32_t ctrl_reg;
	uintptr_t cmctr_base;
	uint32_t mpuclk_div;
	

	/*
	 * Map the timer registers.
	 * We are using CPU private timer counter which is
	 * part of MPU Core registers
	 */
	timer_base = startup_io_map(MC1892VM14_PRIVATE_TIMER_SIZE,
			MC1892VM14_PRIVATE_TIMER_BASE);

	/* Setup the Load register */
	out32(timer_base + MC1892VM14_PRIVATE_TIMER_LOAD, 0xFFFFFFFF);

	/*
	 * Select clock input source:
	 * - Enable the timer
	 * - Enable the auto reload
	 * - Enable IRQ
	 * - Prescaler is not set
	 * @TODO: Calculate the best prescaler value to reduce
	 * the round errors
	 */
	ctrl_reg = MC1892VM14_PRIVATE_TIMER_CTRL_EN
			| MC1892VM14_PRIVATE_TIMER_CTRL_AUTO_RELOAD
			| MC1892VM14_PRIVATE_TIMER_CTRL_IRQ;
	out32(timer_base + MC1892VM14_PRIVATE_TIMER_CTRL, ctrl_reg);

	timer_start = timer_start_mc1892vm14;
	timer_diff = timer_diff_mc1892vm14;

	if (timer_freq == 0) {
		/*
		 * Map the CMCTR registers to read MPU DIV value for private timer
		 */
		cmctr_base = startup_io_map(MC1892VM14_PRIVATE_TIMER_SIZE, MC1892VM14_CMCTR_BASE);	
		mpuclk_div = in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_DIV_MPU_REG);
		startup_io_unmap(cmctr_base);
	
		timer_freq = cpu_freq >> mpuclk_div;
	}
	
	invert_timer_freq(qtime, timer_freq);

	
	// count register increments at full the clock rate
	if (cycles_freq == 0)
		cycles_freq = (uint64_t)timer_freq;
	
	qtime->cycles_per_sec = (uint64_t)cycles_freq;
	
	qtime->intr = TIMER_INTERRUPT;
	
	/*
	 * Generic timer registers are banked per-cpu so ensure that the
	 * system clock is only operated on via cpu0
	 */
	qtime->flags |= QTIME_FLAG_TIMER_ON_CPU0;

	add_callout_array(timer_callouts, sizeof(timer_callouts));

	timer_base = startup_io_map(MC1892VM14_GLOBAL_TIMER_COUNTER_BASE_MAP_SIZE,
			 MC1892VM14_GLOBAL_TIMER_COUNTER_BASE);
	ctrl_reg =  MC1892VM14_GLOBAL_TIMER_CONTROL_TIMER_EN;
	out32(timer_base +  MC1892VM14_GLOBAL_TIMER_CONTROL_REG, ctrl_reg);

	/*
	 * Add clock_cycles callout to directly access 64-bit counter
	 */
	arm_add_clock_cycles(&clock_cycles_mc1892vm14, 0);
}
