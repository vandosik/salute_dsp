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
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include "vm14timer.h"

#define LCC_MAX 100

pthread_t time_tid;

struct data_timer
{
	int irqnum;
	vm14_timer_id_t tid;
	int mode;
	unsigned int max_interr;
	unsigned int sleep_time;
	uint32_t irq_period;
	uint64_t lcc[LCC_MAX];
};

struct data_timer timer_info;


void *vm14_event_handler(void *data)
{
	int iid;
	uint64_t lcc_prev;
	uint64_t lcc;
	struct sigevent event;

	ThreadCtl(_NTO_TCTL_IO, 0);
	vm14_timer_reset_irq(timer_info.tid);
	vm14_timer_mask(timer_info.tid);

	SIGEV_INTR_INIT(&event);
	iid = InterruptAttachEvent(timer_info.irqnum, &event, _NTO_INTR_FLAGS_TRK_MSK);
	if ( iid == -1 )
	{
		fprintf(stderr, "InterruptAttachEvent(): error\n");
		return (void *) -1;
	}

	vm14_timer_set_count(timer_info.tid, timer_info.irq_period);

	if (timer_info.mode == VM14_TIMERS_USER_MODE )
		vm14_timer_set_user_mode(timer_info.tid);
	else
		vm14_timer_set_free_run_mode(timer_info.tid);

	vm14_timer_unmask(timer_info.tid);
	vm14_timer_start(timer_info.tid);

	lcc_prev = ClockCycles();

	while ( 1 )
	{
		while ( timer_info.max_interr < LCC_MAX )
		{
			if ( InterruptWait(0, NULL) == -1 ) {
				fprintf(stderr, "InterruptWait(): error\n");
				return (void *) -1;
			}
			lcc = ClockCycles();
			timer_info.lcc[timer_info.max_interr] = (lcc - lcc_prev);
			lcc_prev = lcc;
			timer_info.max_interr++;
			vm14_timer_reset_irq(timer_info.tid);
			InterruptUnmask(timer_info.irqnum, iid);
		}
	}
}

int main(int argc, char *argv[])
{
	int i;
	int opt;
	int tmp_val;

	timer_info.tid = TIMER0;
	timer_info.mode = VM14_TIMERS_FREE_RUN_MODE;
	timer_info.sleep_time = 5;
	timer_info.max_interr = 0;
	timer_info.irq_period = 1000;

	while ( (opt = getopt(argc, argv, "n:m:d:t:")) != -1 )
	{
		switch ( opt )
		{
		case 'n':
			timer_info.tid = atoi(optarg);
			break;

		case 'm':
			timer_info.mode = atoi(optarg) ? VM14_TIMERS_USER_MODE : VM14_TIMERS_FREE_RUN_MODE;
			break;

		case 'd':
			timer_info.sleep_time = atoi(optarg);
			break;

		case 't':
			timer_info.irq_period = atoi(optarg);
			break;

		default:
			break;
		}
	}

	timer_info.irq_period = timer_info.irq_period * (vm14_timer_freq() / 1000);

	if ( vm14_timer_valid(timer_info.tid) )
	{
		fprintf(stderr, "vm14_timer_valid(): error\n");
		return -1;
	}

	for ( i = 0; i < LCC_MAX; i++ )
	{
		timer_info.lcc[i] = 0;
	}

	timer_info.irqnum = vm14_timer_get_irqnum(timer_info.tid);

	if ( vm14_timer_setup() )
	{
		fprintf(stderr, "vm14_timer_setup(): error\n");
		return -1;
	}

	tmp_val = vm14_timer_running(timer_info.tid);
	if ( tmp_val == -1  ) 
	{
		fprintf(stderr, "vm14_timer_is_work(): error\n");
		return -1;
	}
	else if ( tmp_val != 0  )
	{
		if ( vm14_timer_stop(timer_info.tid) )
		{
			fprintf(stderr, "vm14_timer_stop(): error\n");
			return -1;
		}
	}

	pthread_create(&time_tid, NULL, (void *)vm14_event_handler, NULL);
	pthread_setname_np(0, "main");
	pthread_setschedprio(time_tid, 9);

	sleep(timer_info.sleep_time);

	if ( vm14_timer_stop(timer_info.tid) )
	{
		fprintf(stderr, "vm14_timer_stop(): error\n");
		return -1;
	}

	printf("Timer id: %d\n", timer_info.tid);
	tmp_val = vm14_timer_get_mode(timer_info.tid);

	if ( tmp_val == -1  ) 
	{
		fprintf(stderr, "vm14_timer_get_mode(): error\n");
		return -1;
	}
	else if ( tmp_val == VM14_TIMERS_USER_MODE )
		printf("Timer mode: user\n");
	else
		printf("Timer mode: free run\n");
	printf("Timer sleep time: %u second\n", timer_info.sleep_time);
	printf("IRQ count: %u \n", timer_info.max_interr);

	for ( i = 0; i < timer_info.max_interr; i++ )
	{
		printf("%.3f micro second\n", (double) (timer_info.lcc[i] * 1000) / SYSPAGE_ENTRY(qtime)->cycles_per_sec);
	}

	if ( vm14_timer_destroy() )
	{
		fprintf(stderr, "vm14_timer_destroy(): error\n");
		return -1;
	}
	return 0;
}
