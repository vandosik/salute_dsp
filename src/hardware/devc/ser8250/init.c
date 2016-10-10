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
#include <sys/mman.h>

void
set_port(DEV_8250 *dev, unsigned port, unsigned mask, unsigned data) {
	unsigned char c;

	c = dev->read_8250(port);
	dev->write_8250(port, (c & ~mask) | (data & mask));
}

static void
clear_device(DEV_8250 *dev) {
	unsigned char tmp;

	dev->write_8250(dev->port[REG_IE], IE_CLR_ALL);	    /* Disable all interrupts		*/
	tmp = dev->read_8250(dev->port[REG_LS]);			/* Clear Line Status Interrupt	*/
	tmp = dev->read_8250(dev->port[REG_RX]);			/* Clear RX Interrupt	 		*/
	tmp = dev->read_8250(dev->port[REG_TX]);			/* Clear TX Interrupt	 		*/
	tmp = dev->read_8250(dev->port[REG_MS]);			/* Clear Modem Interrupt 		*/
	dev->write_8250(dev->port[REG_FC], 0x00);		    /* Disable and reset FIFOs 		*/
}

//
// Clean up the device then add it to the interrupt list and enable it.
//
int
ser_attach_intr(DEV_8250 *dev) {
	uintptr_t	*port = dev->port;
	struct dev_list	**owner;
	struct dev_list *curr;

	// According to the National bug sheet you must wait for the transmit
	// holding register to be empty.
	do {
	} while((dev->read_8250(port[REG_LS]) & LSR_TXRDY) == 0);

	// Clean the device so we get a level change on the intr line to the bus.
	// Enable out2 (gate intr to bus)
	set_port(dev, port[REG_MC], MCR_OUT2, MCR_OUT2);
	clear_device(dev);

	// Add it to the interrupt list
	owner = &devices;
	for( ;; ) {
		curr = *owner;
		if(curr == NULL) {
			curr = malloc(sizeof(*curr));
         if (curr == NULL)
         {
            slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_ERROR, "io-char: Allocation of device interrupt entry failed (%d)", errno);
            return (-1);
         }
			*owner = curr;
			curr->next = NULL;
			curr->device = NULL;
			break;
		}
		if(curr->device->intr == dev->intr) break;
		owner = &curr->next;
	}
	// Delay interrupts while we're fiddling around with the list
	InterruptMask(dev->intr, -1);
	dev->next = curr->device;
	curr->device = dev;
	InterruptUnmask(dev->intr, -1);

	// If first handler, attach the interrupt.
	if(curr->device->next == NULL) {
		curr->iid = InterruptAttach(dev->intr, ser_intr, curr, 0, 0);
	}

	// Enable ALL interrupt sources.
	if (dev->tty.c_cflag & CREAD)
		dev->write_8250(port[REG_IE], IE_SET_ALL);
	else
		dev->write_8250(port[REG_IE], (IE_SET_ALL & ~IE_RX) );

	return EOK;
}


void
ser_detach_intr(DEV_8250 *dev) {
	struct dev_list		*curr_list, *prev_list;
	DEV_8250			*curr_dev, *prev_dev;
	uintptr_t			*port = dev->port;

	// Disable ALL interrupt sources
	dev->write_8250(port[REG_IE], IE_CLR_ALL);	/* Disable interrupts */
	set_port(dev, port[REG_MC], MCR_OUT2, 0x00);	// Disable out2

	//
	// Remove from list of devices to scan on an interrupt.
	//
	
   // Delay interrupts while we're fiddling around with the list
   InterruptMask(dev->intr, -1);
	
   // Find the right interrupt list
   curr_list = prev_list = devices;
   while (curr_list)
   {
      if (curr_list->device->intr == dev->intr)
      {
         /* Now that we found the right interrupt list
          * find and remove the device from the device list (but don't free it) 
          */
         prev_dev = curr_dev = curr_list->device;
         while (curr_dev)
         {
            if (curr_dev == dev)
            {
               if (curr_dev == curr_list->device) /* If first entry in device list */
               {
                  curr_list->device = curr_list->device->next;
                  if (curr_list->device == NULL) /* No more devices in list */
                  {
                     if (curr_list == devices) /* If first list in list of lists :-) */
                        devices = devices->next;
                     else 
                        prev_list->next = curr_list->next;
		               InterruptDetach(curr_list->iid);
                     free(curr_list);  /* Free empty list */
                     curr_list = NULL;
                  }

               }
               else                
                  prev_dev->next = curr_dev->next;
   
               curr_dev = NULL;
               curr_list = NULL; /* We found what we were looking for, now bail out of both loops */
            }
            else
            {
               if (prev_dev != curr_dev)
                  prev_dev = prev_dev->next;
               curr_dev = curr_dev->next;
	         }
         }
      }
      else
      {
         if (prev_list != curr_list)
            prev_list = prev_list->next;
         curr_list = curr_list->next;
      }
   }
   clear_device(dev);
   InterruptUnmask(dev->intr, -1);

	dev->intr = _NTO_INTR_SPARE;
   return;
}


