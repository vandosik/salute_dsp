/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. 
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


#ifdef DEFN
	#define EXT
	#define INIT1(a)				= { a }
#else
	#define EXT extern
	#define INIT1(a)
#endif

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <sys/iomsg.h>
#include <atomic.h>
#include <hw/inout.h>
#include <hw/8250.h>
#include <sys/io-char.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <variant.h>

#if defined(VARIANT_pa6t) || defined(VARIANT_tegra2)
	// enable workarounds for bugs in the UARTS of early PA6T-1682's
	/* Note: The implementation fo the 8250 on the nVIDIA Tegra2
             behave the same way as the workaround for the PA6T.
             We will reuse the same code unless there is some
             difference between both implementation.
    */
	#define PA6T_WORKAROUND
#endif

#if defined(VARIANT_mpc8540)
	#define MPC850_WORKAROUND
#endif

#if defined(VARIANT_jace5)
	#define TL16C752B_WORKAROUND
#endif

#if defined(VARIANT_dw)
#define SER_REG_TOTAL		0x20
/*Additional DW UART Status Register*/
#define DW_UART_USR			0x1f
/*DW BUSY Interrupt Status */
#define II_BUSY				0x7
#else
#define SER_REG_TOTAL		REG_TOTAL
#endif

#ifndef DEV_8250			/* Can be defined in variant.h to override default */
typedef struct dev_8250 {
	TTYDEV			tty;
	struct dev_8250	*next;
	unsigned		intr;
	unsigned		clk;
	unsigned		div;
	unsigned char	rx_fifo; /* rx fifo size */
	unsigned char	tx_fifo; /* tx fifo size */
	unsigned char   fcr;     /* FCR is write-only register, so keep local copy for read-modify-write */
#if defined(PA6T_WORKAROUND)	
	unsigned char   irr_fiddle;			/* for PA6T-1682 workaround */
	unsigned char	tx_empty_disable;	/* for PA6T-1682 workaround */
#endif
	uintptr_t		port[SER_REG_TOTAL];
    uint32_t        (*read_8250)(uintptr_t port);                   /* HW access function pointers */
    void            (*write_8250)(uintptr_t port, uint32_t val);
} DEV_8250;
#endif

struct dev_list {
	struct dev_list	*next;
	DEV_8250		*device;
	int				iid;
};

EXT TTYCTRL						ttyctrl;
EXT struct dev_list				*devices;

#define MAX_DEVICES  16
#define FIFO_SIZE 16

#include "proto.h"

#ifndef IE_SET_ALL
    #define IE_SET_ALL ( IE_RX | IE_TX | IE_LS | IE_MS )
#endif

#ifndef IE_CLR_ALL
    #define IE_CLR_ALL 0x0
#endif



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.5.0/trunk/hardware/devc/ser8250/externs.h $ $Rev: 746538 $")
#endif
