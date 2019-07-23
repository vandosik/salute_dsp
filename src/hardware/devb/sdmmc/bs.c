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

int sdhci_pci_init( sdio_hc_t *hc );

#define SDIO_VENDOR_INTEL

sdio_product_t	sdio_sdhci_product[] = {
	{ SDIO_DEVICE_ID_WILDCARD, ( PCI_CLASS_SYSTEM | PCI_SUBCLASS_SYSTEM_SD ), 0, "Generic SDHCI", sdhci_pci_init },
	{ 0, 0, 0, NULL, NULL }
};

sdio_product_t	sdio_sdhci_o2_products[] = {
	{ PCI_DEVICE_ID_O2_8321, 0, 0, "O2 Micro 8321", sdhci_pci_init },
	{ 0, 0, 0, NULL, NULL }
};

//#define PCI_DEVICE_ID_INTEL_BAYTRAIL_MMC    0x0f14
#define PCI_DEVICE_ID_INTEL_BAYTRAIL_SDIO   0x0f15
#define PCI_DEVICE_ID_INTEL_BAYTRAIL_SD     0x0f16
#define PCI_DEVICE_ID_INTEL_BAYTRAIL_SDHCI  0x0f50

#define PCI_DEVICE_ID_INTEL_BROXTON_SDCARD (0x5aca)
#define PCI_DEVICE_ID_INTEL_BROXTON_EMMC   (0x5acc)
#define PCI_DEVICE_ID_INTEL_BROXTON_SDIO   (0x5ad0)

sdio_product_t	sdio_sdhci_intel_products[] = {
	{ PCI_DEVICE_ID_INTEL_SCH_SDIO1, 0, 0, "Intel SCH", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_SCH_SDIO2, 0, 0, "Intel SCH", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_SCH_SDIO3, 0, 0, "Intel SCH", sdhci_pci_init },
//	{ PCI_DEVICE_ID_INTEL_BAYTRAIL_MMC, 0, 0, "Intel BayTrail", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_BAYTRAIL_SDIO, 0, 0, "Intel BayTrail", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_BAYTRAIL_SD,   0, 0, "Intel BayTrail", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_BAYTRAIL_SDHCI, 0, 0, "Intel BayTrail", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_BROXTON_SDCARD, 0, 0, "Intel Broxton", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_BROXTON_EMMC,   0, 0, "Intel Broxton", sdhci_pci_init },
	{ PCI_DEVICE_ID_INTEL_BROXTON_SDIO,   0, 0, "Intel Broxton", sdhci_pci_init },
	{ SDIO_DEVICE_ID_WILDCARD, ( PCI_CLASS_SYSTEM | PCI_SUBCLASS_SYSTEM_SD ), 0, "Generic SDHCI", sdhci_pci_init },
	{ 0, 0, 0, NULL, NULL }
};

sdio_vendor_t	sdio_vendors[] = {
	{ PCI_VENDOR_ID_INTEL, "Intel", sdio_sdhci_intel_products },
	{ PCI_VENDOR_ID_O2, "O2 Micro", sdio_sdhci_o2_products },
	{ SDIO_VENDOR_ID_WILDCARD, "Generic", sdio_sdhci_product },
	{ 0, NULL, NULL }
};

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/x86/o/bs.c $ $Rev: 794188 $")
#endif
