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
//offset - number of bytes from the begining of PRAM of core
static uint32_t get_dsp_addr(delcore30m_t *dev, uint32_t offset, uint8_t core)
{
	printf("%s: entry\n", __func__);
	uint32_t				dsp_addr;
	
	//now supported only one block for pram and four for xyram
	uint32_t pram_size = ( dev->pm_conf + 1) * DLCR30M_BANK_SIZE;
	

	if (offset >= pram_size)
	{
		//XYRAM adrrs per word, starting with 0x0000 for all cores
		//for 2nd core xyram addrs starts with 0x8000
		dsp_addr = addr2delcore30m(offset - pram_size) + (addr2delcore30m(DLCR30M_BANK_SIZE * 4) * core);
	}
	else
	{  //PRAM
		dsp_addr = addr2delcore30m(offset);
	}

	return dsp_addr;
}

int dsp_core_print_regs(dsp_core* core)
{
	int iter = 0;
	printf("\tDump regs %d core:\n",core->id);
	printf("\tDSCR:       %4.8X\n",dsp_get_reg16(core, DLCR30M_DSCR));
// 	printf("\tSR:         %4.8X\n",dsp_get_reg16(core, DLCR30M_SR));
// 	printf("\tIDR:        %4.8X\n",dsp_get_reg16(core, DLCR30M_IDR));
// 	printf("\tEFR:        %4.8X\n",dsp_get_reg32(core, DLCR30M_EFR));
// 	printf("\tDSTART:     %4.8X\n",dsp_get_reg32(core, DLCR30M_DSTART));
// 	printf("\tIRQR:       %4.8X\n",dsp_get_reg32(core, DLCR30M_IRQR));
// 	printf("\tIMASKR:     %4.8X\n",dsp_get_reg32(core, DLCR30M_IMASKR));
// 	printf("\tTMR:        %4.8X\n",dsp_get_reg32(core, DLCR30M_TMR));
// 	printf("\tARBR:       %4.8X\n",dsp_get_reg16(core, DLCR30M_ARBR));
	printf("\tPC:         %4.8X\n",dsp_get_reg16(core, DLCR30M_PC));
// 	printf("\tSS:         %4.8X\n",dsp_get_reg16(core, DLCR30M_SS));
// 	printf("\tLA:         %4.8X\n",dsp_get_reg16(core, DLCR30M_LA));
// 	printf("\tCSL:        %4.8X\n",dsp_get_reg16(core, DLCR30M_CSL));
// 	printf("\tLC:         %4.8X\n",dsp_get_reg16(core, DLCR30M_LC));
// 	printf("\tCSH:        %4.8X\n",dsp_get_reg16(core, DLCR30M_CSH));
// 	printf("\tSP:         %4.8X\n",dsp_get_reg16(core, DLCR30M_SP));
// 	printf("\tSAR:        %4.8X\n",dsp_get_reg16(core, DLCR30M_SAR0));
// 	printf("\tCNTR:       %4.8X\n",dsp_get_reg16(core, DLCR30M_CNTR));
	/*for (i=0;i<6;i++)
	printf("\tSAR%d       %4.8X\n",i,dsp_get_reg16(core,DLCR30M_dbSAR(i)));*/
// 	printf("\tCCR:        %4.8X\n",dsp_get_reg16(core,DLCR30M_CCR));
// 	printf("\tPDNR:       %4.8X\n",dsp_get_reg16(core,DLCR30M_PDNR));
// 	printf("\tSFR:        %4.8X\n",dsp_get_reg32(core,DLCR30M_SFR));
// 	printf("\tIVAR:       %4.8X\n",dsp_get_reg16(core,DLCR30M_IVAR));
// 	printf("\tdbPCa:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCa));
// 	printf("\tdbPCf:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCf));
// 	printf("\tdbPCd:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCd));
// 	printf("\tdbPCe:      %4.8X\n",dsp_get_reg16(core,DLCR30M_dbPCe));
// 	printf("\tdbSAR0:     %4.8X\n",dsp_get_reg16(core,DLCR30M_dbSAR0));
// 	printf("\tdbCNTR:     %4.8X\n",dsp_get_reg16(core,DLCR30M_dbCNTR));
// 	printf("\tCNT_RUN:    %4.8X\n",dsp_get_reg32(core, DLCR30M_Cnt_RUN));
	/*for (i=0;i<6;i++)
	printf("\tdbSAR%d       %4.8X\n",i,dsp_get_reg16(core,DLCR30M_dbSAR(i)));*/
    
   
	for ( ; iter <= 31; iter++)
	{
		printf("\tR%u:\t\t%4.8X\n", iter, dsp_get_reg32(core,
	                                                (!(iter % 2)? DLCR30M_R2L(iter):DLCR30M_R1L(iter))));
	}
	
		for (iter = 0; iter <= 7; iter++) 
	{
		printf("\tA%u:\t\t%4.8X\n", iter, dsp_get_reg32(core, DLCR30M_A(iter)));
	}
    
// 	sdma_mem_dump(core->xyram, 40);


    
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
	{
	    dsp_core_print_regs(&(dsp->core[i]));
	}
	
	sdma_print_regs(2);
	
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
	//FIXME: need these two core count identifications???
	dev->core_count = DLCR30M_MAX_CORES;
	dev->dma_count = SDMA_MAX_CHANNELS;
	dev->irq = DLCR30M_IRQ_NUM;
	dev->pbase = DLCR30M_BASE;
	//set defaults
	DLCR30M_SET_MEM_PARTS(&dev->core[0], 1);
	DLCR30M_SET_MEM_PARTS(&dev->core[1], 1);
	
	
	if (elcore_parce_opts(dev, options) != EOK)
	{
        //BUG: later this thread don't kill the main thread
		free(dev);
        
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
	//put stacks to the end of XYRAM
	dev->core[0].stack = dev->base + DLCR30M_DSP0_XYRAM + 4 * DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
	dev->core[0].stack_offset = 5 * DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
	dev->core[1].stack = dev->base + DLCR30M_DSP1_XYRAM + 4 * DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
	dev->core[1].stack_offset = 5 * DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
    
	dev->core[0].regs = dev->base + DLCR30M_DSP0_REGS;
	dev->core[1].regs = dev->base + DLCR30M_DSP1_REGS;

	
	dev->pm_conf = (dsp_get_reg32(dev, DLCR30M_CSR) & DLCR30M_CSR_PM_CONFIG_MASK) >> 2;
	
	if (sdma_init(dev->dma_count))
	{
		perror("sdma_init failure");
		elcore_func_fini(dev);
        
		return NULL;
	}
	
	for (it = 0; it < dev->dma_count; it++)
	{
		dev->sdma[it].busy = 0;
		dev->sdma[it].id = it;
	}
	
	dsp_cluster_print_regs(dev);
	
	return dev;
    
}

// int		elcore_wait_irq

#if 1

static int elcore_setup_sdma(delcore30m_t *dev, elcore_job_t *cur_job)
{
	printf("%s: entry\n", __func__);
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];
	uint32_t				qmaskr0_val;
	int						sdma_channel, i;

	for (i = 0; i < cur_job->sdma_chaincount; i++)
	{
		if (dev->sdma[cur_job->sdma_chains[i].chain_pub.channel].busy)
		{
			printf("%s: channel %d is busy\n", __func__, i);
			
			return EBUSY;
		}
	}
	
	//разрешаем внешние запросы на прерывания от группы QST0 в DSP
	dsp_set_reg32(cur_core, DLCR30M_IMASKR, (1 << 30));
	
	//разрешить прерывание от n-го канала SDMA в DSP
	qmaskr0_val = dsp_get_reg32(cur_core, DLCR30M_QMASKR(0));
	
	for (i = 0; i < cur_job->sdma_chaincount; i++)
	{
		sdma_channel = cur_job->sdma_chains[i].chain_pub.channel;
		qmaskr0_val |= 1 << (8 + sdma_channel );
		
		dev->sdma[sdma_channel].busy = 1;
	}
	
	dsp_set_reg32(cur_core, DLCR30M_QMASKR(0), qmaskr0_val);
	
	/* FIXME: interrupt handler address */
	/*
	* Irq handler distination is set in crt0 file
	*/
	
	dsp_set_reg32(cur_core, DLCR30M_IVAR, cur_job->code_dspaddr + addr2delcore30m(0x0C));
	
	return EOK;
}

