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

#ifndef	__ARM_MC1892VM14_IRQ_H_INCLUDED
#define	__ARM_MC1892VM14_IRQ_H_INCLUDED

/* 1892VM14 Interrupts */
#define MC1892VM14_SPI_IRQ			32

#define MC1892VM14_IRQ_SDMA0		(MC1892VM14_SPI_IRQ + 8)
#define MC1892VM14_IRQ_UART0		(MC1892VM14_SPI_IRQ + 64)
#define MC1892VM14_IRQ_UART1		(MC1892VM14_SPI_IRQ + 65)
#define MC1892VM14_IRQ_UART2		(MC1892VM14_SPI_IRQ + 66)
#define MC1892VM14_IRQ_UART3		(MC1892VM14_SPI_IRQ + 67)
#define MC1892VM14_IRQ_EMAC			(MC1892VM14_SPI_IRQ + 73)
#define MC1892VM14_IRQ_SDMMC0		(MC1892VM14_SPI_IRQ + 78)
#define MC1892VM14_IRQ_SDMMC1		(MC1892VM14_SPI_IRQ + 79)

#endif  // __ARM_MC1892VM14_IRQ_H_INCLUDED
