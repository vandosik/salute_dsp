

#ifndef _SDMA_H
#define _SDMA_H

#define SDMA_MAX_CHANNELS			8
#define SDMA_PROG_MAXSIZE			1000

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
//
#define SDMA_DMALPEND(loop_counter)		(0x38 + ((loop_counter) << 2))
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
#define SDMA_DMAKILL				0x1
//завершить поток
#define SDMA_DMAEND				0x0

#define SDMA_DMAGO				0xA0
//no operation
#define SDMA_DMANOP				0x18
/*---------------------------------*/

/* ----------SDMA registers------------- */
#define SDMA_BASE					0x37220000
#define SDMA_SIZE					0x1000

#define SDMA_INTEN					0x020		//разрешение прерываний
#define SDMA_CHANNEL_STATUS(x)			(0x100 + (8 * (x)))
#define SDMA_DBGSTATUS				0xD00		//Состояние отладки, перед записью CMD нужно его прочитать
#define SDMA_DBGCMD					0xD04		//управление выполнением инструкций, загружаемых через APB интерфейс
#define SDMA_DBGINST0				0xD08		//нулевой отладочный регистр инструкций
#define SDMA_DBGINST1				0xD0C		//первый отладочный регистр инструкций
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
#define SDMA_CCR(channel)			(0x408 + 0x20 * channel)
#define SDMA_CSR(channel)			(0x100 + 0x8 * channel)
#define SDMA_CPC(channel)			(0x104 + 0x8 * channel)

#define SDMA_CR(channel)			(0xE00 + 0x4 * channel)

//значение сигнала ARSIZE AXI. Определяет разрядность одной пересылки внутри пакета. 1/2/4/8/16 байт за пересылку
//1-3 биты регистра CCR
#define SDMA_BURST_SIZE(ccr)			(1 << (((ccr) >> 1) & 0x7))
#define SDMA_CCR_SRC_INC			(1 << 0)
#define SDMA_CCR_DST_INC			(1 << 14)


typedef struct sdma_channel{
        uint8_t* rram;
        uint8_t id;
} sdma_channel_t;

typedef struct sdma_exchange{
        uint32_t from;
        uint32_t to;
//         dma_direction direction;
        uint32_t word_size;
        uint32_t size;
		uint32_t	iterations;
// 		uint32_t	ccr; //TODO: mmap SDMA_CCR reg, need to be here???
        uint8_t flags;
        sdma_channel_t* channel;
} sdma_exchange_t;


//sdma funcs

int sdma_init(void);

int sdma_fini(void);

int sdma_transfer(sdma_exchange_t *dma_exchange);

#endif