static void elcore_release_sdma(delcore30m_t *dev, elcore_job_t *cur_job)
{
	printf("%s: entry\n", __func__);
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];
	uint32_t				qmaskr0_val;
	int						sdma_channel, i;


    //запрещаем прерывание от n-го канала SDMA в DSP
	qmaskr0_val = dsp_get_reg32(cur_core, DLCR30M_QMASKR(0));
	
	for (i = 0; i < cur_job->sdma_chaincount; i++)
	{
		sdma_channel = cur_job->sdma_chains[i].chain_pub.channel;
		qmaskr0_val &= ~(1 << (8 + sdma_channel ));
		
		dev->sdma[sdma_channel].busy = 0;
	}
	
	dsp_set_reg32(cur_core, DLCR30M_QMASKR(0), qmaskr0_val);
	/*
    *запрещаем внешние запросы на прерывания от группы QST0 в DSP, если нет активных задач,
	* использующих sdma
	*/
	int it = 0;
	for (;it < dev->dma_count; it++)
	{
		if (dev->sdma[it].busy)
		{
			return;
		}
	}
	dsp_set_reg32(cur_core, DLCR30M_IMASKR, 0);
    
}

#endif

#define DELCORE30M_ARGS_INREG		3

static uint32_t elcore_set_args(delcore30m_t *dev, elcore_job_t *cur_job)
{
	printf("%s: entry\n", __func__);
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];


	int instack = 0, i, i_num, o_num;
	uint32_t	dsp_addr;
	
	i_num = cur_job->job_pub.inum;
	o_num = cur_job->job_pub.onum;
	
	int inregs = i_num + o_num;
	
	if (inregs > DELCORE30M_ARGS_INREG)
	{
	    instack = inregs - DELCORE30M_ARGS_INREG;
		inregs = DELCORE30M_ARGS_INREG;
	}
	//get stack pointer from the end, stack rises to lower adresses
	uint64_t *stack = (uint64_t *)cur_core->stack - instack + DLCR30M_STACK_SIZE / sizeof(uint64_t);


	
	for (i = 0; i < inregs; ++i)
	{
		dsp_addr = i < i_num ? cur_job->input_dspaddr[i] : cur_job->output_dspaddr[i - i_num];
		//BUG: compiler does bit move itself, we need to move back after get_dsp_addr
		dsp_addr <<= 2;
		
		printf("%s: set R%u to 0x%08x  \n", __func__, i * 2 , dsp_addr );
		dsp_set_reg32(cur_core, DLCR30M_R2L(i * 2), dsp_addr);
	}
	
	for (i = DELCORE30M_ARGS_INREG; i < i_num + o_num; ++i)
	{
		dsp_addr = i < i_num ? cur_job->input_dspaddr[i] : cur_job->output_dspaddr[i - i_num];
		
		//BUG: compiler does bit move itself, we need to move back after get_dsp_addr
		dsp_addr <<= 2;
		
		printf("%s: set stack[%u] to 0x%08x  \n", __func__, i - DELCORE30M_ARGS_INREG  , dsp_addr );
		stack[i - DELCORE30M_ARGS_INREG] = dsp_addr;
	}

	printf("%s: got stack offset: 0x%08x  \n", __func__ , (uint32_t)((uint8_t *)stack - (uint8_t *)cur_core->stack) );
	//BUG: this seems not very beautiful
	if (instack > 0)
	{
		return (uint32_t)((uint8_t *)stack - (uint8_t *)cur_core->stack);
	}
	else
	{	//-1 need not to get 0x10000 stack_dspaddr?
		return DLCR30M_STACK_SIZE - 1;
		
	}
}

