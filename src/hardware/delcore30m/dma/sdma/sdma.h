

#ifndef _SDMA_H
#define _SDMA_H

#define SDMA_MAX_CHANNELS			8
#define SDMA_PROG_MAXSIZE			1000

//set in arm/mc1892vm14.h, TODO: need to take this from hwi
#define SDMA_BASE					0x37220000UL
#define SDMA_SIZE					0x1000
//set in arm/mc1892vm14_irq.h, TODO: need to take this from hwi
#define SDMA_IRQ_NUM				(32 + 8)

#define SPINLOCK_REG_BASE			0x38081000
#define SPINLOCK_REG_SIZE			0x1000
#define SPINLOCK_REG_OFFSET(num)	(0x800 + (0x4 * num))
#define SPINLOCK_SDMA_REG_OFFSET	SPINLOCK_REG_OFFSET(1) 

/* ---------SDMA commands----------- */
//загружает заданное значение в один из регистров
#define SDMA_DMAMOVE_SAR			0x00BC //адрес источника
#define SDMA_DMAMOVE_CCR			0x01BC //регистр управлени я канала
#define SDMA_DMAMOVE_DAR			0x02BC //адрес приемника
//увеличивает значение SARn/DARn на 16 разрядную величину, заданную в коде команды
#define SDMA_DMAADDH_DAR			0x56
#define SDMA_DMAADDH_SAR			0x54

//установка начала блока инструкций в цикле в LC0
//SDMA повторяет инструкции между DMALP и DMAEND, количество операций - аргумент
#define SDMA_DMALP(loop_counter)		(0x20 + ((loop_counter) << 1))
	#define SDMA_LCO				0
	#define SDMA_LC1				1
//конец цикла
#define SDMA_DMALPEND(loop_counter)		(0x38 + ((loop_counter) << 2))

//возврат в бесконечном цикле
#define SDMA_DMALPFE					(0x2C)

//ожидать событие, приостановка работы программы, пока не поступит событие с номером - аргументом
#define SDMA_DMAWFE				0x36
//послать событие с номером - аргументом
#define SDMA_DMASEV				0x34
//ожидание предшествующих инструкций, барьер записи в память
#define SDMA_DMAWMB				0x13
//-""- барьер чтения памяти
#define SDMA_DMARMB				0x12
//запись данных из MIFO в приемник, адрес приемника в DAR
#define SDMA_DMAST				0x0B
//чтения данных из источника в MIFO, адрес источника в SAR
#define SDMA_DMALD				0x07
//перывает выполнение потока
#define SDMA_DMAKILL			0x1
//завершить поток
#define SDMA_DMAEND				0x0

#define SDMA_DMAGO				0xA0
//no operation
#define SDMA_DMANOP				0x18
/*---------------------------------*/

/* ----------SDMA registers------------- */

//разрешение прерываний по инструкции DMASEV
#define SDMA_INTEN					0x020
//регистр статуса прерываний
#define SDMA_INTSTATUS				0x028
//Регистр сброса прерываний
#define SDMA_INTCLR					0x02C
//Состояние отладки, перед записью CMD нужно его прочитать
#define SDMA_DBGSTATUS				0xD00
//управление выполнением инструкций, загружаемых через APB интерфейс
#define SDMA_DBGCMD					0xD04
//нулевой отладочный регистр инструкций
#define SDMA_DBGINST0				0xD08
//первый отладочный регистр инструкций
#define SDMA_DBGINST1				0xD0C
//регистр состояния потока управления
#define SDMA_DSR					0x0
//счетчик команд потока управления
#define SDMA_DPC					0x4
//сбой потока управления
#define SDMA_FSRD					0x30
//сбой потока каналов
#define SDMA_FSRC					0x34
//тип сбоя потока упр
#define SDMA_FTRD					0x38
//тип сбоя потоков каналла
#define SDMA_FTR(channel)			(0x40 + 0x4 * channel)

//channel regs
#define SDMA_SAR(channel)			(0x400 + 0x20 * channel) //src addr reg
#define SDMA_DAR(channel)			(0x404 + 0x20 * channel) //dst addr reg
#define SDMA_CCR(channel)			(0x408 + 0x20 * channel) //setup reg
#define SDMA_CSR(channel)			(0x100 + 0x8 * channel) //channel ctatus
#define SDMA_CPC(channel)			(0x104 + 0x8 * channel) //channel PC


#define SDMA_CR(num)				(0xE00 + 0x4 * num)
#define SDMA_CRD					0xE14

//значение сигнала ARSIZE AXI. Определяет разрядность одной пересылки внутри пакета. 1/2/4/8/16 байт за пересылку
//1-3 биты регистра CCR     
#define SDMA_CCR_DEFAULT			0x00800200
// #define SDMA_BURST_SIZE(ccr)		(1 << (((ccr) >> 1) & 0x7))
#define SDMA_CCR_SRC_BURST_SIZE		1
#define SDMA_CCR_DST_BURST_SIZE		15
#define SDMA_CCR_SRC_INC			(1 << 0)
#define SDMA_CCR_DST_INC			(1 << 14)
#define SDMA_CCR_SRC_BURST_LEN		4
#define SDMA_CCR_DST_BURST_LEN		18



/**
 * struct sdma_program_buf - Internal data about SDMA program
 * @start: Pointer to start of SDMA program buffer.
 * @pos:   Pointer to current position of SDMA program buffer.
 * @end:   Pointer to end of SDMA program buffer.
 */
struct sdma_program_buf {
	uint8_t *start, *pos, *end;
	uint32_t	code_paddr;
};

#define SDMA_PROG_READY			0x10101010

#define SDMA_INTR_MEM_START		0x20000000UL
#define SDMA_INTR_MEM_END		0x40000000UL

typedef enum {EXTR_TO_EXTR = 0, INTR_TO_EXTR, EXTR_TO_INTR, INTR_TO_INTR} sdma_direction;

struct sdma_descriptor {
	uint32_t f_off; //offset from sdma_exchange->from
	uint32_t t_off;
	uint32_t size; //size of data to be sent
	uint8_t iter; // количество повторов отправки данного пакета от 1 до 255, 0 - повторять бесконечно
};

typedef enum {SDMA_DSP_, SDMA_CPU_} sdma_extype; //TODO;расширить на отправку/ожидание событий

//public part of job
typedef struct _elcore_sdma_chain {
		uint32_t					job_id;
		uint32_t					from;
		uint32_t					to;
		uint32_t					channel; //channel number
		uint32_t					chain_size; //in sdma_desc
		struct sdma_descriptor		*sdma_chain; //цепочка пакетов обмена
} SDMA_CHAIN;

//TODO: some fields, such as code_addr, are not for customer. Need to hide them.
typedef struct sdma_exchange{
		SDMA_CHAIN					chain_pub;
		sdma_extype					type;
		sdma_direction				direction;
		struct sdma_program_buf		program_buf;
		uint32_t					prog_ready;
		uint8_t flags;
} sdma_exchange_t;



//sdma funcs

int sdma_init(uint32_t chnl_num);

int sdma_fini(void);
//prepare sdma exchange to work
int sdma_prepare_task(sdma_exchange_t *dma_exchange);
//do exchange
int sdma_transfer(sdma_exchange_t *dma_exchange);
//release resources, assosiated with dma exchange
int sdma_release_task(sdma_exchange_t *dma_exchange);

int sdma_reset( int channel);

int sdma_mem_dump(uint8_t* addr, uint32_t len);

#endif
