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

static inline uint32_t io_read (uintptr_t port)
{
    return in8(port);
}

static inline void io_write (uintptr_t port, uint32_t val)
{
    out8(port, (uint8_t)val);
}

static inline uint32_t mem_read(uintptr_t port)
{
    return (*(volatile uint8_t*)port);
}

static inline void mem_write(uintptr_t port, uint32_t val)
{
    (*(volatile uint8_t*)port) = (uint8_t)val;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.5.0/trunk/hardware/devc/ser8250/variant.h $ $Rev: 746538 $")
#endif