int		elcore_start_core(void *hdl, uint32_t core_num)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	dsp_core				*core = &dev->core[core_num];
	uint32_t				val32, stack_offset, stack_dspaddr;
	int						rc, i;

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
	
	if (cur_job->sdma_chaincount > 0 )
	{
		if ((rc = elcore_setup_sdma(dev, cur_job)) != EOK)
		{
			return rc;
		}
	}

	stack_offset = core->stack_offset + elcore_set_args(dev, cur_job);
	
	stack_dspaddr = get_dsp_addr(dev, stack_offset, core_num);
	
	printf("%s: Got stack offset: 0x%08x stack_dspaddr: 0x%08x\n", __func__, stack_offset, stack_dspaddr);
	
	dsp_set_reg32(core, DLCR30M_A(6), stack_dspaddr); //frame pointer
	dsp_set_reg32(core, DLCR30M_A(7), stack_dspaddr); //core stack pointer
	
	dsp_set_reg16(core, DLCR30M_PC,cur_job->code_dspaddr); //start of prog

	//enable interrupts
	val32 = dsp_get_reg32(dev, DLCR30M_MASKR);
	val32 |= DLCR30M_QSTR_CORE_MASK(core_num);
	dsp_set_reg32(dev, DLCR30M_MASKR, val32);

	cur_job->job_pub.status = ELCORE_JOB_RUNNING;
	core->job_id = cur_job->job_pub.id; 
	
	
	dsp_set_reg16(core,DLCR30M_DSCR,dsp_get_reg16(core,DLCR30M_DSCR) | DLCR30M_DSCR_RUN);
	
	//start sdma transfers
	for (i = 0; i < cur_job->sdma_chaincount; i++)
	{
		sdma_transfer(&cur_job->sdma_chains[i]);
	}
	
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
	 *	   one job in the queue.
	 */

	dsp_set_reg32(dev, DLCR30M_CSR, DLCR30M_CSR_PM_CONFIG(pmem_ctr));
	dev->pm_conf = pmem_ctr;

	return 0;
}

