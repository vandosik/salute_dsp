#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <sdma.h>

/**
 * struct sdma_program_buf - Internal data about SDMA program
 * @start: Pointer to start of SDMA program buffer.
 * @pos:   Pointer to current position of SDMA program buffer.
 * @end:   Pointer to end of SDMA program buffer.
 */
struct sdma_program_buf {
	uint8_t *start, *pos, *end;
};

typedef struct sdma_dev {
	uintptr_t vbase;
} sdma_dev_t;

sdma_dev_t sdma;

#define sdma_read32(offset)				in32(sdma->vbase + offset)
#define sdma_write32(offset, value)		out32(sdma->vbase + offset, value)

int sdma_init(void)
{
	if ((sdma.vbase = mmap_device_io(SDMA_SIZE, SDMA_BASE)) == MAP_DEVICE_FAILED)
	{
		perror("SDMA alloc failed");
		return -1;
	}
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


static void sdma_program(struct sdma_program_buf *program_buf,
			      sdma_exchange_t *task)
{
	uint8_t *loop_start;
	uint32_t loop_length;
	const uint32_t acnt = task.size / SDMA_BURST_SIZE(/*TODO: get CCR val*/task->ccr);
	uint32_t i, trans16_pack = (acnt / 16);
	const uint32_t trans_pack = (acnt % 16);

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
	//set number of loop iterations
	sdma_command_add(program_buf, SDMA_DMALP(SDMA_LCO) + ((iterations-1) << 8), 2);

	loop_start = program_buf->pos;
	if (trans16_pack) {
		sdma_command_add(program_buf, SDMA_DMAMOVE_CCR, 2);
		sdma_command_add(program_buf, task->ccr | (15 << 18) | (15 << 4),
				 4); //устанавливаем 16и пересылок за пакет для dst и src
	}

	for (i = 0; i < trans16_pack / 256; ++i) {
		sdma_command_add(program_buf, SDMA_DMALP(SDMA_LC1) + (255 << 8), 2);
		sdma_command_add(program_buf, SDMA_DMALD, 1);
		sdma_command_add(program_buf, SDMA_DMAST, 1);
		sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LC1) + (2 << 8), 2);
	}

	trans16_pack = trans16_pack % 256;
	if (trans16_pack) {
		sdma_command_add(program_buf,
				 SDMA_DMALP(SDMA_LC1) + ((trans16_pack - 1) << 8), 2);
		sdma_command_add(program_buf, SDMA_DMALD, 1);
		sdma_command_add(program_buf, SDMA_DMAST, 1);
		sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LC1) + (2 << 8), 2);
	} 

	//до этого места передаем по 16 байт?
	
	if (trans_pack) {
		sdma_command_add(program_buf, SDMA_DMAMOVE_CCR, 2);
		sdma_command_add(program_buf,
			    task->ccr | (trans_pack-1) << 18 | (trans_pack-1) << 4,
			    4); //устанавливаем оставшееся количество пересылок за пакет (оно меньше 16)

		sdma_command_add(program_buf, SDMA_DMALD, 1);
		sdma_command_add(program_buf, SDMA_DMAST, 1);
	}
	//move of DAR and SAR, not need now
// 	if (type == SDMA_CHANNEL_INPUT)
// 		sdma_addr_add(program_buf, SDMA_DMAADDH_SAR,
// 			      sd.astride - sd.asize);
// 	else
// 		sdma_addr_add(program_buf, SDMA_DMAADDH_DAR,
// 			      sd.astride - sd.asize);



	/* FIXME: Using barrier SDMA_DMARMB or/and SDMA_DMAWMB? */

	loop_length = program_buf->pos - loop_start;
	sdma_command_add(program_buf, SDMA_DMALPEND(SDMA_LCO) + (loop_length << 8), 2);

	//sending events, no need now
// 	if ((sd.type == SDMA_DESCRIPTOR_E0I1) ||
// 	    (sd.type == SDMA_DESCRIPTOR_E1I1))
// 		sdma_command_add(program_buf, SDMA_DMASEV + (channel << 11), 2);
	
	sdma_command_add(&program_buf, SDMA_DMAWMB, 1);
	sdma_command_add(&program_buf, SDMA_DMAEND, 1);
}

int sdma_transfer(sdma_exchange_t *dma_exchange) //maybe code addr pass as arg?
{
	int 	rc;
	uint8_t *code_vaddr;
	struct sdma_program_buf program_buf;


	if (dma_exchange->channel->id >= SDMA_MAX_CHANNELS)
		return -EINVAL;

// 	regmap_read(pdata->sdma, CHANNEL_STATUS(dmachain.channel.num),
// 		    &channel_status);
// 	if (channel_status & 0xF)
// 		return -EBUSY;

	if ((code_vaddr = mmap(NULL, SDMA_PROG_MAXSIZE, MAP_PHYS | MAP_ANON, NOFD, 0) == MAP_FAILED)
	{
		perror
	}
	
	
	program_buf.pos = program_buf.start = code_vaddr;
	program_buf.end = program_buf.start + SDMA_PROG_MAXSIZE;
	
	rc = sdma_program(dma_exchange, );
	if (rc)
		return rc;
	core_id = dmachain.core;

	/* TODO: Move DSP registers setup to try_run() */

// 	/* FIXME: interrupt handler address */
// 	delcore30m_writel(pdata, core_id, DELCORE30M_INVAR,
// 			  cpu_to_delcore30m(phys_to_xyram(0x0C)));

	//set SDMA to set interrupts on channel
// 	regmap_read(pdata->sdma, INTEN, &inten_value);
// 	inten_value |= 1 << dmachain.channel.num;
// 	regmap_write(pdata->sdma, INTEN, inten_value);

	
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
		regmap_read(pdata->sdma, DBGSTATUS, &dbg_status);
	} while (dbg_status & 1);
    //нулевой и первый байт инструкции DMAGO, номер канала(присутствует в самой команде и в "полях регистра") в регистр 
0
	regmap_write(pdata->sdma, DBGINST0,
		     (0xA0 << 16) | (dmachain.channel.num << 8) |
		     (dmachain.channel.num << 24));
    //со 2го по 5й байт инструкции DMAGO в регистр 1
	regmap_write(pdata->sdma, DBGINST1, code_addr);
    //запустить выполнение нструкций
	regmap_write(pdata->sdma, DBGCMD, 0);

	delcore30m_spinlock_unlock(pdata);
	return 0;
}