DEV_8250 *
create_device(TTYINIT *dip, unsigned unit, int map_dev_mem) {
	DEV_8250 			*dev;
	unsigned			i;
	uintptr_t			port;

	// Get a device entry and the input/output buffers for it.
	if ((dev = (void *) malloc(sizeof(*dev))) == NULL)
   {
      slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_ERROR, "io-char: Allocation of device entry failed (%d)", errno);
      return (dev);
   }
	memset(dev, 0, sizeof(*dev));

	// Get buffers.
	dev->tty.ibuf.head = dev->tty.ibuf.tail = dev->tty.ibuf.buff = malloc(dev->tty.ibuf.size = dip->isize);
   if (dev->tty.ibuf.head == NULL)
   {
      slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_ERROR, "io-char: Allocation of input buffer failed (%d)", errno);
      free(dev);
      return (NULL);
   }
      
	dev->tty.obuf.head = dev->tty.obuf.tail = dev->tty.obuf.buff = malloc(dev->tty.obuf.size = dip->osize);
   if (dev->tty.obuf.head == NULL)
   {
      slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_ERROR, "io-char: Allocation of output buffer failed (%d)", errno);
      free(dev->tty.ibuf.head);
      free(dev);
      return (NULL);
   }
   
	dev->tty.cbuf.head = dev->tty.cbuf.tail = dev->tty.cbuf.buff = malloc(dev->tty.cbuf.size = dip->csize);
   if (dev->tty.cbuf.head == NULL)
   {
      slogf(_SLOG_SETCODE(_SLOGC_CHAR, 0), _SLOG_ERROR, "io-char: Allocation of canonical buffer failed (%d)", errno);
      free(dev->tty.ibuf.head);
      free(dev->tty.obuf.head);
      free(dev);
      return (NULL);
   }
   
	if (dip->highwater)
		dev->tty.highwater = dip->highwater;
	else
		dev->tty.highwater = dev->tty.ibuf.size - FIFO_SIZE * 2;
	
	if (dev->tty.highwater <= 0)
		return (NULL);

	strcpy(dev->tty.name, dip->name);

	dev->tty.baud = dip->baud;
	dev->tty.fifo = dip->fifo;

#if defined(__X86__)
    // If specified, map device physical memory instead of I/O space and set the h/w access function pointers
    if (map_dev_mem)
    {
        port = (uintptr_t)mmap_device_memory(NULL, 
                                 (sizeof(dev->port)/sizeof(dev->port[0])) << dip->port_shift,
						          PROT_READ | PROT_WRITE | PROT_NOCACHE,
						          MAP_SHARED | MAP_PHYS,
						          dip->port );
        dev->read_8250 = &mem_read;
        dev->write_8250 = &mem_write;
    }
    else
