/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
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

// Module Description:  board specific interface

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <devctl.h>
#include <confname.h>
#include <hw/pci.h>
#include <hw/sysinfo.h>
#include <hw/pci_devices.h>
#include <sys/utsname.h>
#include <sys/pathmgr.h>

#include <sdhci.h>

#include <bs.h>

#define SDIO_1892VM14_VID			0x01
#define SDIO_1892VM14_I0_DID		0x01
#define SDIO_1892VM14_I1_DID		0x02


sdio_product_t	sdio_sdhci_product[] = {
	{ SDIO_DEVICE_ID_WILDCARD, 0, 0, "Generic SDHCI", sdhci_init },
	{ 0, 0, 0, NULL, NULL }
};

sdio_product_t	sdio_sdhci_el24_products[] = {
	{ SDIO_1892VM14_I0_DID, 0, 0, "sdhci0", sdhci_init },
    { SDIO_1892VM14_I1_DID, 0, 0, "sdhci1", sdhci_init },
    { 0, 0, 0, NULL, NULL }
};


sdio_vendor_t	sdio_vendors[] = {
	{ SDIO_1892VM14_VID, "1892vm14-salute-el24", sdio_sdhci_el24_products },
    { SDIO_VENDOR_ID_WILDCARD, "Generic", sdio_sdhci_product },
	{ 0, NULL, NULL }
};

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/x86/o/bs.c $ $Rev: 794188 $")
#endif
