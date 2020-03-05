#include <stdint.h>
// #include <string.h>
#include <stdio.h>
#include <string.h>
#include <sdma.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <hw/inout.h>




//TODO: need some object for DMA controller??

typedef struct sdma_dev {
	uintptr_t	vbase;
	int			irq_num;
	int			irq_hdl;         
} sdma_dev_t;

sdma_dev_t sdma;
struct sigevent sdma_event;

#define U16_MAX							0xFFFF

#define sdma_read32(offset)				in32(sdma.vbase + offset)
#define sdma_write32(offset, value)		out32(sdma.vbase + offset, value)

void sdma_reset(int channel) //rearm after fault
{
	printf("%s: entry\n", __func__);
	uint32_t dbg_status;
    
	do {
		dbg_status = sdma_read32(SDMA_DBGSTATUS);
		printf("%s: dbg_status 0x%08x\n", __func__, dbg_status);
	} while (dbg_status & 1);

	sdma_write32(SDMA_DBGINST0,
	         (SDMA_DMAKILL << 16) | (channel << 8) | 1);
	sdma_write32(SDMA_DBGINST1, 0);
	sdma_write32(SDMA_DBGCMD, 0);

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

static int sdma_irq_init(int irq)
{
	/* fill in "event" structure */
	memset(&sdma_event, 0, sizeof(sdma_event));
	sdma_event.sigev_notify = SIGEV_INTR;
	
	/* Obtain I/O privileges */
	if (ThreadCtl( _NTO_TCTL_IO, 0 ) < 0)
	{
		perror("ThreadCtl");
		return -1;
	}
	
	sdma.irq_num = irq;
	
	if ((sdma.irq_hdl = InterruptAttachEvent( sdma.irq_num, &sdma_event,0 )) < 0)
	{
		perror("InterruptAttachEvent");
		return -1;
	}
	
	return 0;
}

int sdma_init(void)
{
	
	if ((sdma.vbase = mmap_device_io(SDMA_SIZE, SDMA_BASE)) == MAP_DEVICE_FAILED)
	{
		perror("SDMA alloc failed");
		return -1;
	}
	
	if (sdma_irq_init(SDMA_IRQ_NUM))
	{
		perror("Irq init error");
		munmap_device_io( sdma.vbase, SDMA_SIZE );
		return -1;
	}

	sdma_print_regs(-1);
	
	return 0;
}

int sdma_fini(void)
{
	
	InterruptDetach(sdma.irq_hdl);
	munmap_device_io( sdma.vbase, SDMA_SIZE );
	
	return 0;
}
//добавление одной команды в код программы для SDMA побайтово
static void sdma_command_add(struct sdma_program_buf *buf, uint64_t command,
			     size_t commandlen)
{ 
	/* TODO: Remove commandlen arg from this function */
	while (commandlen-- && buf->pos < buf->end) {
		*buf->pos++ = command & 0xFF;
		command >>= 8;
	}
}

//прибавть получлово к адресу, нужна для 2D пересылок...???
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

//convert num to bits
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
	
	uint32_t trans_count = task->size;
	uint32_t trans_rest; //residue, that less than
	
	//because internal and external 
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
	
    //get byte number per transaction
	uint32_t i, j, trans16_pack = (trans_count / brst_len);
	const uint32_t trans_pack = (trans_count % brst_len);
	
	printf("%s: trans_count:\t %u\n", __func__, trans_count);
	printf("%s: trans16_pack:\t %u\n", __func__, trans16_pack);
	printf("%s: trans_pack:\t %u\n", __func__, trans_pack);
	
    
	sdma_command_add(program_buf, SDMA_DMAMOVE_SAR, 2);
	sdma_command_add(program_buf, task->from, 4);
    
    
	sdma_command_add(program_buf, SDMA_DMAMOVE_DAR, 2);
	sdma_command_add(program_buf, task->to, 4);

	//waiting for events, no need now
// 	if (sd.type == SDMA_DESCRIPTOR_E1I1 ||
// 	    sd.type == SDMA_DESCRIPTOR_E1I0) {
// 		sdma_command_add(program_buf,
// 				 SDMA_DMAWFE +
// 					((MAX_SDMA_CHANNELS + channel) << 11),
// 				 2);
// 	}
	//set number of loop iterations, for several pieces of data

	sdma_command_add(program_buf, SDMA_DMALP(SDMA_LCO) + ((task->iterations-1) << 8), 2);
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
		| SDMA_CCR_DST_INC, 4); //устанавливаем 16и пересылок за пакет для dst и src
	}
//циклы по 255 пересылок
	for (i = 0; i < trans16_pack / 256; ++i) 
	{
		sdma_command_add(program_buf, SDMA_DMALP(SDMA_LC1) + (255 << 8), 2);

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
//цикл на оставшееся после n*255 пересылок
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

	//до этого места передаем по 16 пересылок за раз
	
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

        
    //move of DAR and SAR, not need now, cause we have only one task to do, not chain
// 	if (type == SDMA_CHANNEL_INPUT)
// 		sdma_addr_add(program_buf, SDMA_DMAADDH_SAR,
// 			      sd.astride - sd.asize);
// 	else
// 		sdma_addr_add(program_buf, SDMA_DMAADDH_DAR,
// 			      sd.astride - sd.asize);
        
	/* FIXME: Using barrier SDMA_DMARMB or/and SDMA_DMAWMB? */

	loop_length = program_buf->pos - loop_start;
	sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LCO) + (loop_length << 8), 2);

	//sending events , interrupts