#endif
    {
        port = mmap_device_io(
		       (sizeof(dev->port)/sizeof(dev->port[0])) << dip->port_shift, dip->port);
        dev->read_8250 = &io_read;
        dev->write_8250 = &io_write;
    }
	for(i = 0; i < sizeof(dev->port)/sizeof(dev->port[0]); ++i) {
		dev->port[i] = port;
		port += 1 << dip->port_shift;
	}
	dev->intr = dip->intr;
	dev->clk = dip->clk;
	dev->div = dip->div;

	dev->tty.flags = EDIT_INSERT | LOSES_TX_INTR;
	dev->tty.c_cflag = dip->c_cflag;
	dev->tty.c_iflag = dip->c_iflag;
	dev->tty.c_lflag = dip->c_lflag;
	dev->tty.c_oflag = dip->c_oflag;
   dev->tty.verbose = dip->verbose;

	// Initialize termios cc codes to an ANSI terminal.
	ttc(TTC_INIT_CC, &dev->tty, 0);

	// Initialize the device's name.
	// Assume that the basename is set in device name.  This will attach
	// to the path assigned by the unit number/minor number combination
	unit = SET_NAME_NUMBER(unit) | NUMBER_DEV_FROM_USER;
	ttc(TTC_INIT_TTYNAME, &dev->tty, unit);

	// Initialize power management structures before attaching ISR
	ttc(TTC_INIT_POWER, &dev->tty, 0);

  /* Disable Interrupts on port 
   * Interrupts must be disbled on chip for all ports on a
   * multiport card before attaching an ISR to any of the ports.
   * enable_device() must be called for each port before returning 
   * from options() to complete port initialization
   */
	clear_device(dev);

   return (dev);
}

void free_device( DEV_8250 *dev )
{
   free(dev->tty.ibuf.head);
   free(dev->tty.obuf.head);
   free(dev->tty.cbuf.head);
   ser_detach_intr(dev);
   free(dev);
}

int
enable_device( DEV_8250 *dev)
{
	unsigned char		reg;
	
	// Only setup IRQ handler for non-pcmcia devices.
	// Pcmcia devices will have this done later when card is inserted.
	if(dev->port != 0 && dev->intr != _NTO_INTR_SPARE) 
	{
		/*
		 * If CREAD is set (default) then we must clear it before we call ser_stty(), 
		 * else ser_stty() will enable the receive interrupt before we attach our ISR.
		 * ser_attach_intr() will enable all the interrupts after the ISR is attached.
		 */
		if (dev->tty.c_cflag & CREAD)
		{
			dev->tty.c_cflag &= ~(CREAD);
			ser_stty(dev);
			dev->tty.c_cflag |= CREAD;
		}
		else
			ser_stty(dev);
		
		if (ser_attach_intr(dev) == -1)
		{
			free_device(dev);
			return (-1);
		}
	}

	// Extract the rx and tx fifo sizes
	dev->rx_fifo = dev->tty.fifo & 0xf;
	dev->tx_fifo = (dev->tty.fifo >> 4) & 0xf;

	// Set RX fifo trigger level
	switch(dev->rx_fifo)
	{
		case 1:
		default: 
			dev->fcr = FCR_RX_TRIG_1; 
			break;	
	    case 4:
			dev->fcr = FCR_RX_TRIG_4;
			break;
    	case 8:
			dev->fcr = FCR_RX_TRIG_8;
			break;
	    case 14:
			dev->fcr = FCR_RX_TRIG_14;
			break;
    }
	dev->fcr |= FCR_FIFO_ENABLE;
	dev->write_8250(dev->port[REG_FC], dev->fcr);

#if defined(VARIANT_jacinto)
	dev->write_8250(dev->port[REG_PWREMU], (PWREMU_FREE | PWREMU_URRST | PWREMU_UTRST));	/* Enable Tx/Rx in Free running mode */
	dev->write_8250(dev->port[REG_MDR], 0x0);	/* 16x over-sampling */
#endif

	/* Turn on DTR and RTS */
	set_port(dev, dev->port[REG_MC], MCR_DTR|MCR_RTS, MCR_DTR|MCR_RTS);
      
	/* Get current MSR stat */
	reg = dev->read_8250(dev->port[REG_MS]);

	if(reg & MSR_DDCD)
		tti(&dev->tty, (reg & MSR_DCD) ? TTI_CARRIER : TTI_HANGUP);
					
	if((reg & MSR_DCTS)  &&  (dev->tty.c_cflag & OHFLOW))
		tti(&dev->tty, (reg & MSR_CTS) ? TTI_OHW_CONT : TTI_OHW_STOP);

	/* Attach the resource manager */
	ttc(TTC_INIT_ATTACH, &dev->tty, 0);
	return EOK; 
}



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.5.0/trunk/hardware/devc/ser8250/init.c $ $Rev: 746538 $")
#endif
