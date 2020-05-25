#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sdma.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <pthread.h>

/*
*TODO: check the vacancy of the channel
* may be get the map of free channels?
*/

typedef struct sdma_dev {
	uintptr_t	vbase;
	int			irq_num[SDMA_MAX_CHANNELS];
	int			irq_hdl[SDMA_MAX_CHANNELS];
	uint32_t	chnl_num;
	uintptr_t	spinlock;
} sdma_dev_t;

sdma_dev_t sdma;
struct sigevent sdma_event;

#define U16_MAX							0xFFFF

#define sdma_read32(offset)				in32(sdma.vbase + offset)
#define sdma_write32(offset, value)		out32(sdma.vbase + offset, value)

// #define _SDMA_USE_SPINLOCK

#ifdef _SDMA_USE_SPINLOCK
static int sdma_try_lock(uint32_t timeout)
{
	uint8_t		spinlock_value;
	uint32_t	it;
	
	do
	{
		spinlock_value = in32(sdma.spinlock + SPINLOCK_SDMA_REG_OFFSET);
		
		if (spinlock_value == 0)
		{
			return 0;
		}
		delay(1);
	} while (it < timeout);

		return -EBUSY;
}

static void sdma_unlock()
{
	out8( sdma.spinlock + SPINLOCK_SDMA_REG_OFFSET, 0);
}
#else

static pthread_mutex_t sdma_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct sigevent mutex_event = {
	.sigev_notify = SIGEV_UNBLOCK
};

static int sdma_try_lock(uint32_t timeout)
{
	uint64_t mutex_timeout = 1000 * timeout;
	int rc;
	//BUG:not sure, we can operate like this
	TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_MUTEX, &mutex_event, &mutex_timeout, NULL);
	rc = pthread_mutex_lock( &sdma_mutex );
	
	return -rc;
	
}
								

#define sdma_unlock()		pthread_mutex_unlock( &sdma_mutex )

#endif


int sdma_reset(int channel) //rearm after fault, stop infinite cycle
{
	printf("%s: entry\n", __func__);
	uint32_t dbg_status;
	int rc;
	
	rc = sdma_try_lock(1000);
	if (rc)
	{
		return rc;
	}
    
	do
	{
		dbg_status = sdma_read32(SDMA_DBGSTATUS);
		printf("%s: dbg_status 0x%08x\n", __func__, dbg_status);
	} while (dbg_status & 1);

	sdma_write32(SDMA_DBGINST0,
	         (SDMA_DMAKILL << 16) | (channel << 8) | 1);
	sdma_write32(SDMA_DBGINST1, 0);
	sdma_write32(SDMA_DBGCMD, 0);
	
	sdma_unlock();
	
	return 0;

}

int sdma_mem_dump(uint8_t* addr, uint32_t len)
{
	printf("%s: entry\n", __func__);
	uint32_t iter = 0;
	
	printf("Numeric\n");
	for (; iter < len; iter++)
	{
		if (iter % 16 == 0)
		{
			printf("\n");
		}
		printf(" %02x ", *(addr+iter));

	}
// 	printf("Symbolic\n");
// 	for (iter = 0; iter < len; iter++)
// 	{
// 		if (!iter % 16 == 0)
// 		{
// 			printf("\n");
// 		}
// 		printf(" %c ", *(addr+iter));
// 	}
	printf("\n");
    
	return 0;
}

