

#ifndef _SDMA_H
#define _SDMA_H

/* ---------SDMA commands----------- */
//загружает заданное значение в один из регистров
#define SDMA_DMAMOVE_SAR			0x00BC //адрес источника
#define SDMA_DMAMOVE_CCR			0x01BC //адрес приемника
#define SDMA_DMAMOVE_DAR			0x02BC //регистр управлени я канала

//увеличивает значение SARn/DARn на 16 разрядную величину, заданную в коде команды
#define SDMA_DMAADDH_DAR			0x56
#define SDMA_DMAADDH_SAR			0x54

//установка начала блока инструкций в цикле в LC0
//SDMA повторяет инструкции между DMALP и DMAEND, количество операций - аргумент
#define SDMA_DMALP(loop_counter)		(0x20 + ((loop_counter) << 1))
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
/*---------------------------------*/

/* ----------SDMA registers------------- */
#define SDMA_INTEN					0x020		//разрешение прерываний
#define SDMA_CHANNEL_STATUS(x)			(0x100 + (8 * (x)))
#define SDMA_DBGSTATUS				0xD00		//Состояние отладки, перед записью CMD нужно его прочитать
#define SDMA_DBGCMD					0xD04		//управление выполнением инструкций, загружаемых через APB интерфейс
#define SDMA_DBGINST0				0xD08		//нулевой отладочный регистр инструкций
#define SDMA_DBGINST1				0xD0C		//первый отладочный регистр инструкций


//значение сигнала ARSIZE AXI. Определяет разрядность одной пересылки внутри пакета. 1/2/4/8/16 байт за пересылку
//1-3 биты регистра CCR
#define SDMA_BURST_SIZE(ccr)			(1 << (((ccr) >> 1) & 0x7))




#endif
