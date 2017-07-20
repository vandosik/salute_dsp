/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
 *
 * Copyright 2016, CBD BC, http://www.kpda.ru  
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

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include "vm14timer.h"

static uint32_t *timer_addr;

static const vm14_timer_reg_t	vm14_timer_reg[VM14_TIMER_MAX] = {
	{MC1892VM14_TIMER0_LC, MC1892VM14_TIMER0_LC2, MC1892VM14_TIMER0_CV, MC1892VM14_TIMER0_CNTR, MC1892VM14_TIMER0_EOI, MC1892VM14_TIMER0_IS, MC1892VM14_IRQ_TIMER0},
	{MC1892VM14_TIMER1_LC, MC1892VM14_TIMER1_LC2, MC1892VM14_TIMER1_CV, MC1892VM14_TIMER1_CNTR, MC1892VM14_TIMER1_EOI, MC1892VM14_TIMER1_IS, MC1892VM14_IRQ_TIMER1},
	{MC1892VM14_TIMER2_LC, MC1892VM14_TIMERN_LC2, MC1892VM14_TIMER2_CV, MC1892VM14_TIMER2_CNTR, MC1892VM14_TIMER2_EOI, MC1892VM14_TIMER2_IS, MC1892VM14_IRQ_TIMER2},
	{MC1892VM14_TIMER3_LC, MC1892VM14_TIMERN_LC2, MC1892VM14_TIMER3_CV, MC1892VM14_TIMER3_CNTR, MC1892VM14_TIMER3_EOI, MC1892VM14_TIMER3_IS, MC1892VM14_IRQ_TIMER3},
	{MC1892VM14_TIMER4_LC, MC1892VM14_TIMERN_LC2, MC1892VM14_TIMER4_CV, MC1892VM14_TIMER4_CNTR, MC1892VM14_TIMER4_EOI, MC1892VM14_TIMER4_IS, MC1892VM14_IRQ_TIMER4},
	{MC1892VM14_TIMER5_LC, MC1892VM14_TIMERN_LC2, MC1892VM14_TIMER5_CV, MC1892VM14_TIMER5_CNTR, MC1892VM14_TIMER5_EOI, MC1892VM14_TIMER5_IS, MC1892VM14_IRQ_TIMER5},
	{MC1892VM14_TIMER6_LC, MC1892VM14_TIMERN_LC2, MC1892VM14_TIMER6_CV, MC1892VM14_TIMER6_CNTR, MC1892VM14_TIMER6_EOI, MC1892VM14_TIMER6_IS, MC1892VM14_IRQ_TIMER6},
	{MC1892VM14_TIMER7_LC, MC1892VM14_TIMERN_LC2, MC1892VM14_TIMER7_CV, MC1892VM14_TIMER7_CNTR, MC1892VM14_TIMER7_EOI, MC1892VM14_TIMER7_IS, MC1892VM14_IRQ_TIMER7}
};

static uint32_t timer_reg_read(uint32_t off)
{
	return *(uint32_t *) ((uint8_t *) timer_addr + off);
}

static void timer_reg_write(uint32_t off, uint32_t val)
{
	*(uint32_t *) ((uint8_t *) timer_addr + off) = val;
}

int vm14_timer_setup(void)
{
	if ( ThreadCtl(_NTO_TCTL_IO, 0) == -1 )
		return -1;

	timer_addr = (uint32_t *) mmap( NULL, MC1892VM14_TIMERS_SIZE,
				PROT_NOCACHE | PROT_READ | PROT_WRITE,
				MAP_PHYS | MAP_SHARED,
				NOFD, MC1892VM14_TIMERS_BASE );

	if ( timer_addr == MAP_FAILED )
		return -1;

	return 0;
}

int vm14_timer_destroy(void)
{
	if ( munmap(timer_addr, MC1892VM14_TIMERS_SIZE) == -1 )
		return -1;

	return 0;
}

int vm14_timer_valid(vm14_timer_id_t id)
{
	if ((id >= 0) && (id < VM14_TIMER_MAX))
		return 0;

	return -1;
}