void sdma_print_regs(int channel)
{
	int iter = 0;

	printf("\n");
	
	printf("DSR: 0x%08x \n", sdma_read32(SDMA_DSR));
	printf("DPC: 0x%08x \n", sdma_read32(SDMA_DPC));
	printf("FSRD: 0x%08x \n", sdma_read32(SDMA_FSRD));
	printf("FSRC: 0x%08x \n", sdma_read32(SDMA_FSRC));
	printf("FTRD: 0x%08x \n", sdma_read32(SDMA_FTRD));
	
	if (channel < 0 || channel >= SDMA_MAX_CHANNELS)
	{
		for (; iter < SDMA_MAX_CHANNELS; iter++)
		{
			printf("CCR%d: 0x%08x \n", iter, sdma_read32(SDMA_CCR(iter)));
			printf("SAR%d: 0x%08x \n", iter, sdma_read32(SDMA_SAR(iter)));
			printf("DAR%d: 0x%08x \n", iter, sdma_read32(SDMA_DAR(iter)));
			printf("CSR%d: 0x%08x \n", iter, sdma_read32(SDMA_CSR(iter)));
			printf("CPC%d: 0x%08x \n", iter, sdma_read32(SDMA_CPC(iter)));
			printf("FTR%d: 0x%08x \n", iter, sdma_read32(SDMA_FTR(iter)));
		}
	}
	else
	{
		iter = channel;
		
		printf("CCR%d: 0x%08x \n", iter, sdma_read32(SDMA_CCR(iter)));
		printf("SAR%d: 0x%08x \n", iter, sdma_read32(SDMA_SAR(iter)));
		printf("DAR%d: 0x%08x \n", iter, sdma_read32(SDMA_DAR(iter)));
		printf("CSR%d: 0x%08x \n", iter, sdma_read32(SDMA_CSR(iter)));
		printf("CPC%d: 0x%08x \n", iter, sdma_read32(SDMA_CPC(iter)));
		printf("FTR%d: 0x%08x \n", iter, sdma_read32(SDMA_FTR(iter)));
	}
	
	for (iter = 0; iter <= 4; iter++)
	{
		printf("CR%d: 0x%08x \n", iter, sdma_read32(SDMA_CR(iter)));
	}
	printf("CRD: 0x%08x \n", sdma_read32(SDMA_CRD));
	printf("\n");
}
//attach vector of interrupts
static int sdma_irq_init(int irq )
{
	int it;
	/* fill in "event" structure */
	memset(&sdma_event, 0, sizeof(sdma_event));
	sdma_event.sigev_notify = SIGEV_INTR;
	
	/* Obtain I/O privileges */
	if (ThreadCtl( _NTO_TCTL_IO, 0 ) < 0)
	{
		perror("ThreadCtl");
		return -1;
	}
	
	
	
	for (it = 0; it < sdma.chnl_num; it++)
	{
		sdma.irq_num[it] = irq + it;
		
		if ((sdma.irq_hdl[it] = InterruptAttachEvent( sdma.irq_num[it], &sdma_event, 0 )) < 0)
		{
			perror("InterruptAttachEvent");
			return -1;
		}
		InterruptMask(sdma.irq_num[it], sdma.irq_hdl[it]); //hide irq
	}
	
	return 0;
}

int sdma_init(uint32_t chnl_num)
{
	printf("%s: entry\n", __func__);
	
	if ((sdma.vbase = mmap_device_io(SDMA_SIZE, SDMA_BASE)) == MAP_DEVICE_FAILED)
	{
		perror("SDMA alloc failed");
		goto sdma_fail0;
	}
#ifdef _SDMA_USE_SPINLOCK
	//mmap spinlock regs
    if ((sdma.spinlock = mmap_device_io(SPINLOCK_REG_SIZE, SPINLOCK_REG_BASE)) == MAP_DEVICE_FAILED)
	{
		perror("SPINCLOCK alloc failed");
		goto sdma_fail1;
		
		return -1;
	}
#endif
	sdma.chnl_num = chnl_num;

	if (sdma_irq_init(SDMA_IRQ_NUM))
	{
		perror("Irq init error");
		goto sdma_fail2;
		
		return -1;
	}

	sdma_print_regs(-1);
	
	return 0;
	
sdma_fail2:
#ifdef _SDMA_USE_SPINLOCK
	munmap_device_io( sdma.spinlock, SPINLOCK_REG_SIZE );
#endif
sdma_fail1:
	munmap_device_io( sdma.vbase, SDMA_SIZE );
sdma_fail0:
	return -1;
}

