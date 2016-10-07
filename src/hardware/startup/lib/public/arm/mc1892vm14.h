/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems.
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
 * Elvees 1892VM14 processor with Dual ARMv7 Cortex-A9 core
 */

#ifndef	__ARM_MC1892VM14_H_INCLUDED
#define	__ARM_MC1892VM14_H_INCLUDED

#include <arm/mc1892vm14_irq.h>


/* -------------------------------------------------------------------------
 * HWI Devices
 * -------------------------------------------------------------------------
 */
#define MC1892VM14_HWI_UART				"uart"
#define MC1892VM14_HWI_WDT				"wdt"
#define MC1892VM14_HWI_DMA				"dma"

/* -------------------------------------------------------------------------
 * SDRAM
 * -------------------------------------------------------------------------
 */
#define MC1892VM14_DDRMC0_BASE			0x40000000
#define MC1892VM14_DDRMC1_BASE			0xA0000000


/* -------------------------------------
 * SDMA
 * -------------------------------------
 */

#define MC1892VM14_SDMA_BASE_ADDR		0x37220000
#define MC1892VM14_SDMA_REG_SIZE		0x1000


/* -------------------------------------------------------------------------
 * ARM / SMP
 * -------------------------------------------------------------------------
 */

#define MC1892VM14_SPRAM_START			0x20000000
#define MC1892VM14_PL310_BASE			0x39004000



/* -------------------------------------------------------------------------
 * CPU Private timers
 * -------------------------------------------------------------------------
 */

#define MC1892VM14_PRIVATE_TIMER_BASE				0x39000600
#define MC1892VM14_PRIVATE_TIMER_SIZE				0x40

/* Timer Load register */
#define MC1892VM14_PRIVATE_TIMER_LOAD				0x00

/* Timer Counter register */
#define MC1892VM14_PRIVATE_TIMER_COUNTER			0x04

/* Timer Control register */
#define MC1892VM14_PRIVATE_TIMER_CTRL				0x08
#define MC1892VM14_PRIVATE_TIMER_CTRL_EN			(1 << 0)
#define MC1892VM14_PRIVATE_TIMER_CTRL_AUTO_RELOAD	(1 << 1)
#define MC1892VM14_PRIVATE_TIMER_CTRL_IRQ			(1 << 2)
#define MC1892VM14_PRIVATE_TIMER_CTRL_PRESCALER_MSK	(0xFF << 8)

/* Timer Interrupt Status register */
#define MC1892VM14_PRIVATE_TIMER_INT_STS 			0x0C

#define MC1892VM14_PRIVATE_WDT_LOAD					0x20
#define MC1892VM14_PRIVATE_WDT_COUNTER				0x24
#define MC1892VM14_PRIVATE_WDT_CTRL					0x28
#define MC1892VM14_PRIVATE_WDT_STS					0x2C
#define MC1892VM14_PRIVATE_WDT_RESET				0x30
#define MC1892VM14_PRIVATE_WDT_DISABLE				0x34

/* -------------------------------------
 * Global timer
 * -------------------------------------
 */
#define MC1892VM14_GLOBAL_TIMER_COUNTER_BASE			0x39000200
/* Size of the register area (used for mapping) */
#define MC1892VM14_GLOBAL_TIMER_COUNTER_BASE_MAP_SIZE	0x20

#define MC1892VM14_GLOBAL_TIMER_COUNTER_REG0			0x00
#define MC1892VM14_GLOBAL_TIMER_COUNTER_REG1			0x04
#define MC1892VM14_GLOBAL_TIMER_CONTROL_REG				0x08
#define MC1892VM14_GLOBAL_TIMER_INT_STS					0x08
#define MC1892VM14_GLOBAL_TIMER_CMP_REG0				0x10
#define MC1892VM14_GLOBAL_TIMER_CMP_REG1				0x14

#define MC1892VM14_GLOBAL_TIMER_CONTROL_TIMER_EN		(1 << 0)
#define MC1892VM14_GLOBAL_TIMER_CONTROL_CMP_EN			(1 << 1)
#define MC1892VM14_GLOBAL_TIMER_CONTROL_IRQ				(1 << 2)
#define MC1892VM14_GLOBAL_TIMER_CONTROL_AE				(1 << 3)
#define MC1892VM14_GLOBAL_TIMER_CONTROL_PRESCALER_MSK	(0xFF << 8)

/*
 * -------------------------------------------------------------------------
 * CMCTR - synchronization controller
 * -------------------------------------------------------------------------
 */