uint32_t elcore_core_read(void *hdl, uint32_t core_num, void* to,  void* offset, int *size)
{
	delcore30m_t		*dev = hdl;
	dsp_core			*core = &dev->core[core_num];
    
	if ((uintptr_t)offset < DLCR30M_BANK_SIZE)// 	delcore30m_firmware *frw = (delcore30m_firmware*)data;
// 	dsp_core			*core = &dev->core[frw->cores];
// 	void*				from = (void*)frw->data;
// 	uint32_t			size = /*frw->size*/20;
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
    
    
	return get_dsp_addr(hdl, (uint32_t)offset, core_num);
}

uint32_t  elcore_core_write(void *hdl, uint32_t core_num, void* from, void* offset, 
int 
*size)
{
	delcore30m_t		*dev = hdl;
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
	
	return get_dsp_addr(hdl, (uint32_t)offset, core_num);
}


int		elcore_set_prog( void *hdl, void *job)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	elcore_job_t			*cur_job = (elcore_job_t*)job;
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];
	uint32_t pram_size, mem_parts;
	int		part_n, code_send_size;
    
	mem_parts = DLCR30M_GET_MEM_PARTS(cur_core);
    
	printf("%s: mem_parts for core %u: %u\n", __func__, cur_job->job_pub.core, mem_parts);
    
	for ( part_n = 1; part_n <= mem_parts; part_n++) //find first free mem part
	{
		if ( !DLCR30M_CHECK_MEM_PART(cur_core, part_n) )
		{
			break;
		}
	}
	
	if (part_n > mem_parts) //no vacant mem parts
	{
		return EBUSY;
	}
	
	
// 	pram_size = dsp_get_reg32(dev, DLCR30M_CSR);
	
	switch (/*(pram_size & DLCR30M_CSR_PM_CONFIG_MASK) >> 2*/dev->pm_conf)
	{
	case DLCR30M_PMCONF_1:
		pram_size = DLCR30M_BANK_SIZE;
		break;
	case DLCR30M_PMCONF_3:
		return ENOTSUP; //TODO: temporary not avail
		pram_size = 3 * DLCR30M_BANK_SIZE;
		break;
	case DLCR30M_PMCONF_4:
		return ENOTSUP; //TODO: temporary not avail
		pram_size = 4 * DLCR30M_BANK_SIZE;
		break;
	default:
		return EINVAL;
	}

	code_send_size = cur_job->job_pub.code.size;
	pram_size = pram_size / mem_parts; //mem per one block
	
	if (code_send_size > pram_size)
	{
		printf("Firmware is too big\n");
		return ENOMEM;
	}
	
	if (1)
	{	//use dma
		cur_job->code_dspaddr = elcore_dmasend(dev, cur_job->job_pub.core, cur_job->job_pub.code.client_paddr,
										(part_n - 1) * pram_size, &code_send_size);
	}
	else
	{	//not use dma, need to mmap paddr
// 		cur_job->code_dspaddr = elcore_core_write(dev, cur_job->job_pub.core, cur_job->job_pub.code.client_paddr,
// 										(part_n - 1) * pram_size, &code_send_size);
	}
	
	
	if (code_send_size < 0)
	{
		return EINVAL;
	}
	
	cur_job->code_cpuaddr = cur_core->pram_phys + (part_n - 1) * pram_size;
	cur_job->mem_part = part_n;
	
	return EOK;
}