int sdma_fini(void)
{//BUG:  verify that sdma inited
	int it;
	
	for (it = 0; it < sdma.chnl_num; it++)
	{
		InterruptDetach(sdma.irq_hdl[it]);
	}
	
	
	munmap_device_io( sdma.vbase, SDMA_SIZE );
#ifdef _SDMA_USE_SPINLOCK
	munmap_device_io( sdma.spinlock, SPINLOCK_REG_SIZE );
#endif
	return 0;
}
//add one SDMA command byte by byte
static void sdma_command_add(struct sdma_program_buf *buf, uint64_t command,
			     size_t commandlen)
{ 
	/* TODO: Remove commandlen arg from this function */
	while (commandlen-- && buf->pos < buf->end)
	{
		*buf->pos++ = command & 0xFF;
		command >>= 8;
	}
}

//прибавть полуслово к адресу, нужно для 2D пересылок в двух направлениях?
static void sdma_addr_add(struct sdma_program_buf *program_buf, uint8_t cmd,
			  uint32_t value)
{
	int i;

	if (value == 0)
		return;

	for (i = 0; i < value / U16_MAX; ++i)
		sdma_command_add(program_buf, cmd + (U16_MAX << 8), 3);

	value %= U16_MAX;
	if (value)
		sdma_command_add(program_buf, cmd + (value << 8), 3);
}

#define SDMA_NEED_SRC_BURSTSIZE		1
#define SDMA_NEED_DST_

//convert brst_size num to bits
static uint32_t brstsize_to_bits(uint32_t brst_size)
{
	uint32_t bits = 0;

	while (brst_size > 1)
	{
		bits++;
		brst_size /= 2;
	}
	return bits;
}

