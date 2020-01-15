#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elcore-manager.h>
#include <sys/mman.h>
#include <errno.h>
#include "mcom_dsp.h"

#include <unistd.h>

char* fw_path = "/tmp/set_regs.fw.bin";

int getbytes(uint8_t** data, const char *filename, uint32_t *size)
{
	printf("%s: entry\n", __func__);

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		perror("err opening file");
		*size = 0;
		return -1;
	}
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);

	printf("size of file: %u\n", *size);
	
	*data = malloc(*size);
	
	if (*data == NULL)
	{
		perror("err alloc data");
	}
	
	printf ("bytes read: %d\n",fread(*data, 1, *size, f));

	if (*data == NULL)
	{
		perror("err reading data");
	}
	
	fclose(f);

	return 0;
}

int dsp_core_print_regs(dsp_core* core)
{
   int iter = 0;
    printf("\tDump regs %d core:\n",core->id);
    printf("\tDSCR:       %4.8X\n",dsp_get_reg16(core, DLCR30M_DSCR));
    printf("\tSR:         %4.8X\n",dsp_get_reg16(core, DLCR30M_SR));
    printf("\tIDR:        %4.8X\n",dsp_get_reg16(core, DLCR30M_IDR));
    printf("\tEFR:        %4.8X\n",dsp_get_reg32(core, DLCR30M_EFR));
    printf("\tDSTART:     %4.8X\n",dsp_get_reg32(core, DLCR30M_DSTART));
    printf("\tIRQR:       %4.8X\n",dsp_get_reg32(core, DLCR30M_IRQR));
    printf("\tIMASKR:     %4.8X\n",dsp_get_reg32(core, DLCR30M_IMASKR));
    printf("\tTMR:        %4.8X\n",dsp_get_reg32(core, DLCR30M_TMR));
    printf("\tARBR:       %4.8X\n",dsp_get_reg16(core, DLCR30M_ARBR));
    printf("\tPC:         %4.8X\n",dsp_get_reg16(core, DLCR30M_PC));
    printf("\tSS:         %4.8X\n",dsp_get_reg16(core, DLCR30M_SS));
    printf("\tLA:         %4.8X\n",dsp_get_reg16(core, DLCR30M_LA));
    printf("\tCSL:        %4.8X\n",dsp_get_reg16(core, DLCR30M_CSL));
    printf("\tLC:         %4.8X\n",dsp_get_reg16(core, DLCR30M_LC));
    printf("\tCSH:        %4.8X\n",dsp_get_reg16(core, DLCR30M_CSH));
    printf("\tSP:         %4.8X\n",dsp_get_reg16(core, DLCR30M_SP));
    printf("\tSAR:        %4.8X\n",dsp_get_reg16(core, DLCR30M_SAR0));
    printf("\tCNTR:       %4.8X\n",dsp_get_reg16(core, DLCR30M_CNTR));
    /*for (i=0;i<6;i++)
    printf("\tSAR%d       %4.8X\n",i,dsp_get_reg16(core,DLCR30M_dbSAR(i)));*/
    printf("\tCCR:        %4.8X\n",dsp_get_reg16(core,DLCR30M_CCR));
    printf("\tPDNR:       %4.8X\n",dsp_get_reg16(core,DLCR30M_PDNR));
    printf("\tSFR:        %4.8X\n",dsp_get_reg32(core,DLCR30M_SFR));
    printf("\tIVAR:       %4.8X\n",dsp_get_reg16(core,DLCR30M_IVAR));
    printf("\tdbPCa:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCa));
    printf("\tdbPCf:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCf));
    printf("\tdbPCd:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCd));
    printf("\tdbPCe:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCe));
    //printf("\tdbSAR0:     %4.8X\n",dsp_get_reg16(core,DLCR30M_dbSAR0));
    printf("\tdbCNTR:     %4.8X\n",dsp_get_reg16(core,DLCR30M_dbCNTR));
    printf("\tCNT_RUN:    %4.8X\n",dsp_get_reg32(core, DLCR30M_Cnt_RUN));
    /*for (i=0;i<6;i++)
    printf("\tdbSAR%d       %4.8X\n",i,dsp_get_reg16(core,DLCR30M_dbSAR(i)));*/
    
   
    for ( ; iter <= 31; iter++)
    {
    printf("\tR%u:\t\t%4.8X\n", iter, dsp_get_reg32(core,
                                                    (!(iter % 2)? DLCR30M_R2L(iter):DLCR30M_R1L(iter))));
    }

    return 0;
}

int dsp_cluster_print_regs(delcore30m_t* dsp)
{
    int i = 0;
    printf("CLUSTER REGISTER INFORMATION:\n");
    printf("DSP_MASKR:            %4.8X\n",dsp_get_reg32(dsp,DLCR30M_MASKR));
    printf("DSP_QSTR:             %4.8X\n",dsp_get_reg32(dsp,DLCR30M_QSTR));
    printf("DSP_CSR:              %4.8X\n",dsp_get_reg32(dsp,DLCR30M_CSR));
    printf("TOTAL_RUN:            %4.8X\n",dsp_get_reg32(dsp,DLCR30M_TOTAL_RUN));
    printf("TOTAL_CLK:            %4.8X\n",dsp_get_reg32(dsp,DLCR30M_TOTAL_CLK));


    for (i=0;i<dsp->core_count;i++)
        dsp_core_print_regs(&(dsp->core[i]));
    return 0;
}

void *elcore_func_init(void *hdl, char *options)
{
	delcore30m_t			*dev;

	dev = calloc(1, sizeof(delcore30m_t));
	if ( dev == NULL )
	{
	    printf("%s: error\n", __func__);
	    return NULL;
	}

	dev->drvhdl.hdl = hdl;
    
	if ( (void *)( dev->base = mmap_device_memory( NULL, DLCR30M_SIZE,
	        PROT_READ | PROT_WRITE | PROT_NOCACHE, 0,
	        DLCR30M_BASE ) ) == MAP_FAILED )
	{
		fprintf(stderr,"DSP mapping device memory failed\n");
		free(hdl);
		
        return NULL;
	}

	dev->regs = dev->base;
	dev->core_count = DLCR30M_MAX_CORES;
    
	dev->core[0].id = 0;
	dev->core[1].id = 1;
	
	dev->core[0].cluster = (struct delcore30m_t*)dev;
	dev->core[1].cluster = (struct delcore30m_t*)dev;

	dev->core[0].xyram = dev->base + DLCR30M_DSP0_XYRAM;
	dev->core[1].xyram = dev->base + DLCR30M_DSP1_XYRAM;
	
	dev->core[0].pram = dev->base + DLCR30M_DSP0_PRAM;
	dev->core[1].pram = dev->base + DLCR30M_DSP1_PRAM;
	
	dev->core[0].regs = dev->base + DLCR30M_DSP0_REGS;
	dev->core[1].regs = dev->base + DLCR30M_DSP1_REGS;

	dsp_cluster_print_regs(dev);
	
// 	int it = 0;
// 	
// 	for (;it < DLCR30M_SIZE; )
// 	{
// 		printf(" %.2x ", *(dev->base + it));
// 		it++;
// 		if (it % 16 == 0) printf("\n");
// 	}
// 	printf("\n");
	
	uint32_t size;
	uint8_t *fw_data;
	getbytes(&fw_data, fw_path, &size);
	
	if (fw_data == NULL)
	{
		perror("File opening");
		elcore_func_fini(hdl);
	}
	
	
	
	delcore30m_firmware firmware = {
		.cores = 1,
		.size = size,
		.data = fw_data
	};
	
	elcore_set_pram(dev, &firmware);
	
	
	elcore_start_core(dev, firmware.cores);
	
	delay(10000);
	printf("\n\n\n\n\n DOING DSP PROG \n\n\n\n\n");
	
	dsp_cluster_print_regs(dev);
	
	printf("\n\n\n\n\n STOP DSP \n\n\n\n\n");
	
	elcore_stop_core(dev, firmware.cores);
	
    printf("%s: success\n", __func__);
    
	free(firmware.data);
	
    return dev;
    
}

int		elcore_start_core(void *hdl, uint32_t core_num)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	dsp_core				*core = &dev->core[core_num];

	dsp_set_reg16(core, DLCR30M_PC, 0x0);
	
	dsp_set_reg16(core,DLCR30M_DSCR,dsp_get_reg16(core,DLCR30M_DSCR) | DLCR30M_DSCR_RUN);
	
	return EOK;
	
}

int		elcore_stop_core(void *hdl, uint32_t core_num)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	dsp_core				*core = &dev->core[core_num];

	dsp_set_reg16(core, DLCR30M_DSCR, (dsp_get_reg16(core,DLCR30M_DSCR)  & (~DLCR30M_DSCR_RUN)));
	
	return EOK;
	
}

int		elcore_set_pram(void *hdl, delcore30m_firmware *firmware)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	uint32_t pram_size;
	
	pram_size = dsp_get_reg32(dev, DLCR30M_CSR);
	
	switch ((pram_size & DLCR30M_CSR_PM_CONFIG_MASK) >> 2)
	{
	case 0:
		pram_size = DLCR30M_BANK_SIZE;
		break;
	case 1:
		return -EINVAL;
	case 2:
		pram_size = 3 * DLCR30M_BANK_SIZE;
		break;
	case 3:
		pram_size = 4 * DLCR30M_BANK_SIZE;
		break;
	}
	if (firmware->size > pram_size)
	{
		printf("Firmware is too big\n");
		return -EINVAL;
	}
	
	memcpy(dev->core[firmware->cores].pram, firmware->data, firmware->size);
	
	dev->core[firmware->cores].fw_size = firmware->size;
	dev->core[firmware->cores].fw_ready = DLCR30M_FWREADY;
	
	return EOK;
}

int		elcore_release_pram(void *hdl, uint32_t core_num)
{
	delcore30m_t			*dev = hdl;
	
	dev->core[core_num].fw_ready = DLCR30M_FWEMPTY;
	
	return EOK;
}

int		elcore_reset_core(void *hdl, uint32_t core_num)
{
	delcore30m_t			*dev = hdl;
	dsp_core				*core = &dev->core[core_num];
	uint32_t				iter;
	
	dsp_set_reg16(core, DLCR30M_PC, 0x0);
	
	dsp_set_reg32(core, DLCR30M_DSCR, 0x0);
	dsp_set_reg32(core, DLCR30M_SR, 0x0);
	dsp_set_reg32(core, DLCR30M_DSTART, 0x0);
	dsp_set_reg32(core, DLCR30M_IRQR, 0x0);
	dsp_set_reg32(core, DLCR30M_IMASKR, 0x0);
	dsp_set_reg32(core, DLCR30M_TMR, 0x0);
	dsp_set_reg16(core, DLCR30M_ARBR, 0x0F01);
	dsp_set_reg32(core, DLCR30M_LA, 0xFFFF);
	dsp_set_reg32(core, DLCR30M_CSL, 0x0);
	dsp_set_reg32(core, DLCR30M_LC, 0x0);
	dsp_set_reg32(core, DLCR30M_CSH, 0x0);
	dsp_set_reg32(core, DLCR30M_SP, 0x0);
	dsp_set_reg16(core, DLCR30M_CNTR, 0x0);
	
	for (iter = 0; iter < 7; iter++) //???????????? WHY <= 7
	{
		dsp_set_reg32(core, DLCR30M_A(iter), 0x0);
	}
	
    dsp_set_reg16(core, DLCR30M_SAR0, 0xFFFF);
	
    for (iter = 0; iter < 7; iter++)
	{
        dsp_set_reg16(core,DLCR30M_SAR(iter),0xFFFF);
	}
	dsp_set_reg16(core, DLCR30M_CCR, 0x0);
	dsp_set_reg16(core, DLCR30M_PDNR, 0x0);

	for (iter = 0; iter < 7; iter++)
	{
		dsp_set_reg16(core,DLCR30M_I(iter), 0x0);
	}
	for (iter = 0; iter < 7; iter++)
	{
		dsp_set_reg16(core, DLCR30M_M(iter), 0xFFFF);
	}
	dsp_set_reg16(core, DLCR30M_IT, 0x0);
	dsp_set_reg16(core, DLCR30M_MT, 0xFFFF);
	dsp_set_reg16(core, DLCR30M_DT, 0x0);
	dsp_set_reg16(core, DLCR30M_IVAR, 0x1F00);
	dsp_set_reg16(core, DLCR30M_dbDCSR, 0x0);
	dsp_set_reg16(core, DLCR30M_dbSAR0, 0xFFFF);
	dsp_set_reg16(core, DLCR30M_dbCNTR, 0x0);

	for (iter = 0; iter < 7; iter++)
	{
	    dsp_set_reg16(core, DLCR30M_dbSAR(iter), 0xFFFF);
	}
}

void elcore_func_fini(void *hdl)
{
    delcore30m_t			*dev = hdl;

    
    munmap_device_memory(dev->base ,DLCR30M_SIZE);
    free(hdl);
	
    printf("%s: success\n", __func__);
}
