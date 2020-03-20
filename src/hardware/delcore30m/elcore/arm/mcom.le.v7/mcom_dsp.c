#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elcore-manager.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <errno.h>
#include "mcom_dsp.h"

#include <unistd.h>
#include <sdma.h>


static uint32_t addr2delcore30m(uint32_t addr)
{
	return (((addr) & 0xFFFFF) >> 2);
}

static uint32_t get_dsp_addr(void *hdl, uint32_t offset)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	uint32_t				dsp_addr;
	
	
	uint32_t pram_size = ( dev->pm_conf + 1) * DLCR30M_BANK_SIZE;
	

	if (offset > pram_size)
	{
		//XYRAM adrrs per word, starting with 0x0000
		dsp_addr = addr2delcore30m(offset - pram_size);
	}
	else
	{
		dsp_addr = addr2delcore30m(offset);
	}

	return dsp_addr;
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

int dsp_cluster_print_regs(void *hdl)
{
    delcore30m_t			*dsp = hdl;
    
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

static char *elcore_opts[] =
{
    "queue0",   //queue length / parts tp devide memory to
	"queue1",
    NULL
};

static int elcore_parce_opts(delcore30m_t *dev, char *optstring)
{
    char	*options, *freeptr, *c, *value;
	int		num_val;
	
    if ( optstring == NULL )
    {
        return EOK;
    }
    freeptr = options = strdup(optstring);

    while ( options && *options != '\0' )
    {
        c = options;

		
		
        switch ( getsubopt(&options, elcore_opts, &value) )
        {
            case 0:
					num_val = atoi(value);
					printf("%s: got queue0 len: %u\n", __func__, num_val);
					
					if (num_val != 1 && num_val != 2 && num_val != 4 && num_val != 8)
					{
						fprintf(stderr,"%s: illegal mem parts\n",__func__);
						return EINVAL;
					}
					
					DLCR30M_SET_MEM_PARTS(&dev->core[0], num_val);
                break;
			case 1:
					num_val = atoi(value);
					printf("%s: got queue1 len: %u\n", __func__, num_val);
				
					if (num_val != 1 && num_val != 2 && num_val != 4 && num_val != 8)
					{
						fprintf(stderr,"%s: illegal mem parts\n",__func__);
						return EINVAL;
					}
					
					DLCR30M_SET_MEM_PARTS(&dev->core[1], num_val);

                break;
            default:
                fprintf(stderr,"%s: unknown option %s\n",__func__, c);
                return EINVAL;
        }
    }
    free(freeptr);

    return EOK;
}

void *elcore_func_init(void *hdl, char *options)
{
	delcore30m_t			*dev;
	int						it;

	dev = calloc(1, sizeof(delcore30m_t));
	if ( dev == NULL )
	{
	    printf("%s: error\n", __func__);
	    return NULL;
	}
	//necessary manipulations
	dev->drvhdl.hdl = hdl;
	dev->drvhdl.cores_num = DLCR30M_MAX_CORES;
	
	dev->core_count = DLCR30M_MAX_CORES;
    dev->irq = DLCR30M_IRQ_NUM;
	dev->pbase = DLCR30M_BASE;
	//set defaults
    DLCR30M_SET_MEM_PARTS(&dev->core[0], 1);
	DLCR30M_SET_MEM_PARTS(&dev->core[1], 1);
	
	
	if (elcore_parce_opts(dev, options) != EOK)
	{
		elcore_func_fini(dev);
        
        return NULL;
	}
	
	if ((dev->drvhdl.job_hdl = elcore_job_hdl_init(dev->core_count)) == NULL)
    {
		free(dev);
		        
		return NULL;
    }
	
	//TODO: set this somewhere else, to use jobs as less as possible on this layer
	for (it = 0; it < dev->core_count; it++)
	{
		set_core_jobs_max( &dev->drvhdl, it, DLCR30M_GET_MEM_PARTS(&dev->core[it]));
	}
	
	if ( (void *)( dev->base = mmap_device_memory( NULL, DLCR30M_SIZE,
	        PROT_READ | PROT_WRITE | PROT_NOCACHE, 0,
	        dev->pbase ) ) == MAP_FAILED )
	{
		fprintf(stderr,"DSP mapping device memory failed\n");
		free(hdl);
		
        return NULL;
	}

	dev->regs = dev->base;
    
	dev->core[0].id = 0;
	dev->core[1].id = 1;
	
	dev->core[0].cluster = (struct delcore30m_t*)dev;
	dev->core[1].cluster = (struct delcore30m_t*)dev;

	dev->core[0].xyram = dev->base + DLCR30M_DSP0_XYRAM;
	dev->core[0].xyram_phys = dev->pbase + DLCR30M_DSP0_XYRAM;
	dev->core[1].xyram = dev->base + DLCR30M_DSP1_XYRAM;
	dev->core[1].xyram_phys = dev->pbase + DLCR30M_DSP1_XYRAM;
	
	dev->core[0].pram = dev->base + DLCR30M_DSP0_PRAM;
	dev->core[0].pram_phys = dev->pbase + DLCR30M_DSP0_PRAM;
	dev->core[1].pram = dev->base + DLCR30M_DSP1_PRAM;
	dev->core[1].pram_phys = dev->pbase + DLCR30M_DSP1_PRAM;
	
	dev->core[0].regs = dev->base + DLCR30M_DSP0_REGS;
	dev->core[1].regs = dev->base + DLCR30M_DSP1_REGS;

	dsp_cluster_print_regs(dev);
	
	dev->pm_conf = (dsp_get_reg32(dev, DLCR30M_CSR) & DLCR30M_CSR_PM_CONFIG_MASK) >> 2;
	
	//FIXME:enable interrups
	dsp_set_bit_reg32(dev, DLCR30M_MASKR, 3);
	/////////////////////////////////////////////////////////////////
	if (sdma_init())
	{
		perror("sdma_init failure");
		elcore_func_fini(dev);
        
        return NULL;
	}
	
// 	delcore30m_firmware firmware = {
// 		.cores = 0,
// 		.size = size,
// 		.data = fw_data
// 	};
	
    return dev;
    
}

// int		elcore_wait_irq

int		elcore_start_core(void *hdl, uint32_t core_num)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	dsp_core				*core = &dev->core[core_num];
	uint32_t				val32;

	elcore_job_t			*cur_job = get_job_first_enqueued( &dev->drvhdl, core_num);
	
	if ( cur_job == NULL  )
	{
		printf("No jobs in queue for this core\n");
		return EINVAL;
	}
	if (cur_job->job_pub.status == ELCORE_JOB_RUNNING)
	{//TODO: this we can find out by reading DSCR
		printf("Job is already running\n");
		return EBUSY;
	}
	//enable interrupts
	val32 = dsp_get_reg32(dev, DLCR30M_MASKR);
	val32 |= DLCR30M_QSTR_CORE_MASK(core_num);
	dsp_set_reg32(dev, DLCR30M_MASKR, val32);

	cur_job->job_pub.status = ELCORE_JOB_RUNNING;
	core->job_id = cur_job->job_pub.id; 
	
	dsp_set_reg16(core, DLCR30M_PC, /*cur_job->job_pub.code.client_paddr*/cur_job->code_dspaddr);
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

int		elcore_pram_config(void *hdl, uint32_t size)
{
	uint32_t pmem_ctr;
	delcore30m_t			*dev = hdl;


	switch (size / DLCR30M_BANK_SIZE) {
	case 0:
	case 1:
		pmem_ctr = DLCR30M_PMCONF_1;
		break;
	case 2:
	case 3:
        return -ENOTSUP; //TODO: temporary not avail
		pmem_ctr = DLCR30M_PMCONF_3;
		break;
	case 4:
        return -ENOTSUP; //TODO: temporary not avail
		pmem_ctr = DLCR30M_PMCONF_4;
		break;
	default:
		return -EFAULT;
	}

	size = (pmem_ctr + 1) * DLCR30M_BANK_SIZE;
	/*
	 * TODO: Add a ban on changing the memory boundary if there is at least
	 *       one job in the queue.
	 */

	dsp_set_reg32(dev, DLCR30M_CSR, DLCR30M_CSR_PM_CONFIG(pmem_ctr));
	dev->pm_conf = pmem_ctr;

	return 0;
}

uint32_t elcore_core_read(void *hdl, /*void* data, void* offset*/uint32_t core_num, void* to,  void* offset, int *size)
{
	delcore30m_t		*dev = hdl;
// 	delcore30m_firmware *frw = (delcore30m_firmware*)data;
// 	dsp_core			*core = &dev->core[frw->cores];
// 	void*				to = (void*)frw->data;
// 	uint32_t			size = /*frw->size*/20; 
	dsp_core			*core = &dev->core[core_num];
    
    if ((uintptr_t)offset < DLCR30M_BANK_SIZE)
    {
        if ((uintptr_t)(offset + *size) > DLCR30M_BANK_SIZE)
          {
            if ((uintptr_t)(offset + *size) < (DLCR30M_BANK_SIZE * 5))
			{
			memcpy(to,(void*)(core->pram + (uintptr_t)offset), DLCR30M_BANK_SIZE - (uint32_t)offset);
			memcpy(to + (uintptr_t)(DLCR30M_BANK_SIZE - (uintptr_t)offset),
			core->xyram, *size - (uint32_t)(DLCR30M_BANK_SIZE - (uintptr_t)offset));

			}
			else
			{
				*size = -1;
                return 0;
			}
          }
        else
          {
            memcpy(to,core->pram + (uintptr_t)offset,*size);
          }
    }
    else
    {
        if ((uint32_t)((uintptr_t)offset - DLCR30M_BANK_SIZE + *size) < (DLCR30M_BANK_SIZE * 5)) 
		{
	        memcpy(to,core->xyram + (uintptr_t)(offset - DLCR30M_BANK_SIZE),*size);
		}
		else
		{
			*size = -1;
			return 0;
		}
    }
	printf("%s: %u bytes had been read from offset: %u\n", __func__, *size, (uint32_t)offset);
    
    
    return get_dsp_addr(hdl, (uint32_t)offset);
}

uint32_t  elcore_core_write(void *hdl, /*void* data, void* offset*/uint32_t core_num, void* from, void* offset, 
int 
*size)
{
	delcore30m_t		*dev = hdl;
// 	delcore30m_firmware *frw = (delcore30m_firmware*)data;
// 	dsp_core			*core = &dev->core[frw->cores];
// 	void*				from = (void*)frw->data;
// 	uint32_t			size = /*frw->size*/20;
	
	dsp_core			*core = &dev->core[core_num];
	
	if ((uintptr_t)offset < DLCR30M_BANK_SIZE)
	{

	     if ((uintptr_t)(offset + *size) > DLCR30M_BANK_SIZE) //need offset use XYRAM
	       {
			   if ((uintptr_t)(offset + *size) < (DLCR30M_BANK_SIZE * 5)) 
			   {	//writing from pram we need to leave one xyram bank?? yes - 4, no - 5
		         memcpy((void*)(core->pram + (uintptr_t)offset),from, DLCR30M_BANK_SIZE - (uintptr_t)offset);
		         memcpy(core->xyram,from + (uint64_t)(DLCR30M_BANK_SIZE - (uintptr_t)offset),
						*size - (uint32_t)(DLCR30M_BANK_SIZE - (uintptr_t)offset));
			   }
			   else
			   {
					*size = -1;
					return 0;
			   }
	       }
	     else
	       {
	         memcpy(core->pram + (uintptr_t)offset,from, *size);
	       }
	 }
	 else
	 {
	     if ((uint32_t)((uintptr_t)offset - DLCR30M_BANK_SIZE + *size) < (DLCR30M_BANK_SIZE * 5))
		 {
		     memcpy(core->xyram + (uint64_t)((uintptr_t)offset - DLCR30M_BANK_SIZE), from, *size);
		 }
		 else
		 {
			*size = -1;
			return 0;
		 }
	 }
	printf("%s: %u bytes had been written to offset: %u\n", __func__, *size, (uint32_t)offset);
	
    return get_dsp_addr(hdl, (uint32_t)offset);
}


int		elcore_set_pram(void *hdl, void *job)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	elcore_job_t *cur_job = (elcore_job_t*)job;
	uint32_t pram_size;
	
// 	pram_size = dsp_get_reg32(dev, DLCR30M_CSR);
	
	switch (/*(pram_size & DLCR30M_CSR_PM_CONFIG_MASK) >> 2*/dev->pm_conf)
	{
	case DLCR30M_PMCONF_1:
		pram_size = DLCR30M_BANK_SIZE;
		break;
	case DLCR30M_PMCONF_3:
        return -ENOTSUP; //TODO: temporary not avail
		pram_size = 3 * DLCR30M_BANK_SIZE;
		break;
	case DLCR30M_PMCONF_4:
        return -ENOTSUP; //TODO: temporary not avail
		pram_size = 4 * DLCR30M_BANK_SIZE;
		break;
	default:
		return -EINVAL;
	}
	if (cur_job->job_pub.code.size > pram_size)
	{
		printf("Firmware is too big\n");
		return -EINVAL;
	}
	
// 	memcpy(dev->core[firmware->cores].pram, firmware->data, firmware->size);
// 	elcore_core_write(dev,firmware->cores, firmware->data, 0, &firmware->size);
// 	
// 	if (firmware->size < 0)
// 	{
// 		return EINVAL;
// 	}
// 	
// 	elcore_core_write(dev, firmware, 0);
// 	
// 	dev->core[firmware->cores].fw_size = firmware->size;
// 	dev->core[firmware->cores].fw_ready = DLCR30M_FWREADY;
	
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
	
	return 0;
}