int vm14_timer_start(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val |= MC1892VM14_TIMERS_ENABLE;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_stop(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val &= ~MC1892VM14_TIMERS_ENABLE;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_is_work(vm14_timer_id_t id)
{
	if ( vm14_timer_valid(id) )
		return -1;

	return timer_reg_read(vm14_timer_reg[id].cntr) & MC1892VM14_TIMERS_ENABLE;
}

int vm14_timer_set_free_run_mode(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val |= MC1892VM14_TIMERS_MODE;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_set_user_mode(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val &= ~MC1892VM14_TIMERS_MODE;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_get_mode(vm14_timer_id_t id)
{
	if ( vm14_timer_valid(id) )
		return -1;

	return timer_reg_read(vm14_timer_reg[id].cntr) & MC1892VM14_TIMERS_MODE;
}

int vm14_timer_irq_mask(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val |= MC1892VM14_TIMERS_IRQ_MASK;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_irq_unmask(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val &= ~MC1892VM14_TIMERS_IRQ_MASK;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_irq_mask_status(vm14_timer_id_t id)
{
	if ( vm14_timer_valid(id) )
		return -1;

	return timer_reg_read(vm14_timer_reg[id].cntr) & MC1892VM14_TIMERS_IRQ_MASK;
}

int vm14_timer_enable_toggle(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	if ( vm14_timer_reg[id].count2 == 0 )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val |= MC1892VM14_TIMERS_TPWM;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_disable_toggle(vm14_timer_id_t id)
{
	uint32_t tmp_val;

	if ( vm14_timer_valid(id) )
		return -1;

	if ( vm14_timer_reg[id].count2 == 0 )
		return -1;

	tmp_val = timer_reg_read(vm14_timer_reg[id].cntr);
	tmp_val &= ~MC1892VM14_TIMERS_TPWM;
	timer_reg_write(vm14_timer_reg[id].cntr, tmp_val);

	return 0;
}

int vm14_timer_toggle_status(vm14_timer_id_t id, uint32_t *val)
{
	if ( vm14_timer_valid(id) )
		return -1;

	if ( vm14_timer_reg[id].count2 == 0 )
		return -1;

	return timer_reg_read(vm14_timer_reg[id].cntr) & MC1892VM14_TIMERS_TPWM;
}

int vm14_timer_get_cur_val(vm14_timer_id_t id, uint32_t *val)
{
	if ( vm14_timer_valid(id) )
		return -1;

	*val = timer_reg_read(vm14_timer_reg[id].cur_val);

	return 0;
}

int vm14_timer_set_count(vm14_timer_id_t id, uint32_t val)
{
	if ( vm14_timer_valid(id) )
		return -1;

	timer_reg_write(vm14_timer_reg[id].count, val);

	return 0;
}

int vm14_timer_get_count(vm14_timer_id_t id, uint32_t *val)
{
	if ( vm14_timer_valid(id) )
		return -1;

	*val = timer_reg_read(vm14_timer_reg[id].count);

	return 0;
}

int vm14_timer_set_count2(vm14_timer_id_t id, uint32_t val)
{
	if ( vm14_timer_valid(id) )
		return -1;

	timer_reg_write(vm14_timer_reg[id].count2, val);

	return 0;
}

int vm14_timer_get_count2(vm14_timer_id_t id, uint32_t *val)
{
	if ( vm14_timer_valid(id) )
		return -1;

	*val = timer_reg_read(vm14_timer_reg[id].count2);

	return 0;
}

int vm14_timer_get_irqnum(vm14_timer_id_t id)
{
	if ( vm14_timer_valid(id) )
		return -1;
	return vm14_timer_reg[id].irq;
}

int vm14_timer_reset_irq(vm14_timer_id_t id)
{
	if ( vm14_timer_valid(id) )
		return -1;

	timer_reg_read(vm14_timer_reg[id].eoi);

	return 0;
}

int vm14_timer_reset_all_irq(void)
{
	timer_reg_read(MC1892VM14_TIMERS_EOI);
	return 0;
}

int vm14_timer_irq_status(vm14_timer_id_t id)
{
	if ( vm14_timer_valid(id) )
		return -1;

	return timer_reg_read(vm14_timer_reg[id].status);;
}

int vm14_timer_all_irq_status(void)
{
	return timer_reg_read(MC1892VM14_TIMERS_IS);
}

int vm14_timer_all_raw_irq_status(void)
{
	return timer_reg_read(MC1892VM14_TIMERS_IRS);
}
