/*
 * $QNXLicenseC: 
 * Copyright 2012 QNX Software Systems.  
 * Copyright 2013, Adeneo Embedded.
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
 * Elvees 1892VM14 General Interrupt Controller support.
 */

#include "startup.h"
#include <arm/mc1892vm14.h>
#include <arm/mpcore.h>


static const struct startup_intrinfo	intrs[] = {

	/* ARM General Interrupt Controller */
	{	.vector_base     = _NTO_INTR_CLASS_EXTERNAL,
		.num_vectors     = 160,
		.cascade_vector  = _NTO_INTR_SPARE,
		.cpu_intr_base   = 0,
		.cpu_intr_stride = 0,
		.flags           = 0,
		.id = { INTR_GENFLAG_LOAD_SYSPAGE,	0, &interrupt_id_gic },
		.eoi = { INTR_GENFLAG_LOAD_SYSPAGE | INTR_GENFLAG_LOAD_INTRMASK, 0, &interrupt_eoi_gic },
		.mask            = &interrupt_mask_gic,
		.unmask          = &interrupt_unmask_gic,
		.config          = &interrupt_config_gic,
		.patch_data      = &mpcore_scu_base,
	},
};

void init_intrinfo()
{
	/* mpcore_scu_base is set in function armv_detect_a9 */
	unsigned gic_dist = mpcore_scu_base + MPCORE_GIC_DIST_BASE;
	unsigned gic_cpu = mpcore_scu_base + MPCORE_GIC_CPU_BASE;

	/*
	 * Initialise GIC distributor and our cpu interface
	 */
	arm_gic_init(gic_dist, gic_cpu);

	add_interrupt_array(intrs, sizeof(intrs));
}
