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


#include "externs.h"
/*
 * Process data in a line status register
 */
unsigned 
process_lsr(DEV_8250 *dev, unsigned char lsr) {
	unsigned key = 0;//, eventflag = 0;

	// Return immediately if no errors.
	if((lsr & (LSR_BI|LSR_OE|LSR_FE|LSR_PE)) == 0)
	{
#if defined(MPC8540_WORKAROUND)
		/* No break, can re-enable the LSR interrupt. */
		dev->write_8250(dev->port[REG_IE], IE_SET_ALL);
#endif
		return(0);
	}
	
	// Save the error as out-of-band data which can be retrieved via devctl().
	dev->tty.oband_data |= (lsr >> 1) & 0x0f;
	atomic_set(&dev->tty.flags, OBAND_DATA);

	if(lsr & LSR_BI)
	{
		key |= TTI_BREAK;
#if defined(MPC8540_WORKAROUND)
		/* On the MPC8540 chip, when a break occurs, the lsr interrupt stays
		asserted until a character is input.  We therefore need to disable the
		lsr interrupt to prevent an interrupt overflow condition. */
		dev->write_8250(dev->port[REG_IE], (IE_SET_ALL & ~IE_LS));
#endif		
	}
	else
	{
#if defined(MPC8540_WORKAROUND)
		/* No break, can re-enable the LSR interrupt. */
		dev->write_8250(dev->port[REG_IE], IE_SET_ALL);
#endif		
		if(lsr & LSR_OE)
			key |= TTI_OVERRUN;
		else if(lsr & LSR_FE)
			key |= TTI_FRAME;
		else if(lsr & LSR_PE)
			key |= TTI_PARITY;
	}
   return (key);
	}

/*
 * Serial interrupt handler
 */
const struct sigevent *
ser_intr(void *area, int id) {
	struct dev_list	*list = area;
	int				status = 0;
	int				something_happened;
	unsigned char	msr, lsr;
	DEV_8250		*dev;
	struct sigevent *event = NULL;
	unsigned 		c;
#if defined(PA6T_WORKAROUND) 				
	int				first = 1;
#endif	

	do {
		something_happened = 0;
		for(dev = list->device; dev != NULL; dev = dev->next) {
			unsigned	iir;
			uintptr_t	*port = dev->port;

#if defined(PA6T_WORKAROUND) 				
			if(first && (dev->tx_empty_disable <= 50)) dev->tx_empty_disable = 0;
#endif			
			status = 0;

			iir = (dev->read_8250(port[REG_II]) & 0x07) 
#if defined(PA6T_WORKAROUND) 				
				^ dev->irr_fiddle
#endif				
				;
			switch(iir) {
#if defined(PA6T_WORKAROUND)			
			case II_RX ^ 0x1:
				// Work around the fact that the early PA6T-1682's have
				// the bottom bit of the IRR register flipped.
				dev->irr_fiddle = 0x1;
				// fall through
#endif				
			case II_RX:		// Receive data
			case II_LS:
            /* Some UARTs will generate a LS interrupt when there is an 
             * error anywhere in the RX FIFO, and will clear this interrupt
             * only when there are no more errors remaining in the FIFO. The 
             * error bits in REG_LS (BI/PR/FE/OE) always represent the error 
             * status for the received character at the top of the Rx FIFO. 
             * Reading the Rx FIFO updates these bits to the appropriate status
             * of the new character. This means that it is possible to get an
             * LS interrupt with none of the error status bits set, in order
             * to clear the LS interrupt we must read out all of the characters
             * in the FIFO until we find and handle the erronous character.
             */
            while((lsr = dev->read_8250(port[REG_LS])) & LSR_RXRDY)
            {
				c = dev->read_8250(port[REG_RX]);
               	c |= process_lsr(dev, lsr);
                status |= tti(&dev->tty, c);
            };
            break;
            
#if defined(PA6T_WORKAROUND)			
			case II_TX ^ 0x1:
				// Work around the fact that the early PA6T-1682's have
				// the bottom bit of the IRR register flipped.
				dev->irr_fiddle = 0x1;
				// fall through
#endif				

			case II_TX:		// Transmit buffer empty
#if defined(TL16C752B_WORKAROUND)
					// ugly, bug workaround false interrupt
					// from TL16C752B dual uart
					 while(!(dev->read_8250(port[REG_LS]) & LSR_TXRDY));
#endif					 
					dev->tty.un.s.tx_tmr = 0;
					/* Send evnet to io-char, tto() will be processed at thread time */
					atomic_set(&dev->tty.flags, EVENT_TTO);
					status |= 1;
#if defined(PA6T_WORKAROUND) 				
					if(dev->tx_empty_disable > 50) {
						// Turn off the TX holding empty interrupt - early
						// PA6T-1682's keep reporting it even though the IIR 
						// read is supposed to turn it off.
						set_port(dev, port[REG_IE], 0x2, 0x0);
					} else {
						++dev->tx_empty_disable;
					}
#endif					
				break;

			case II_MS:		// Modem change
				msr = dev->read_8250(port[REG_MS]);

				if(msr & MSR_DDCD)
					status |= tti(&dev->tty, (msr & MSR_DCD) ? TTI_CARRIER : TTI_HANGUP);
				
				
				if((msr & MSR_DCTS)  &&  (dev->tty.c_cflag & OHFLOW))
					status |= tti(&dev->tty, (msr & MSR_CTS) ? TTI_OHW_CONT : TTI_OHW_STOP);
				
				/* OBAND notification of Modem status change */	
				dev->tty.oband_data |= _OBAND_SER_MS;
				atomic_set(&dev->tty.flags, OBAND_DATA);
				atomic_set(&dev->tty.flags, EVENT_NOTIFY_OBAND);
				status |= 1;
				break;

			default:
				continue;

				}

			something_happened = 1;
			if(status) {
				if((dev->tty.flags & EVENT_QUEUED) == 0) {
					event = &ttyctrl.event;
					dev_lock(&ttyctrl);
					ttyctrl.event_queue[ttyctrl.num_events++] = &dev->tty;
					atomic_set(&dev->tty.flags, EVENT_QUEUED);
					dev_unlock(&ttyctrl);
					}
				}
			}
#if defined(PA6T_WORKAROUND) 				
			first = 0;
#endif

		} while(something_happened);
	
	return(event);
	}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.5.0/trunk/hardware/devc/ser8250/intr.c $ $Rev: 746538 $")
#endif