// 	if ((sd.type == SDMA_DESCRIPTOR_E0I1) ||
// 	    (sd.type == SDMA_DESCRIPTOR_E1I1))
// 		sdma_command_add(program_buf, SDMA_DMASEV + (channel << 11), 2);
	
	//send irq to OS
	sdma_command_add(program_buf, SDMA_DMASEV + (task->channel->id << 11), 2);
	
	
	
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
	
	if (dma_exchange->channel->id >= SDMA_MAX_CHANNELS)
		return -EINVAL;

	if ((code_vaddr = mmap(NULL, SDMA_PROG_MAXSIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("Code mmap err");
		return -ENOMEM;
	}
	
	//TODO: check the vacancy of the channel
	
	dma_exchange->program_buf.pos = dma_exchange->program_buf.start = code_vaddr;
	dma_exchange->program_buf.end = dma_exchange->program_buf.start + SDMA_PROG_MAXSIZE;
	
	
	//get direction by addrs
	uint32_t dir = 0;
	
	if (dma_exchange->from >= 0x20000000UL && dma_exchange->from < 0x40000000UL)
	{
		dir |= (1 << 0);
	}
	if (dma_exchange->to >= 0x20000000UL && dma_exchange->to < 0x40000000UL)
	{
		dir |= (1 << 1);
	}
	dma_exchange->direction = dir;
	
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
	uint32_t intstatus;
	uint32_t inten;
	int i;
	int result;

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

		if (intstatus & (1 << task->channel->id))
	    {
			sdma_write32(SDMA_INTCLR, 1 << task->channel->id);

			/* disable channel interrupt */
			inten &= ~(1 << task->channel->id);
			sdma_write32(SDMA_INTEN, inten);
			
			InterruptUnmask(sdma.irq_num, sdma.irq_hdl);
			
			return 0;
		}

		InterruptUnmask(sdma.irq_num, sdma.irq_hdl);
	}

	return 1;
}

int sdma_transfer(sdma_exchange_t *dma_exchange)
{
	printf("%s: entry  from: 0x%08x    to: 0x%08x\n", __func__, dma_exchange->from, dma_exchange->to);
	uint32_t	dbg_status;
    uint32_t	val32;


	if (dma_exchange->prog_ready != SDMA_PROG_READY)
	{
		printf("DMA exchange not prepared\n");
		return -EINVAL;
	}

// 	regmap_read(pdata->sdma, CHANNEL_STATUS(dmachain.channel.num),
// 		    &channel_status);
// 	if (channel_status & 0xF)
// 		return -EBUSY;


	
// 	sdma_mem_dump((uint8_t*)code_vaddr, SDMA_PROG_MAXSIZE);
	
// 	
// 	core_id = dmachain.core;

	/* TODO: Move DSP registers setup to try_run() */

// 	/* FIXME: interrupt handler address */
// 	delcore30m_writel(pdata, core_id, DELCORE30M_INVAR,
// 			  cpu_to_delcore30m(phys_to_xyram(0x0C)));

// 	set SDMA to set interrupts on channel
	sdma_write32(SDMA_INTEN, sdma_read32(SDMA_INTEN) | (1 << dma_exchange->channel->id));
	
	//sey DSP to get interrups from SDMA
// 	delcore30m_writel(pdata, core_id, DELCORE30M_IMASKR, (1 << 30));
// 
// 	qmaskr0_val = delcore30m_readl(pdata, core_id, DELCORE30M_QMASKR0);
// 	qmaskr0_val |= 1 << (8 + dmachain.channel.num);
// 	delcore30m_writel(pdata, core_id, DELCORE30M_QMASKR0, qmaskr0_val);

// 	rc = delcore30m_spinlock_try(pdata, 1000);
// 	if (rc)
// 		return rc;
    //пока STATUS не ноль, SDMA будет все игнорировать
	do {
		dbg_status = sdma_read32(SDMA_DBGSTATUS);
		printf("%s: dbg_status 0x%08x\n", __func__, dbg_status);
	} while (dbg_status & 1);
	//set command with chnl num as arg,
	val32 = (SDMA_DMAGO << 16) | (dma_exchange->channel->id << 24);
	//if not as manager
	if (0)
	{
		val32 |= (dma_exchange->channel->id << 8); //set chnl num at reg field
		val32 |= (1 << 0); //work with thread channel, otherwise with manager chnl
	}
	
	sdma_write32(SDMA_DBGINST0, val32);
	
    //со 2го по 5й байт инструкции DMAGO в регистр 1
	sdma_write32(SDMA_DBGINST1, dma_exchange->program_buf.code_paddr);
	
	sdma_print_regs(dma_exchange->channel->id);
    //запустить выполнение нструкций
	sdma_write32(SDMA_DBGCMD, 0);
    		
	printf("%s: dbg_status 0x%08x\n", __func__, sdma_read32(SDMA_DBGSTATUS));
    
	if (irq_wait(dma_exchange))
	{
		printf("error receiveing interrupt\n");
		return -1;
	}
	
    sdma_print_regs(dma_exchange->channel->id);

// 	delcore30m_spinlock_unlock(pdata);
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
