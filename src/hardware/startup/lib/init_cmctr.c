#include "startup.h"
#include <arm/mc1892vm14.h>

#define EL24D1_XTI_FREQ		24000000

void init_cmctr(void)
{
	struct cmctr_entry *cmctr = set_syspage_section(&lsp.cmctr, sizeof(struct cmctr_entry));
	cmctr->spllclk = ((in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_SEL_SPLL_REG) & 0xFF) + 1) * EL24D1_XTI_FREQ;
	cmctr->div_sys0_ctr = (in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_DIV_SYS0_REG) & 0x01) + 1;
	cmctr->div_sys1_ctr = (in32(MC1892VM14_CMCTR_BASE + MC1892VM14_CMCTR_DIV_SYS1_REG) & 0x01) + 1;
	cmctr->l1_hclk = cmctr->spllclk / cmctr->div_sys0_ctr;
	cmctr->l3_pclk = cmctr->l1_hclk / cmctr->div_sys1_ctr;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn/product/branches/6.5.0/trunk/hardware/startup/lib/x86/init_cmctr.c $ $Rev: 711024 $")
#endif