int		elcore_set_data( void *hdl, void *job)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	elcore_job_t			*cur_job = (elcore_job_t*)job;
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];
	uint32_t  xyram_size, mem_parts, xy_offset;
	int		part_n, i, data_send_size;
	
	mem_parts = DLCR30M_GET_MEM_PARTS(cur_core);
	part_n = cur_job->mem_part;
	
	printf("%s: mem_parts for core %u: %u\n", __func__, cur_job->job_pub.core, mem_parts);
   
	
	if (part_n > mem_parts && part_n == DLCR30M_NO_MEM_PARTS) //call set pram first
	{
		return EBUSY;
	}
	
	switch (dev->pm_conf)
	{//mem for stack lies at the end of block
	case DLCR30M_PMCONF_1:
		xyram_size = 4 * DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
		break;
	case DLCR30M_PMCONF_3:
		return ENOTSUP; //TODO: temporary not avail
		xyram_size = 2 * DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
		break;
	case DLCR30M_PMCONF_4:
		return ENOTSUP; //TODO: temporary not avail
		xyram_size = DLCR30M_BANK_SIZE - DLCR30M_STACK_SIZE;
		break;
	default:
		return EINVAL;
	}

	xyram_size = xyram_size / mem_parts; //mem per one block
	
	for (i = 0, data_send_size = 0; i < cur_job->job_pub.inum; i++)
	{
		data_send_size += cur_job->job_pub.input[i].size;
	}
	
	for (i = 0; i < cur_job->job_pub.onum ; i++)
	{
		data_send_size += cur_job->job_pub.output[i].size;
	}
	
	if (data_send_size > xyram_size)
	{
		printf("Firmware is too big\n");
		return ENOMEM;
	}
	//starting offset for first data block
	xy_offset = DLCR30M_BANK_SIZE + (part_n - 1) * xyram_size; 
	
	for (i = 0  ; i < cur_job->job_pub.inum; i++)
	{
		data_send_size = cur_job->job_pub.input[i].size;
		
		if (1)
		{	//use dma
			cur_job->input_dspaddr[i] = elcore_dmasend(dev, cur_job->job_pub.core,
				cur_job->job_pub.input[i].client_paddr, xy_offset, &data_send_size);
			
			printf("%s: input_dspaddr 0x%04x\n", __func__, cur_job->input_dspaddr[i]);
		}
		else
		{	//not use dma, need to mmap paddr
// 			cur_job->input_dspaddr[i] = elcore_core_write(dev, cur_job->job_pub.core,
// 				cur_job->job_pub.input[i].client_paddr, xy_offset, &data_send_size);
		}
		
		if (data_send_size < 0)
		{
			return EINVAL;
		}
		cur_job->input_cpupaddr[i] = cur_core->xyram_phys + xy_offset;
		xy_offset += data_send_size;
		
	}
	
	//TODO: NEED THE SAME CYCLE FOR OUTPUT?
	for (i = 0  ; i < cur_job->job_pub.onum; i++)
	{
		data_send_size = cur_job->job_pub.output[i].size;
		
		cur_job->output_dspaddr[i] = get_dsp_addr(dev, xy_offset, cur_job->job_pub.core);
		cur_job->output_cpupaddr[i] = cur_core->xyram_phys + xy_offset - DLCR30M_BANK_SIZE;
		
		printf("%s: output_dspaddr 0x%04x\n", __func__, cur_job->output_dspaddr[i]);
		
		xy_offset += data_send_size;
		
	}
	
	
	printf("%s: sum size: %u\n", __func__, xy_offset - DLCR30M_BANK_SIZE);
	//set part busy at mem_config
	DLCR30M_SET_PART_BUSY(cur_core, part_n);
	
	return EOK;
}