#define MC1892VM14_CMCTR_BASE			0x38094000
#define MC1892VM14_CMCTR_SIZE			0x200
#define MC1892VM14_CMCTR_DIV_MPU_REG	0x4
#define MC1892VM14_CMCTR_DIV_SYS0_REG	0x40
#define MC1892VM14_CMCTR_DIV_SYS1_REG	0x44
#define MC1892VM14_CMCTR_SEL_APLL_REG	0x100
#define MC1892VM14_CMCTR_SEL_SPLL_REG	0x10C

/*
 * -------------------------------------------------------------------------
 * PMCTR - power managment controller
 * -------------------------------------------------------------------------
 */
#define MC1892VM14_PMCTR_BASE			0x38095000
#define MC1892VM14_PMCTR_SIZE			0x100
#define MC1892VM14_PMCTR_SW_RST_REG		0x40


/* -------------------------------------------------------------------------
 * Serial ports
 * -------------------------------------------------------------------------
 */

/* UART base addresses */
#define MC1892VM14_UART_CLK_FREQ	144000000U

#define	MC1892VM14_UART0_BASE		0x38028000
#define	MC1892VM14_UART1_BASE		0x38029000

#define	MC1892VM14_UART_IO_SIZE		0x1000

/* UART registers */
#define MC1892VM14_UART_THR			0x00  /* Transmitter */
#define MC1892VM14_UART_RBR			0x00  /* Receiver */
#define MC1892VM14_UART_DLL			0x00  /* DLL */
#define MC1892VM14_UART_DHL			0x04  /* DHL */
#define MC1892VM14_UART_LCR			0x0C  /* UART Control register */
#define MC1892VM14_UART_LSR			0x14  /* Channel status register */
#define MC1892VM14_UART_USR			0x7C  /* UART status register */

/* UART Status Regiser */
#define MC1892VM14_UART_LSR_RDR		(1 << 0)
#define MC1892VM14_UART_LSR_BI		(1 << 4)
#define MC1892VM14_UART_LSR_THRE	(1 << 5)
#define MC1892VM14_UART_LSR_TEMPT	(1 << 6)

/* UART Control register */
#define MC1892VM14_UART_LCR_DLS_5	0x00		/* 5-bit characters */
#define MC1892VM14_UART_LCR_DLS_6	0x01		/* 6-bit characters */
#define MC1892VM14_UART_LCR_DLS_7	0x02		/* 7-bit characters */
#define MC1892VM14_UART_LCR_DLS_8	0x03		/* 8-bit characters */
#define MC1892VM14_UART_LCR_STB		(1 << 2)	/* 2 stop bits */
#define MC1892VM14_UART_LCR_PEN		(1 << 3)	/* Parity enable */
#define MC1892VM14_UART_LCR_EPS		(1 << 4)	/* Even parity */
#define MC1892VM14_UART_LCR_STP		(1 << 5)	/* Sticky parity */
#define MC1892VM14_UART_LCR_BC		(1 << 6)	/* Set break */
#define MC1892VM14_UART_LCR_DLAB	(1 << 7)	/* Divisor latch access */

/* -------------------------------------------------------------------------
 * Watchdog
 * -------------------------------------------------------------------------
 */
#define MC1892VM14_WDT_CLOCK_FREQ		144000000U
#define MC1892VM14_WDT_BASEADDR			0x38031000
#define MC1892VM14_WDT_SIZE				0x20

#define MC1892VM14_WDT_CR				0x00		/* Control Register */
#define MC1892VM14_WDT_TORR				0x04		/* Init Timeout Register */
#define MC1892VM14_WDT_CCVR				0x08		/* Current Counter Value Register */
#define MC1892VM14_WDT_CRR 				0x0C		/* Counter Restart Register */
#define MC1892VM14_WDT_STAT				0x10		/* Interrupt Status Register */
#define MC1892VM14_WDT_EOI				0x14		/* Interrupt Reset Register */

#define MC1892VM14_WDT_CR_WDEN			0x1			/* enable WDT */
#define MC1892VM14_WDT_CR_RMOD_IRQ		0x2			/* generate IRQ not reset */
#define MC1892VM14_WDT_CR_RPL_2			(0 << 2) 	/* 2 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_4			(1 << 2) 	/* 4 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_8			(2 << 2) 	/* 8 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_16		(3 << 2) 	/* 16 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_32		(4 << 2) 	/* 32 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_64		(5 << 2) 	/* 64 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_128		(6 << 2) 	/* 128 SPLLCLK reset pulse */
#define MC1892VM14_WDT_CR_RPL_256		(7 << 2) 	/* 256 SPLLCLK reset pulse */

#define MC1892VM14_WDT_TOPINT_SHIFT		4			/* TORR TOP_INIT field shift */

#define MC1892VM14_WDT_RESTART_KEY_VAL	0x76		/* Counter Restart Valid Key */

#endif // __ARM_MC1892VM14_H_INCLUDED