#define DCMD_CUSTOM	__DIOT (_DCMD_ELCORE, 228 + 5, int)

int elcore_ctl(void *hdl, int cmd, void *msg, int msglen, int *nbytes, int *info ) 
{ //for macroses, not defined in elcore-manager.h
	delcore30m_t			*dev = hdl;

	switch(cmd) {
		case DCMD_CUSTOM:
			printf("DCMD_CUSTOM\n");
			break;
		default:
			printf("asshole\n");
			return -1;
	}
	
	printf("%s: cmd: %d  msglen: %d  nbytes: %d\n", __func__, cmd, msglen, *nbytes );
	
    return 0;
}

// int elcore_job_status(void *hdl, uint32_t job_block) /*I need this func?*/
// {
// 	enum elcore_wait_job	block_type = job_block;
// 	
// 	switch (block_type)
// 	{
// 		case ELCORE_WAIT_BLOCK:
// 			break;
// 		case ELCORE_WAIT_NONBLOCK:	/* May be process this  */
// 			break;
// 		default:
// 			break;
// 	}
// }

int elcore_interrupt_thread(void *hdl)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	uint32_t				stopped_cores;
	uint32_t				val32, maskr_dsp;
	int						it;
	elcore_job_t			*cur_job;
	
	struct sigevent event;
	
	/* fill in "event" structure */
	memset(&event, 0, sizeof(event));
	event.sigev_notify = SIGEV_INTR;

	/* Obtain I/O privileges */
	if (ThreadCtl( _NTO_TCTL_IO, 0 ) < 0)
	{
		perror("ThreadCtl");
		return -1;
	}
	
	if ((dev->irq_hdl = InterruptAttachEvent( dev->irq, &event,0 )) < 0)
	{
		perror("InterruptAttachEvent");
		return -1;
	}
	
	
	
	while (1) {
		InterruptWait (NULL, NULL);
		/*  do something about the interrupt,
		 *  perhaps updating some shared
		 *  structures in the resource manager 
		 *
		 *  unmask the interrupt when done
		 */
		printf("%s: IRQ GET\n", __func__);
		
		printf("DLCR30M_QSTR: \t\t %08x\nDSP0_DCSR: \t\t %08x\n", dsp_get_reg32(dev, DLCR30M_QSTR), 
			dsp_get_reg32(&dev->core[0], DLCR30M_DSCR));

		//reset irq
        //check all cores
// 		dsp_set_reg32(&dev->core[0], DLCR30M_DSCR, 0x0);
        val32 = dsp_get_reg32(dev, DLCR30M_QSTR);
		
		stopped_cores = 0;
		for (it = 0; it < DLCR30M_MAX_CORES; ++it)
		{
			if (val32 & DLCR30M_QSTR_CORE_MASK(it))
			{
				stopped_cores |= 1 << it;
			}
		}

		if (!stopped_cores)
		{
			goto unmask;
		}
			
		maskr_dsp = dsp_get_reg32(dev, DLCR30M_MASKR); //запретить прерывания от тех, откуда пришли?
		maskr_dsp &= ~(val32 & DLCR30M_QSTR_MASK);
		dsp_set_reg32 (dev, DLCR30M_MASKR, maskr_dsp);
		
// 		/*wait time to test blocking*/
// 		delay(10000);
		
		for (it = 0; it < DLCR30M_MAX_CORES; ++it) //итерируемся по ядрам
		{
			if ( stopped_cores & (1 << it) )
			{
				cur_job = get_enqueued_by_id(&dev->drvhdl, dev->core[it].job_id);
				
				if (cur_job == NULL)
				{
					//BUG:TODO: what to do???
					goto unmask;
				}
				//TODO: process the result of job
				
				
				val32 = dsp_get_reg16(&dev->core[it], DLCR30M_DSCR);

				if (val32 & (DLCR30M_DSCR_PI | DLCR30M_DSCR_SE | DLCR30M_DSCR_BRK))
				{
					cur_job->job_pub.rc = ELCORE_JOB_ERROR;
				}
				else
				{
					cur_job->job_pub.rc = ELCORE_JOB_SUCCESS;
				}
				
				
				job_remove_from_queue(&dev->drvhdl, cur_job); //sets DELCORE30M_JOB_IDLE
				//TODO: where to release job? now by client
				elcore_reset_core(dev, it);
				
                //TODO: try to start core???
                
				val32 = dsp_get_reg32(dev, DLCR30M_CSR);
				val32 &= ~DLCR30M_CSR_SYNC_START;
				dsp_set_reg32(dev, DLCR30M_CSR, val32);

		        /*TODO: get the job from the list (queue) of jobs.*/
				if (cur_job->rcvid != 0) //if its nonzero, we must manually wake up client by rcvid
				{
					/*FIXME: size and msg are hardcoded now*/
					printf("%s: msgreply!\n", __func__);
					MsgReply(cur_job->rcvid, 0, &cur_job->job_pub.status, sizeof(uint32_t));
				}
			}
		}
		
		
	unmask:
		InterruptUnmask(dev->irq, dev->irq_hdl);
	}
	
}