int			elcore_get_data( void *hdl, void *job)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	elcore_job_t			*cur_job = (elcore_job_t*)job;
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];
	int						i, data_read_size;
	uint32_t				xy_offset;
	
	
	for (i = 0  ; i < cur_job->job_pub.onum; i++)
	{
		data_read_size = cur_job->job_pub.output[i].size;
		xy_offset = cur_job->output_cpupaddr[i] - cur_core->xyram_phys + DLCR30M_BANK_SIZE;
		
		if (1)
		{//using dma
			elcore_dmarecv(dev, cur_job->job_pub.core,
				cur_job->job_pub.output[i].client_paddr, xy_offset, &data_read_size);
		}
		else
		{//no dma
			
		}
		
		if (data_read_size < 0)
		{
			return EINVAL;
		}
	}
	return EOK;
}

int			release_mem(void *hdl, void *job)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	elcore_job_t			*cur_job = (elcore_job_t*)job;
	dsp_core				*cur_core = &dev->core[cur_job->job_pub.core];
	
	DLCR30M_SET_PART_FREE(cur_core, cur_job->mem_part);
	cur_job->mem_part = DLCR30M_NO_MEM_PARTS;
	
	int it;
	
	for (it = 0; it < cur_job->sdma_chaincount; it++)
	{
		free(cur_job->sdma_chains[it].sdma_chain);
	}
	
	cur_job->sdma_chaincount = 0;
	
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
	
	for (iter = 0; iter <= 7; iter++) //???????????? WHY <= 7
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
		printf("%s: DLCR30M_QSTR: %4.8X\n", __func__, val32);
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
			
		maskr_dsp = dsp_get_reg32(dev, DLCR30M_MASKR); //mask the src of interrupts
		maskr_dsp &= ~(val32 & DLCR30M_QSTR_MASK);
		dsp_set_reg32 (dev, DLCR30M_MASKR, maskr_dsp);
		
