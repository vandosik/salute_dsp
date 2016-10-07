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

/*
 * Polled serial operations for 1892VM14 
 */

#include "startup.h"
#include <arm/mc1892vm14.h>

static void parse_line(unsigned channel, const char *line, unsigned *baud,
		unsigned *clk)
{
	/*
	 * Get device base address and register stride
	 */
	if (*line != '.' && *line != '\0') {
		dbg_device[channel].base = strtoul(line, (char **) &line, 16);
		if (*line == '^')
			dbg_device[channel].shift = strtoul(line + 1, (char **) &line, 0);
	}

	/*
	 * Get baud rate
	 */
	if (*line == '.')
		++line;
	if (*line != '.' && *line != '\0')
		*baud = strtoul(line, (char **) &line, 0);

	/*
	 * Get clock rate
	 */
	if (*line == '.')
		++line;
	if (*line != '.' && *line != '\0')
		*clk = strtoul(line, (char **) &line, 0);
}

/*
 * Initialise one of the serial ports
 */
void init_ser1892vm14(unsigned channel, const char *init, const char *defaults)
{
	unsigned int baud, clk, base;
	unsigned int bdiv;
	unsigned val32;

	/*
	 * Default peripheral clock rate is 144MHz
	 */
	clk = MC1892VM14_UART_CLK_FREQ;

	parse_line(channel, defaults, &baud, &clk);
	parse_line(channel, init, &baud, &clk);
	base = dbg_device[channel].base;

	if (baud == 0)
		return;
	
	
	bdiv = clk / (16 * baud);
	
	/* 
	 * Eat extraneous characters before
	 */	
	while( (in32(base + MC1892VM14_UART_LSR) & MC1892VM14_UART_LSR_RDR) != 0)
		in32(base + MC1892VM14_UART_RBR);
	
	/*
	 * Wait Busy before access to the DLL and DHL
	 */
	while ( (in32(base + MC1892VM14_UART_USR) & 0x1 ) == 1);
	
	/*
	 * Set baud rate
	 */
	val32 = in32(base + MC1892VM14_UART_LCR);
	out32(base + MC1892VM14_UART_LCR, val32 | MC1892VM14_UART_LCR_DLAB);
	out32(base + MC1892VM14_UART_DLL, bdiv & 0xff);
	out32(base + MC1892VM14_UART_DHL, (bdiv >> 8) & 0xff);
	
	/*
	 * Set 8-bit, 1-stop, no-parity (also clear DLAB bit)
	 */
	out8(base + MC1892VM14_UART_LCR, MC1892VM14_UART_LCR_DLS_8);
	
	/* 
	 * Eat extraneous characters
	 */
	while( (in32(base + MC1892VM14_UART_LSR) & MC1892VM14_UART_LSR_RDR) != 0)
		in32(base + MC1892VM14_UART_RBR);
}

/*
 * Send a character
 */
void put_ser1892vm14(int c)
{
	unsigned base = dbg_device[0].base;

	while ((in8(base + MC1892VM14_UART_LSR) & MC1892VM14_UART_LSR_TEMPT) == 0);

	if (c == '\n') {
		out8(base + MC1892VM14_UART_THR, '\r');
		while ((in32(base + MC1892VM14_UART_LSR) & MC1892VM14_UART_LSR_TEMPT) == 0);
	}
	out8(base + MC1892VM14_UART_THR, c);
}