uint32_t elcore_dmasend( void *hdl, uint32_t core_num, uint32_t from, uint32_t offset, int *size)
{
	delcore30m_t			*dev = hdl;
	struct sdma_exchange	sdma_task;
	dsp_core				*core = &dev->core[core_num];
	int						job_status;
	
	struct sdma_channel chnl_0 = {
		.rram = NULL,
		.id = 0
	};
	
// 	if (sdma_init())
// 	{
// 		perror("sdma_init failure");
// 		goto exit0;
// 	}
	
	if (offset < DLCR30M_BANK_SIZE)
	{

			if (offset + *size > DLCR30M_BANK_SIZE) //need offset use XYRAM
			{
			   if (offset + *size < (DLCR30M_BANK_SIZE * 5)) 
			   {	//writing from pram we need to leave one xyram bank?? yes - 4, no - 5
				sdma_task.from = from;
				sdma_task.to = core->pram_phys + offset;
				sdma_task.channel = &chnl_0;
				sdma_task.size = DLCR30M_BANK_SIZE - offset;
				sdma_task.iterations = 1;
				
				if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
				{
					printf("Job prepare error: %s\n", strerror(-job_status));
					goto exit0;
				}

				if ((job_status = sdma_transfer(&sdma_task)) != 0 )
				{
				    printf("Job runing error: %s\n", strerror(-job_status));
					goto exit1;
				}
				
				sdma_release_task(&sdma_task);
				
				sdma_task.from = from + (uint64_t)(DLCR30M_BANK_SIZE - offset);
				sdma_task.to = core->xyram_phys;
				sdma_task.channel = &chnl_0;
				sdma_task.size = *size - (DLCR30M_BANK_SIZE - offset);
				sdma_task.iterations = 1;
				
				if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
				{
					printf("Job prepare error: %s\n", strerror(-job_status));
					goto exit0;
				}

				if ((job_status = sdma_transfer(&sdma_task)) != 0 )
				{
				    printf("Job runing error: %s\n", strerror(-job_status));
					goto exit1;
				}
				
				
			   }
			   else
			   {
					goto exit0;
			   }
			}
			else
			{ //send with one block
				sdma_task.from = from;
				sdma_task.to = core->pram_phys + offset;
				sdma_task.channel = &chnl_0;
				sdma_task.size = *size;
				sdma_task.iterations = 1;
				
				if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
				{
					printf("Job prepare error: %s\n", strerror(-job_status));
					goto exit0;
				}

				if ((job_status = sdma_transfer(&sdma_task)) != 0 )
				{
				    printf("Job runing error: %s\n", strerror(-job_status));
					goto exit1;
				}
				
	       }
	 }
	 else
	 { //put to XYRAM
	     if ((offset - DLCR30M_BANK_SIZE + *size) < (DLCR30M_BANK_SIZE * 5))
		 {
			sdma_task.from = from;
			sdma_task.to = core->xyram_phys + (uint64_t)(offset - DLCR30M_BANK_SIZE);
			sdma_task.channel = &chnl_0;
			sdma_task.size = *size;
			sdma_task.iterations = 1;
			
			if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
			{
				printf("Job prepare error: %s\n", strerror(-job_status));
				goto exit0;
			}

			if ((job_status = sdma_transfer(&sdma_task)) != 0 )
			{
			    printf("Job runing error: %s\n", strerror(-job_status));
				goto exit1;
			}
				
		 }
		 else
		 {
			goto exit0;
		 }
	 }
 
	printf("%s: %u bytes had been written to offset: %u\n", __func__, *size, offset);
	sdma_mem_dump(core->pram + offset, 40);
	
	sdma_release_task(&sdma_task);

	return get_dsp_addr(hdl, offset);
	
exit1:
	
	sdma_release_task(&sdma_task);
exit0:
	*size = -1;

	return 0;
}

