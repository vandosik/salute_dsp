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

#ifndef VM14_TIMERS_H
#define VM14_TIMERS_H

#include <stdint.h>
#include <arm/mc1892vm14.h>

#define VM14_TIMER_MAX			8

#define VM14_TIMERS_FREE_RUN_MODE	0x02	// Timer free-running mode
#define VM14_TIMERS_USER_MODE		0x00	// Timer user mode
#define VM14_TIMERS_TOGGLE_ENABLE	0x08	// Timer pulse-width modulation mode
#define VM14_TIMERS_TOGGLE_DISABLE	0x00	// Timer pulse-width modulation mode


typedef enum vm14_timer_id
{
	TIMER0,
	TIMER1,
	TIMER2,
	TIMER3,
	TIMER4,
	TIMER5,
	TIMER6,
	TIMER7
} vm14_timer_id_t;

typedef struct vm14_timer_reg_struct
{
	unsigned	count;
	unsigned	count2;
	unsigned	cur_val;
	unsigned	cntr;
	unsigned	eoi;
	unsigned	status;
	unsigned	irq;
} vm14_timer_reg_t;

#ifdef __cplusplus
extern "C" {
#endif

int vm14_timer_setup(void);
int vm14_timer_destroy(void);

int vm14_timer_valid(vm14_timer_id_t id);

int vm14_timer_start(vm14_timer_id_t id);
int vm14_timer_stop(vm14_timer_id_t id);
int vm14_timer_running(vm14_timer_id_t id);

int vm14_timer_set_free_run_mode(vm14_timer_id_t id);
int vm14_timer_set_user_mode(vm14_timer_id_t id);
int vm14_timer_get_mode(vm14_timer_id_t id);

int vm14_timer_mask(vm14_timer_id_t id);
int vm14_timer_unmask(vm14_timer_id_t id);
int vm14_timer_get_mask(vm14_timer_id_t id);

int vm14_timer_pwm_enable(vm14_timer_id_t id);
int vm14_timer_pwm_disable(vm14_timer_id_t id);
int vm14_timer_pwm_status(vm14_timer_id_t id, uint32_t *val);

int vm14_timer_get_cur_val(vm14_timer_id_t id, uint32_t *val);
int vm14_timer_set_count(vm14_timer_id_t id, uint32_t val);
int vm14_timer_get_count(vm14_timer_id_t id, uint32_t *val);
int vm14_timer_set_count2(vm14_timer_id_t id, uint32_t val);
int vm14_timer_get_count2(vm14_timer_id_t id, uint32_t *val);

int vm14_timer_get_irqnum(vm14_timer_id_t id);
int vm14_timer_reset_irq(vm14_timer_id_t id);
int vm14_timer_reset_all_irq(void);
int vm14_timer_irq_status(vm14_timer_id_t id);
int vm14_timer_all_irq_status(void);
int vm14_timer_all_raw_irq_status(void);

#ifdef __cplusplus
}
#endif

#endif // VM14S_TIMER_H
