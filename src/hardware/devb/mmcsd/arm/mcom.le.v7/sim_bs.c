/*
 * $QNXLicenseC: 
 * Copyright 2011, QNX Software Systems.  
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

#include <sim_mmc.h>
#include <sim_sdhci.h>
#include <drvr/hwinfo.h>

int bs_init(SIM_HBA *hba)
{
	CONFIG_INFO	*cfg;
	SIM_MMC_EXT	*ext;
	sdhci_ext_t *sdhci;
	cfg = (CONFIG_INFO *)&hba->cfg;
	unsigned hwi_off = hwi_find_device("spll", 0);

	if (!cfg->NumIOPorts) {
		cfg->IOPort_Base[0]   = 0x3800B000;
		cfg->NumIOPorts = 1;
	}
	cfg->MemBase[0]   = cfg->IOPort_Base[0];
	cfg->MemLength[0] = 0x2000;
	cfg->NumMemWindows = 1;
	
	if (!cfg->NumIRQs) {
		cfg->IRQRegisters[0] = 0x6E;
		cfg->NumIRQs = 1;
	}
	
	if (sdhci_init(hba) != MMC_SUCCESS)
		return MMC_FAILURE;

	// 
	ext = (SIM_MMC_EXT *)hba->ext;
	sdhci = (sdhci_ext_t *)ext->handle;
	
	/* 
	 * Controller reports wrong base clock capability 
	 * Get the SLPP Clock from the Hwinfo Section if available
	 */
	if(hwi_off != HWI_NULL_OFF) {
		hwi_tag *tag = hwi_tag_find(hwi_off, HWI_TAG_NAME_inputclk, 0);
		if (tag) {
			sdhci->clock = ((tag->inputclk.clk) / (tag->inputclk.div));
			ext->clock = sdhci->clock;
		}
	}
	
	return MMC_SUCCESS;
}

int bs_dinit(SIM_HBA *hba)
{
	SIM_MMC_EXT	*ext;
	sdhci_ext_t	*sdhci;

	ext  = (SIM_MMC_EXT *)hba->ext;
	sdhci = (sdhci_ext_t *)ext->handle;

	return (CAM_SUCCESS);
}