uint32_t elcore_dmarecv(void *hdl, uint32_t core_num, uint32_t to,  uint32_t offset, int *size)
{
	delcore30m_t			*dev = hdl;
	struct sdma_exchange	sdma_task;
	dsp_core				*core = &dev->core[core_num];
	int						job_status;
	
	struct sdma_channel chnl_0 = {
		.rram = NULL,
		.id = 0
	};
	
// 	if (sdma_init())
// 	{
// 		perror("sdma_init failure");
// 		goto exit0;
// 	}
	
    if (offset < DLCR30M_BANK_SIZE)
    {
        if ((offset + *size) > DLCR30M_BANK_SIZE)
          {
            if ((offset + *size) < (DLCR30M_BANK_SIZE * 5))
			{
				sdma_task.from = core->pram_phys + offset;
				sdma_task.to = to;
				sdma_task.channel = &chnl_0;
				sdma_task.size = DLCR30M_BANK_SIZE - offset;
				sdma_task.iterations = 1;
				
				if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
				{
					printf("Job prepare error: %s\n", strerror(-job_status));
					goto exit0;
				}

				if ((job_status = sdma_transfer(&sdma_task)) != 0 )
				{
				    printf("Job runing error: %s\n", strerror(-job_status));
					goto exit1;
				}
				
				sdma_release_task(&sdma_task);
			
				sdma_task.from = core->xyram_phys;
				sdma_task.to = to + (DLCR30M_BANK_SIZE - offset);
				sdma_task.channel = &chnl_0;
				sdma_task.size = *size - (DLCR30M_BANK_SIZE - offset);
				sdma_task.iterations = 1;
				
				if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
				{
					printf("Job prepare error: %s\n", strerror(-job_status));
					goto exit0;
				}

				if ((job_status = sdma_transfer(&sdma_task)) != 0 )
				{
				    printf("Job runing error: %s\n", strerror(-job_status));
					goto exit1;
				}
				
			}
			else
			{
				goto exit0;
			}
		}
		else
		{
			sdma_task.from = core->pram_phys + offset;
			sdma_task.to = to;
			sdma_task.channel = &chnl_0;
			sdma_task.size = *size;
			sdma_task.iterations = 1;

			if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
			{
				printf("Job prepare error: %s\n", strerror(-job_status));
				goto exit0;
			}

			if ((job_status = sdma_transfer(&sdma_task)) != 0 )
			{
			    printf("Job runing error: %s\n", strerror(-job_status));
				goto exit1;
			}

		}
	}
	else
	{
        if ((offset - DLCR30M_BANK_SIZE + *size) < (DLCR30M_BANK_SIZE * 5)) 
		{
			sdma_task.from = core->xyram_phys + (uint64_t)(offset - DLCR30M_BANK_SIZE);
			sdma_task.to = to;
			sdma_task.channel = &chnl_0;
			sdma_task.size = *size;
			sdma_task.iterations = 1;
			
			if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
			{
				printf("Job prepare error: %s\n", strerror(-job_status));
				goto exit0;
			}

			if ((job_status = sdma_transfer(&sdma_task)) != 0 )
			{
			    printf("Job runing error: %s\n", strerror(-job_status));
				goto exit1;
			}
				
		}
		else
		{
			goto exit0;
		}
    }
	printf("%s: %u bytes had been read from offset: %u\n", __func__, *size, offset);
	sdma_mem_dump(core->pram + offset, 40);
	
	sdma_release_task(&sdma_task);

	return get_dsp_addr(hdl, offset);
	
exit1:
	
	sdma_release_task(&sdma_task);
exit0:
	*size = -1;

	return 0;
}

void elcore_func_fini(void *hdl)
{
    delcore30m_t			*dev = hdl;

    InterruptDetach(dev->irq_hdl);
    
	elcore_job_hdl_fini(&dev->drvhdl);
	
    munmap_device_memory(dev->base ,DLCR30M_SIZE);
    free(hdl);
    
    sdma_fini();
	
    printf("%s: success\n", __func__);
}