static int sdma_program(struct sdma_program_buf *program_buf,
			      sdma_exchange_t *task)
{
	printf("%s: entry\n", __func__);
	uint8_t *loop_start;
	uint32_t loop_length;
	uint32_t src_brst_size;
	uint32_t dst_brst_size;
	uint32_t brst_len = 16;
	struct sdma_descriptor *sd = task->sdma_chain;
	
	uint32_t trans_count;
    //get byte number per transaction
	uint32_t i, j, trans16_pack;
	uint32_t trans_pack;
	
	uint32_t desc_it;
	//iterate over sdma_chain
	for (desc_it = 0; desc_it < task->chain_pub.chain_size; sd = &task->sdma_chain[++desc_it])
	{
		trans_count = sd->size;
		
		//because internal and external , TODO: not need to check for every sdma desc
		switch(task->direction)
		{
			case EXTR_TO_EXTR:
				src_brst_size = dst_brst_size = 1;
				break;
			case INTR_TO_INTR:
				src_brst_size = dst_brst_size = 4;
				trans_count /= 4; 
				break;
			case INTR_TO_EXTR:
				src_brst_size = 4;
				dst_brst_size = 1;
				trans_count /= 4;
				break;
			case EXTR_TO_INTR:
				src_brst_size = 1;
				dst_brst_size = 4;
				trans_count /= 4;
				break;
			default:
				errno = EINVAL;
				return -1;
				
		}
		
		trans16_pack = (trans_count / brst_len);
		trans_pack = (trans_count % brst_len);
		
		printf("%s: trans_count:\t %u\n", __func__, trans_count);
		printf("%s: trans16_pack:\t %u\n", __func__, trans16_pack);
		printf("%s: trans_pack:\t %u\n", __func__, trans_pack);
		
	    
		sdma_command_add(program_buf, SDMA_DMAMOVE_SAR, 2);
		sdma_command_add(program_buf, task->chain_pub.from + sd->f_off, 4);
	    
	    
		sdma_command_add(program_buf, SDMA_DMAMOVE_DAR, 2);
		sdma_command_add(program_buf, task->chain_pub.to + sd->t_off, 4);

		//waiting for events, DSP send events and DMA thread starts
	// 	if (sd.type == SDMA_DESCRIPTOR_E1I1 ||
	// 	    sd.type == SDMA_DESCRIPTOR_E1I0) {
	// 		sdma_command_add(program_buf,
	// 				 SDMA_DMAWFE +
	// 					((MAX_SDMA_CHANNELS + channel) << 11),
	// 				 2);
	// 	}
        
		if (task->type == SDMA_DSP_)
		{
			sdma_command_add(program_buf,SDMA_DMAWFE +((SDMA_MAX_CHANNELS + task->chain_pub.channel) << 11),2);
		}
        
		//set number of loop iterations, for several pieces of data
		if (sd->iter > 0)
		{
			sdma_command_add(program_buf, SDMA_DMALP(SDMA_LCO) + ((sd->iter-1) << 8), 2);
		}
		else //infinite cycle
		{
			//do nothing
		}
		
		loop_start = program_buf->pos;
		
			if (trans16_pack) 
			{
				sdma_command_add(program_buf, SDMA_DMAMOVE_CCR, 2);
				sdma_command_add(program_buf, SDMA_CCR_DEFAULT 
				| ((brst_len-1) << SDMA_CCR_DST_BURST_LEN) 
				| ((brst_len-1) << SDMA_CCR_SRC_BURST_LEN)
				| (brstsize_to_bits(src_brst_size) << SDMA_CCR_SRC_BURST_SIZE)
				| (brstsize_to_bits(dst_brst_size) << SDMA_CCR_DST_BURST_SIZE)
				| SDMA_CCR_SRC_INC
				| SDMA_CCR_DST_INC, 4); //set 16 sends by package for dst и src
			}
		//cycles by 255 packages
			for (i = 0; i < trans16_pack / 256; ++i) 
			{
				sdma_command_add(program_buf, SDMA_DMALP(SDMA_LC1) + (255 << 8), 2);

				//BUG: this code works only forsdma_descriptor 1 and 4 burst sizes. Otherwise need to count NOK for src 
				//and dst bursts
				for (j = 0; j < dst_brst_size; j++)
				{
					sdma_command_add(program_buf, SDMA_DMALD, 1);
				}
				for (j = 0; j < src_brst_size; j++)
				{
					sdma_command_add(program_buf, SDMA_DMAST, 1);
				}
				sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LC1) + ((dst_brst_size +  src_brst_size) << 8), 2);
			}
		//cycle for rest packages after n*255 ones
			trans16_pack = trans16_pack % 256;
			if (trans16_pack) 
			{
				sdma_command_add(program_buf,
						 SDMA_DMALP(SDMA_LC1) + ((trans16_pack - 1) << 8), 2);
				//BUG: this code works only for 1 and 4 burst sizes. Otherwise need to count NOK for src and dst bursts
				for (j = 0; j < dst_brst_size; j++)
				{
					sdma_command_add(program_buf, SDMA_DMALD, 1);
				}
				for (j = 0; j < src_brst_size; j++)
				{
					sdma_command_add(program_buf, SDMA_DMAST, 1);
				}
				sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LC1) + ((dst_brst_size +  src_brst_size) << 8), 2);
			}
		//now 1 package with rest sends (sends == rest_bytes)
			
			if (trans_pack) 
			{
				sdma_command_add(program_buf, SDMA_DMAMOVE_CCR, 2);
				//TODO: maybe assosiate unique ccr with task?
				sdma_command_add(program_buf,/*task->ccr*/SDMA_CCR_DEFAULT
						| ((trans_pack-1) << SDMA_CCR_DST_BURST_LEN) 
						| ((trans_pack-1) << SDMA_CCR_SRC_BURST_LEN)
						| (brstsize_to_bits(src_brst_size) << SDMA_CCR_SRC_BURST_SIZE)
						| (brstsize_to_bits(dst_brst_size) << SDMA_CCR_DST_BURST_SIZE)
						| SDMA_CCR_SRC_INC
						| SDMA_CCR_DST_INC, 4);  

				//BUG: this code works only for 1 and 4 burst sizes. Otherwise need to count NOK for src and dst bursts
				for (j = 0; j < dst_brst_size; j++)
				{
					sdma_command_add(program_buf, SDMA_DMALD, 1);
				}
				for (j = 0; j < src_brst_size; j++)
				{
					sdma_command_add(program_buf, SDMA_DMAST, 1);
				}
			}

			//разобраться
		    //TODO: move of DAR and SAR, not need now, cause we have only one task to do, not chain
		    //это для нескольких кусков, у нас типа куски непрерывные
		// 	if (type == SDMA_CHANNEL_INPUT)
		// 		sdma_addr_add(program_buf, SDMA_DMAADDH_SAR,
		// 			      sd.astride - sd.asize);
		// 	else
		// 		sdma_addr_add(program_buf, SDMA_DMAADDH_DAR,
		// 			      sd.astride - sd.asize);
		        
			/* FIXME: Using barrier SDMA_DMARMB or/and SDMA_DMAWMB? */

		loop_length = program_buf->pos - loop_start;
		
		if (sd->iter > 0)
		{
			sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LCO) + (loop_length << 8), 2);
		}
		else //infinite cycle
		{
			sdma_command_add(program_buf, SDMA_DMALPFE + (loop_length << 8), 2);
		}
		
		

		//sending events , interrupts
	// 	if ((sd.type == SDMA_DESCRIPTOR_E0I1) ||
	// 	    (sd.type == SDMA_DESCRIPTOR_E1I1))
	// 		sdma_command_add(program_buf, SDMA_DMASEV + (channel << 11), 2);
		
		//send irq to OS or DSP
		if (task->type == SDMA_CPU_ || task->type == SDMA_DSP_)
		{
			sdma_command_add(program_buf, SDMA_DMASEV + (task->chain_pub.channel << 11), 2);
		}

	}
	
	sdma_command_add(program_buf, SDMA_DMAWMB, 1);

	sdma_command_add(program_buf, SDMA_DMAEND, 1);
	
	return 0;
}

	