// 		/*wait time to test blocking*/
// 		delay(10000);
		
		for (it = 0; it < DLCR30M_MAX_CORES; ++it)
		{
			if ( stopped_cores & (1 << it) )
			{	
				//get the runnung job on core
				cur_job = get_enqueued_by_id(&dev->drvhdl, dev->core[it].job_id);
				
				if (cur_job == NULL)
				{
					//BUG:TODO: what to do???
					goto unmask;
				}
				
				
				val32 = dsp_get_reg16(&dev->core[it], DLCR30M_DSCR);
				printf("%s: core[%d] DCSR: %4.8X\n", __func__, it, val32);
				
				//TODO: process the result of job
				
				if (val32 & (DLCR30M_DSCR_PI | DLCR30M_DSCR_SE | DLCR30M_DSCR_BRK))
				{
					cur_job->job_pub.rc = ELCORE_JOB_ERROR;
				}
				else
				{
					cur_job->job_pub.rc = ELCORE_JOB_SUCCESS;
				}
				
				
				job_remove_from_queue(&dev->drvhdl, cur_job); //sets DELCORE30M_JOB_IDLE
				//release sdma chains
				if (cur_job->sdma_chaincount > 0)
				{
					elcore_release_sdma(dev, cur_job);
				}
				
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
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	struct sdma_exchange	sdma_task;
	dsp_core				*core = &dev->core[core_num];
	int						job_status, it;
    
	//search vacant channel
	struct sdma_channel *chnl;

	for (it = 0; it < dev->dma_count; it++)
	{
		if (!dev->sdma[it].busy)
		{
			chnl = &dev->sdma[it];
			break;
		}
	}
	
	if (it == dev->dma_count)
	{
		printf("No vacant sdma channels\n");
		errno = EBUSY;
		goto exit0; 
	}
	
    struct sdma_descriptor sdma_package = {
	.f_off = 0, //offset from sdma_exchange->from
	.t_off = 0,
	.iter = 1 // количество повторов отправки данного пакета от 1 до 255, 0 - повторять бесконечно
    };
	
	sdma_task.chain_pub.channel = chnl->id;
	sdma_task.type = SDMA_CPU_; //CPU handles interrupts
	sdma_task.sdma_chain = &sdma_package;
	sdma_task.chain_pub.chain_size = 1; //only one package
	
	printf("%s: channel: %d\n", __func__, sdma_task.chain_pub.channel);
	
	if (offset < DLCR30M_BANK_SIZE)
	{

			if (offset + *size > DLCR30M_BANK_SIZE) //need offset use XYRAM
			{
			   if (offset + *size < (DLCR30M_BANK_SIZE * 5)) 
			   {	//writing from pram we need to leave one xyram bank?? yes - 4, no - 5
				sdma_task.chain_pub.from = from;
				sdma_task.chain_pub.to = core->pram_phys + offset;
				sdma_package.size = DLCR30M_BANK_SIZE - offset;


				
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
				
				sdma_task.chain_pub.from = from + (uint64_t)(DLCR30M_BANK_SIZE - offset);
				sdma_task.chain_pub.to = core->xyram_phys;
				sdma_package.size = *size - (DLCR30M_BANK_SIZE - offset);
				
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
				sdma_task.chain_pub.from = from;
				sdma_task.chain_pub.to = core->pram_phys + offset;
				sdma_package.size = *size;
				
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
			sdma_task.chain_pub.from = from;
			sdma_task.chain_pub.to = core->xyram_phys + (uint64_t)(offset - DLCR30M_BANK_SIZE);
			sdma_package.size = *size;
			
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
// 	sdma_mem_dump(core->pram + offset, 40);
	
	sdma_release_task(&sdma_task);

	return get_dsp_addr(hdl, offset, core_num);
	
exit1:
	
	sdma_release_task(&sdma_task);
exit0:
	*size = -1;

	return 0;
}

uint32_t elcore_dmarecv(void *hdl, uint32_t core_num, uint32_t to,  uint32_t offset, int *size)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	struct sdma_exchange	sdma_task;
	dsp_core				*core = &dev->core[core_num];
	int						job_status, it;
	
	//search vacant channel
	struct sdma_channel *chnl;

	for (it = 0; it < dev->dma_count; it++)
	{
		if (!dev->sdma[it].busy)
		{
			chnl = &dev->sdma[it];
			break;
		}
	}

	if (it == dev->dma_count)
	{
		printf("No vacant sdma channels\n");
		errno = EBUSY;
		goto exit0; 
	}
	

	struct sdma_descriptor sdma_package = {
	.f_off = 0, //offset from sdma_exchange->from
	.t_off = 0,
	.iter = 1 // количество повторов отправки данного пакета от 1 до 255, 0 - повторять бесконечно
    };
	
	sdma_task.chain_pub.channel = chnl->id;
	sdma_task.type = SDMA_CPU_; //CPU handles interrupts
	sdma_task.sdma_chain = &sdma_package;
	sdma_task.chain_pub.chain_size = 1; //only one package
	
	
	if (offset < DLCR30M_BANK_SIZE)
	{
		if ((offset + *size) > DLCR30M_BANK_SIZE)
		{
			if ((offset + *size) < (DLCR30M_BANK_SIZE * 5))
			{
				sdma_task.chain_pub.from = core->pram_phys + offset;
				sdma_task.chain_pub.to = to;
				sdma_package.size = DLCR30M_BANK_SIZE - offset;

				
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
			
				sdma_task.chain_pub.from = core->xyram_phys;
				sdma_task.chain_pub.to = to + (DLCR30M_BANK_SIZE - offset);
				sdma_package.size = *size - (DLCR30M_BANK_SIZE - offset);

				
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
			sdma_task.chain_pub.from = core->pram_phys + offset;
			sdma_task.chain_pub.to = to;
			sdma_package.size = *size;

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
			sdma_task.chain_pub.from = core->xyram_phys + (uint64_t)(offset - DLCR30M_BANK_SIZE);
			sdma_task.chain_pub.to = to;
			sdma_package.size = *size;
			
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
// 	sdma_mem_dump(core->pram + offset, 40);
	
	sdma_release_task(&sdma_task);

	return get_dsp_addr(hdl, offset, core_num);
	
exit1:
	
	sdma_release_task(&sdma_task);
exit0:
	*size = -1;

	return 0;
}

int			elcore_setup_dmachain(void *hdl, void *chain)
{
	printf("%s: entry\n", __func__);
	delcore30m_t			*dev = hdl;
	int						job_status;
	
	sdma_exchange_t			*cur_chain = (sdma_exchange_t*)chain;
	
	cur_chain->type = SDMA_DSP_;
	
	if ((job_status = sdma_prepare_task(cur_chain)) != 0 )
	{
		printf("Job prepare error: %s\n", strerror(-job_status));
		return -1;
	}
	
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