int sdma_prepare_task(sdma_exchange_t *dma_exchange)
{
	printf("%s: entry\n", __func__);
	int 	rc;
	uint8_t *code_vaddr;
	uint64_t	code_paddr;
	
	printf("%s: channel: %d\n", __func__, dma_exchange->chain_pub.channel);
	
	if (dma_exchange->chain_pub.channel >= SDMA_MAX_CHANNELS)
	{
		printf("illegal channel num\n");
		return -EINVAL;
	}
	
	if (!(dma_exchange->sdma_chain))
	{
		printf("No sdma_chain\n");
		return -EFAULT;
	}

	if ((code_vaddr = mmap(NULL, SDMA_PROG_MAXSIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("Code mmap err");
		return -ENOMEM;
	}
	
	dma_exchange->program_buf.pos = dma_exchange->program_buf.start = code_vaddr;
	dma_exchange->program_buf.end = dma_exchange->program_buf.start + SDMA_PROG_MAXSIZE;
	
	//get direction by addrs
	uint32_t dir = 0;
	
	if (dma_exchange->chain_pub.from >= SDMA_INTR_MEM_START && dma_exchange->chain_pub.from < SDMA_INTR_MEM_END)
	{
		dir |= (1 << 0);
	}
	if (dma_exchange->chain_pub.to >= SDMA_INTR_MEM_START && dma_exchange->chain_pub.to < SDMA_INTR_MEM_END)
	{
		dir |= (1 << 1);
	}
	dma_exchange->direction = dir;
	/*
	*TODO: check the size of transfer, in resmgr block??
	*/
	
	rc = sdma_program(&dma_exchange->program_buf, dma_exchange);
	
	if (rc)
	{
		return rc;
	}
	
	//get physical addr of code
	if (mem_offset64(code_vaddr, NOFD, 1, &code_paddr, 0) == -1)
	{
		perror("Get phys addr error");
		munmap(code_vaddr, SDMA_PROG_MAXSIZE);
		return -ENOMEM;
	}
	dma_exchange->program_buf.code_paddr = code_paddr;
	dma_exchange->prog_ready = SDMA_PROG_READY;
	
	printf("%s: code_phys 0x%08x\n", __func__, (uint32_t)code_paddr);
	printf("%s: code_virt 0x%08x\n", __func__, (uint32_t)code_vaddr);
	printf("%s: code_len  %-8u  \n", __func__, dma_exchange->program_buf.pos - dma_exchange->program_buf.start);
	
	return EOK;
}

static int irq_wait(sdma_exchange_t *task)
{
	printf("%s: entry\n", __func__);
	uint32_t		intstatus;
	uint32_t		inten;
	int				result;
	uint32_t		chnl_id = task->chain_pub.channel;

	InterruptUnmask(sdma.irq_num[chnl_id], sdma.irq_hdl[chnl_id]);
	
	struct sigevent e;
	e.sigev_notify = SIGEV_UNBLOCK;
	uint64_t timeout = 100 * 1000000;	//TODO: assisiate time with code len or transfer len
	
	while(1)
	{
		TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, &e, &timeout, NULL);
		result = InterruptWait(0, NULL);
		
		if (result < 0)
		{
			printf("Timeout waiting irq\n");
			return -1;
		}
		
		intstatus = sdma_read32(SDMA_INTSTATUS);
		inten = sdma_read32(SDMA_INTEN);

		if (intstatus & (1 << chnl_id))
	    {
            /* clear irq  */
			sdma_write32(SDMA_INTCLR, 1 << chnl_id);

			/* disable channel interrupt */
			inten &= ~(1 << chnl_id);
			sdma_write32(SDMA_INTEN, inten);
			
			InterruptUnmask(sdma.irq_num[chnl_id], sdma.irq_hdl[chnl_id]);
			
			return 0;
		}

		InterruptUnmask(sdma.irq_num[chnl_id], sdma.irq_hdl[chnl_id]);
	}

	return 1;
}

int sdma_transfer(sdma_exchange_t *dma_exchange)
{
	printf("%s: entry  from: 0x%08x    to: 0x%08x\n", __func__, dma_exchange->chain_pub.from, 
dma_exchange->chain_pub.to);
	uint32_t	dbg_status;
	uint32_t	val32;
	uint32_t	channel_sts;
	int			rc;


	if (dma_exchange->prog_ready != SDMA_PROG_READY)
	{
		printf("DMA exchange not prepared\n");
		return -EINVAL;
	}

	//TODO: check the vacancy of the channel
	
	channel_sts = sdma_read32(SDMA_CSR(dma_exchange->chain_pub.channel));
	if (channel_sts & 0xF)
	{
		printf("DMA echannel busy\n");
		return -EBUSY;
	}
	
// 	regmap_read(pdata->sdma, CHANNEL_STATUS(dmachain.channel.num),
// 		    &channel_status);
// 	if (channel_status & 0xF)
// 		return -EBUSY;


// 	set SDMA to set interrupts on channel
	sdma_write32(SDMA_INTEN, sdma_read32(SDMA_INTEN) | (1 << dma_exchange->chain_pub.channel));
	
	rc = sdma_try_lock(1000);
	if (rc)
	{
		return rc;
	}
    //пока STATUS не ноль, SDMA будет все игнорировать
	do {
		dbg_status = sdma_read32(SDMA_DBGSTATUS);
		printf("%s: dbg_status 0x%08x\n", __func__, dbg_status);
	} while (dbg_status & 1);
	//set command with chnl num as arg,
	val32 = (SDMA_DMAGO << 16) | (dma_exchange->chain_pub.channel << 24);
	val32 |= (dma_exchange->chain_pub.channel << 8); //set chnl num at reg field
	//if not as manager
	if (0)
	{
		val32 |= (1 << 0); //work with thread channel, otherwise with manager chnl
	}
	
	sdma_write32(SDMA_DBGINST0, val32);
	
    //со 2го по 5й байт инструкции DMAGO в регистр 1
	sdma_write32(SDMA_DBGINST1, dma_exchange->program_buf.code_paddr);
	
	sdma_print_regs(dma_exchange->chain_pub.channel);
    //запустить выполнение нструкций
	sdma_write32(SDMA_DBGCMD, 0);

	sdma_unlock();
    
    // if DSP need to process irqs, not wait here
	if ( (dma_exchange->type == SDMA_CPU_) && irq_wait(dma_exchange))
	{
		printf("error receiveing interrupt\n");
		sdma_print_regs(dma_exchange->chain_pub.channel);
		return -1;
	}
	
    sdma_print_regs(dma_exchange->chain_pub.channel);

	return 0;
}



int sdma_release_task(sdma_exchange_t *dma_exchange)
{
	printf("%s: entry\n", __func__);
	if (dma_exchange->prog_ready != SDMA_PROG_READY)
	{
		printf("DMA exchange not prepared\n");
		return -EINVAL;
	}
	
	munmap(dma_exchange->program_buf.start, SDMA_PROG_MAXSIZE);
	dma_exchange->prog_ready = !SDMA_PROG_READY;
	
	return EOK;
}
