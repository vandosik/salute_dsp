/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
 *
 * Copyright 2017, CBD BC, http://www.kpda.ru  
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * NAND flash memory chip interface routines
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/slog.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <hw/inout.h>
#include <fs/etfs.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <pthread.h>
#include "arm/mc1892vm14.h"
#include "arm/mc1892vm14_irq.h"

struct  chipio;
#define CHIPIO	struct chipio

#define DATASIZE						(cio->chip.datasize)
#define SPARESIZE						(cio->chip.sparesize)
#define PAGESIZE						(DATASIZE + SPARESIZE)
#define PAGES2BLK						(cio->chip.pages2blk)

#include "chipio.h"
#include "devio.h"

#define NAND_INTERRUPT_EVENT				(_PULSE_CODE_MINAVAIL + 0)
#define NAND_TIMEOUT_EVENT				(_PULSE_CODE_MINAVAIL + 1)
#define NAND_BLK_NUM					4096

#define PKT_REG							0x00
#define PKT_CNT_SHIFT					12

#define MEM_ADDR1_REG					0x04
#define MEM_ADDR2_REG					0x08
#define CMD_REG							0x0C
 #define CMD_COMMAND2_SHIFT				8
 #define CMD_PAGE_SIZE_SHIFT			23
 #define CMD_PAGE_SIZE_512				0
 #define CMD_PAGE_SIZE_2K				1
 #define CMD_PAGE_SIZE_4K				2
 #define CMD_PAGE_SIZE_8K				3
 #define CMD_DMA_EN_SHIFT				26
 #define CMD_DMA_ENABLE					2
 #define CMD_ADDR_CYCLES_SHIFT			28

#define PROG_REG						0x10
 #define PROG_PAGE_READ					(1 << 0)
 #define PROG_ERASE						(1 << 2)
 #define PROG_STATUS					(1 << 3)
 #define PROG_PAGE_PROG					(1 << 4)
 #define PROG_READ_ID					(1 << 6)
 #define PROG_READ_PARAM				(1 << 7)
 #define PROG_RESET						(1 << 8)

#define INTR_STS_EN_REG					0x14
#define INTR_SIG_EN_REG					0x18
#define INTR_STS_REG					0x1C
 #define INTR_WRITE_READY				(1 << 0)
 #define INTR_READ_READY				(1 << 1)
 #define INTR_XFER_COMPLETE				(1 << 2)
 #define INTR_MBIT_ERROR				(1 << 3)
 #define INTR_ERROR						(1 << 4)

#define ID1_REG							0x20
#define ID2_REG							0x24
#define FLASH_STS_REG					0x28
#define DATA_PORT_REG					0x30
#define ECC_REG							0x34
#define ECC_ERR_CNT_REG					0x38
#define ECC_SPR_CMD_REG					0x3C
#define ECC_ERR_CNT_1BIT_REG			0x40
#define ECC_ERR_CNT_2BIT_REG			0x44
#define DMA_ADDR0_REG					0x50
#define DMA_BOUND_REG					0x54
 #define DMA_BOUND_4K					0
 #define DMA_BOUND_8K					1
 #define DMA_BOUND_16K					2

#define PG_ADDR_SHIFT					16
#define MEM_ADDR_MASK					(0xFF)

#define ONFI_ID_ADDR					0x20
#define ONFI_ID_LEN						4

#define DMA_AREA_SIZE					(0x4000)
int nand_badblk_check(struct chipio * cio);
int nand_mark_badblk(struct chipio * cio);
int nand_wait_irq(struct chipio *cio, uint32_t mask, uint32_t usec);
int nand_wait_busy(struct chipio *cio, uint32_t mask, uint32_t usec);

struct chipio 		chipio;
struct sigevent 	int_event;
struct sigevent		timer_event;
// Wait routin ptr
int					(*nand_wait) (struct chipio *cio, uint32_t mask, uint32_t usec);


#ifndef NUM_ELTS
#define NUM_ELTS(x)		(sizeof((x)) / sizeof(*(x)))
#endif

struct chip_info chip_list[] = 
{
	{0x2c, 0x48, "MICRON MT29F32G08AFACAWP", 4096, 4096, 224, 128, 8, 512},
	{0x2c, 0x68, "MICRON MT29F32G08ABAAAWP", 4096, 4096, 224, 128, 8, 512},
};


// Process device specific options (if any).
// This is always called before any access to the part.
// It is called by the -D option to the filesystem. If no -D option is given
// this function will still be called with "" for optstr.
int devio_options( struct etfs_devio *dev, char *optstr )
{
	struct chipio	*cio;
	char			*value;
	char 			*p;
	char 			*buf;
	int 			i = 0;
	static char		*opts[] = {
		"use",		// index 0
		"addr",		// index 1
		"nblk",		// index 2
		"badblk",	// index 3
		"irq",		// index 4
		"pio",		// index 5
		"poll",		// index 6
		NULL
	};

	cio = dev->cio = &chipio;
	cio->chip.nblks = 0;
	cio->irq_num = MC1892VM14_IRQ_NAND;
	cio->use_dma = 1;
	cio->use_polling = 0;
	memset(cio->badblk, 0xFF, sizeof(cio->badblk));

	
	while ( *optstr ) {
		switch ( getsubopt( &optstr, opts, &value ) ) {
			case 0:
				fprintf( stderr, "Device specific options:\n" );
				fprintf( stderr, "  -D use,addr=xxx,nblk=xxx,irq=xxx,pio,poll\n" );
				return ( -1 );

			case 1:
				cio->phy_base = strtoull( value, NULL, 16 );
				break;

			case 2:
				cio->chip.nblks = strtoul( value, NULL, 10 );
				break;
			case 3:
				buf = strdup(value);
				p = strtok( buf, "." );
				while( p != NULL && i < 16) {
					cio->badblk[i++] = strtoul( p, NULL, 10 );
					fprintf( stderr, "Mark bad block[%d] %d \n", i - 1, cio->badblk[i - 1] );
					p = strtok( NULL, "." );
				}
				break;
			case 4:
				cio->irq_num = strtoul( value, NULL, 10 );
				break;
			case 5:
				cio->use_dma = 0;
				break;
			case 6:
				cio->use_polling = 1;
				break;
			default:
				dev->log (_SLOG_ERROR, "Invalid -D suboption." );
				return ( EINVAL );
		}
	}

	return ( EOK );
}

int main( int argc, char *argv[] )
{	
	name_attach_t		*pathname;
	
	if ( ( pathname = name_attach( NULL, "etfs-mcom", 0 ) ) == NULL ) {
		if ( errno == EEXIST )
			fprintf(stderr, "%s: Is already running. Exit.\n", argv[0] );
		else
			fprintf(stderr, "%s: Failed to attach pathname (%s)\n", argv[0] );
		return ( -1 );
	}
	
	return ( etfs_main( argc, argv ) );
}

// Initialize the NAND flash memory device and controller. Called once at startup.
int nand_init( struct etfs_devio *dev )
{
	struct chipio	*cio = dev->cio;

	// set the default I/O base address for the board

	if ( cio->phy_base == 0 )				//if not already set via the -D option
		cio->phy_base = MC1892VM14_NAND_BASE_ADDR;

	// map in the device registers
	cio->iobase = mmap_device_io( MC1892VM14_NAND_REG_SIZE, cio->phy_base );
	if ( cio->iobase == (uintptr_t) MAP_FAILED ) {
		dev->log( _SLOG_CRITICAL, "Unable to map in device registers (%d).", errno );
		return ( -1 );
	}
	
	if (cio->use_dma)  {
		// allocate 8K NOCACHE memory for the DMA area	
		if ((cio->dma_area = mmap(NULL,  DMA_AREA_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE, 
								MAP_PRIVATE | MAP_ANON | MAP_PHYS, NOFD, 0)) == MAP_FAILED) {
			fprintf( stderr, "Unable to alloc memory for the DMA area\n" );
			return(-1);
		}
		// get DMA area	physical address
		if (mem_offset64(cio->dma_area, NOFD, DMA_AREA_SIZE, (off64_t*)&cio->dma_paddr, NULL) != EOK) {
			fprintf( stderr, "%s: mmap failed to get physical address\n", __FUNCTION__);
			return (-1);
		}
	}
	
	/* Reset pending interrupts/status */
	out32(cio->iobase + INTR_STS_REG, in32(cio->iobase + INTR_STS_REG));

	if (!cio->use_polling) {
		
		if ((cio->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1 ||
			(cio->coid = ConnectAttach( 0, 0, cio->chid, _NTO_SIDE_CHANNEL, 0)) == -1) {
			fprintf( stderr, "Unable to attach channel and connection 0x%x\n", errno );
			return(-1);
		}

		/* Set up interrupt event for task completion*/
		int_event.sigev_notify = SIGEV_PULSE;
		int_event.sigev_coid = cio->coid;
		int_event.sigev_code = NAND_INTERRUPT_EVENT;
		int_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
		int_event.sigev_value.sival_ptr = NULL;
	
		/* Set up interrupt timerout event */
		timer_event.sigev_notify = SIGEV_PULSE;
		timer_event.sigev_coid = cio->coid;
		timer_event.sigev_code = NAND_TIMEOUT_EVENT;
		timer_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
		timer_event.sigev_value.sival_ptr = NULL;
	
		nand_wait = nand_wait_irq;
		
		cio->irq_id = InterruptAttachEvent(cio->irq_num, &int_event, _NTO_INTR_FLAGS_TRK_MSK);
		if (cio->irq_id < 0) {
			dev->log(_SLOG_CRITICAL, "Unable to attach interrupt with num - %d errno - %d.", cio->irq_num, errno);
			return (-1);
		}
	}
	else
		nand_wait = nand_wait_busy;

	return ( 0 );
}

int nand_setup_ecc(struct chipio * cio)
{
	// Set ECC Params for DATA
	if (!cio->data_bch.ecc_size || !cio->data_bch.ecc_strength) {
		fprintf(stderr, "ECC params ecc_size and ecc_strength is not specified\n");
		return ( -1 );
	}

	cio->data_bch.ecc_bytes = DIV_ROUND_UP(cio->data_bch.ecc_strength * fls(8 * cio->data_bch.ecc_size), 8);
	cio->data_bch.ecc_steps = cio->chip.datasize / cio->data_bch.ecc_size;
//	fprintf(stderr, "DATA ECC bytes per step %d ecc steps %d\n", cio->data_bch.ecc_bytes, cio->data_bch.ecc_steps);
	
	if (nand_bch_init(&cio->data_bch) == -1) {
		fprintf(stderr, "Data ECC BCH init failed\n");
		return ( -1 ); 
	}
	
	cio->chip.data_ecc_sz = cio->data_bch.ecc_bytes * cio->data_bch.ecc_steps;
	
	// Set ECC Params for SPARE
	cio->spare_bch.ecc_size = sizeof(struct spare) + cio->chip.data_ecc_sz + sizeof(uint32_t);
	cio->spare_bch.ecc_strength = cio->data_bch.ecc_strength;
	cio->spare_bch.ecc_bytes = DIV_ROUND_UP(cio->spare_bch.ecc_strength * fls(8 * cio->spare_bch.ecc_size), 8);
	cio->spare_bch.ecc_steps = 1;
	
//	fprintf(stderr, "SPARE size %d ECC size %d ECC bytes per step %d ecc steps %d\n", 
//			cio->chip.sparesize, cio->spare_bch.ecc_size,
//			cio->spare_bch.ecc_bytes, cio->spare_bch.ecc_steps);
	
	if (nand_bch_init(&cio->spare_bch) == -1) {
		fprintf(stderr, "Spare ECC BCH init failed\n");
		return ( -1 ); 
	}
	
	cio->chip.spare_ecc_sz = cio->spare_bch.ecc_bytes * cio->spare_bch.ecc_steps;
	
	if (cio->chip.data_ecc_sz + cio->chip.spare_ecc_sz + sizeof(struct spare) + sizeof(uint32_t) > cio->chip.sparesize) {
		fprintf(stderr, "Warning! Area for ECC and spare struct (%d bytes) exсeeds page SPARE (%d bytes) \n",
			cio->chip.data_ecc_sz + cio->chip.spare_ecc_sz + sizeof(struct spare) + sizeof(uint32_t), 
			cio->chip.sparesize);
		return ( -1 );
	}
		
	
	return ( 0 ); 
}

int nand_detect_chip( struct etfs_devio *dev, struct chipio * cio, uint8_t *id )
{
	int 				i;
	struct chip_info 	*chip = NULL;
	
	if(nand_read_flashid(cio, id) != 0) {
		dev->log(_SLOG_CRITICAL, "Failed to get chip IP. Timeout on ReadID command");
		return ( -1 );
	}
	
	for (i = 0; i < NUM_ELTS(chip_list); i++) {
		if ((id[0] & 0xF0) == (chip_list[i].vid & 0xF0) && id[1] == chip_list[i].did) {
			chip = &chip_list[i];
			break;
		}
	}
	
	if (chip != NULL) {
		cio->chip.datasize = chip->datasize;
		cio->chip.sparesize = chip->sparesize;
		cio->chip.pages2blk = chip->pages2blk;
		if (cio->chip.nblks == 0)
			cio->chip.nblks = chip->nblks;
		// Set ECC params
		cio->data_bch.ecc_size = chip->ecc_codeword_sz;
		cio->data_bch.ecc_strength = chip->ecc_bits;
		
		dev->log(_SLOG_INFO, "Flash ID: Manufacturer ID=0x%x, Device ID=0x%X", chip->vid, chip->did);
		dev->log(_SLOG_INFO, "Flash model: %s", chip->model);
	}
	else {
		dev->log(_SLOG_INFO, "Warning unsupported flash chip: Manufacturer ID=0x%x, Device ID=0x%x", 
				id[0], id[1]);
		dev->log(_SLOG_INFO, "Read flash chip parameter page");
		// Read Parameter Page
		if (nand_read_onfi(cio)) {
			dev->log(_SLOG_ERROR, "Failed to get OFNI Parameter page");
			return (-1);
		}
	}
	dev->log(_SLOG_INFO, "Flash info: pagesize=%d sparesize=%d nblks=%d pages2blk=%d", 
			 cio->chip.datasize, cio->chip.sparesize, cio->chip.nblks, cio->chip.pages2blk);
	dev->log(_SLOG_INFO, "Flash ECC params: ECC strength - %d  ECC codeword size %d", 
			cio->data_bch.ecc_strength, cio->data_bch.ecc_size);
	
	return ( 0 );
}


// Poll Ready/Busy status of flash memory device.
int nand_wait_busy( struct chipio *cio, uint32_t mask, uint32_t usec )
{
	uint32_t	x = 0;
	uint32_t	enabled = 0;

	enabled = in32(cio->iobase + INTR_STS_EN_REG);	
	for ( ; usec ; --usec ) {
		x = in32( cio->iobase + INTR_STS_REG ) & mask;		//check if operation is completed
		if ( x ) {
			out32(cio->iobase + INTR_STS_REG, x);
			out32(cio->iobase + INTR_STS_EN_REG, enabled & (~mask));
			return ( 0 );
		}
		else
			nanospin_ns( 1000 );
	}
	fprintf(stderr, "%s: wait failed status %x mask %x\n", __FUNCTION__, in32( cio->iobase + INTR_STS_REG ), x);
	out32(cio->iobase + INTR_STS_REG, x);
	out32(cio->iobase + INTR_STS_EN_REG, enabled & (~mask));

	// will exit from the log and never reach here.
	return ( -1 );
}

// Poll Ready/Busy status of flash memory device.
int nand_wait_irq( struct chipio *cio, uint32_t mask, uint32_t usec )
{
	uint32_t		sts = 0;
	uint32_t		enabled = 0;
	int				rval = 0;
	uint64_t 		timeout = usec * 1000;   /*Time in ns */	
	struct _pulse	pulse;
	iov_t			iov;
	int				rcvid;
	
	SETIOV(&iov, &pulse, sizeof(pulse));
	
	TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, &timer_event, &timeout, NULL);

	/* Wait for a timeout or interrupt only */
	while (1) {
		if ((rcvid = MsgReceivev(cio->chid, &iov, 1, NULL)) == -1) 
			continue;
		else
			break;
	}
	
	enabled = in32(cio->iobase + INTR_SIG_EN_REG);
	sts = in32(cio->iobase + INTR_STS_REG);

	out32(cio->iobase + INTR_STS_EN_REG, enabled & (~mask));
	out32(cio->iobase + INTR_SIG_EN_REG, enabled & (~mask));
	
	switch (pulse.code) {
		case NAND_INTERRUPT_EVENT:
			// Disable IRQ and Status
			out32(cio->iobase + INTR_STS_REG, sts);
			InterruptUnmask(cio->irq_num, cio->irq_id);
			rval = 0;
			break;
		case NAND_TIMEOUT_EVENT:
			fprintf(stderr, "%s: Timeout STS %x SIG_EN=0x%08x, MASK=0x%08x\n", __FUNCTION__, sts, enabled, mask);
			rval = -1;
			break;
		default:
			fprintf(stderr, "%s: Unsupported pulse!\n", __FUNCTION__);
			if( rcvid ) 
				MsgReplyv(rcvid, ENOTSUP, &iov, 1);
			rval = -1;
			break;
	}
	
	return (rval);
}

/* Auxiliary to contert size of page to the CMD reg pagesize bits  */
static uint8_t get_cmd_page( uint32_t pagesize )
{
	switch (pagesize) {
		case 512:
			return CMD_PAGE_SIZE_512;
		case 2048:
			return CMD_PAGE_SIZE_2K;
		case 4096:
			return CMD_PAGE_SIZE_4K;
		case 8192:
			return CMD_PAGE_SIZE_8K;
		default:
			fprintf(stderr, "Prepare command: unsupported size: %d\n", pagesize);
			break;
	}

	return 0;
}

/* Setup CMD reg for a new command */
static void nand_prepare_cmd( struct chipio * cio, uint8_t cmd1, uint8_t cmd2, 
							 uint8_t dmamode, uint32_t pagesize, uint8_t addrcycles )
{
	uint32_t	val32;

	val32 = cmd1 | (cmd2 << CMD_COMMAND2_SHIFT);
	if (dmamode) 
		val32 |= CMD_DMA_ENABLE << CMD_DMA_EN_SHIFT;
	if (addrcycles)
		val32 |= addrcycles << CMD_ADDR_CYCLES_SHIFT;
	if (pagesize)
		val32 |= get_cmd_page(pagesize) << CMD_PAGE_SIZE_SHIFT;
	
	out32(cio->iobase + CMD_REG, val32);
}

/* Setup CMD reg for a new command */
static void nand_set_page_addr( struct chipio * cio, uint32_t page, uint16_t col )
{
	uint64_t	val32;

	out32(cio->iobase + MEM_ADDR1_REG, col | (page << PG_ADDR_SHIFT));

	val32 = in32(cio->iobase + MEM_ADDR2_REG);
	val32 = (val32 & ~MEM_ADDR_MASK) |
	      ((page >> PG_ADDR_SHIFT) & MEM_ADDR_MASK);
		  
	out32(cio->iobase + MEM_ADDR2_REG, val32);
}

static inline void nand_set_pkt_size( struct chipio * cio, uint32_t size, uint32_t count )
{
	out32(cio->iobase + PKT_REG, size | (count << PKT_CNT_SHIFT));
}

static inline void nand_set_irq_masks( struct chipio * cio, uint32_t val )
{
	out32(cio->iobase + INTR_STS_REG, in32(cio->iobase + INTR_STS_REG));
	out32(cio->iobase + INTR_STS_EN_REG, val);
	out32(cio->iobase + INTR_SIG_EN_REG, val);
}


int nand_read_onfi( struct chipio * cio )
{
	unsigned  	onfi_buf[64];
	int 	i;
	volatile unsigned  val32;
	unsigned char *databuffer = (unsigned char *)onfi_buf;
	
	nand_prepare_cmd(cio, NANDCMD_PARAM, 0, 0, 0, 1);
	nand_set_page_addr(cio, 0, 0);
	nand_set_pkt_size(cio, 256, 1);
	
	nand_set_irq_masks(cio, INTR_READ_READY);
	// Initiate command
	out32(cio->iobase + PROG_REG, PROG_READ_PARAM);
	
	if ( nand_wait( cio, INTR_READ_READY,  MAX_ERASE_USEC) !=  0) {
		fprintf(stderr, "%s: Read Ready wait failed\n", __FUNCTION__);
		return ( -1 );
	}
	else {
		nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
		for ( i = 0; i < 256; i += 4 ) {
			val32 = in32( cio->iobase + DATA_PORT_REG );
			*(uint32_t *)(databuffer + i)  = ENDIAN_LE32(val32);
			nanospin_ns( 1000 );
		}
		if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Read Complete wait failed\n", __FUNCTION__);
			return ( -1 );
		}
	}

	if (*(uint8_t *)(databuffer) != 'O' || *(uint8_t *)(databuffer + 1) != 'N' ||
		*(uint8_t *)(databuffer + 2) != 'F' || *(uint8_t *)(databuffer + 3) != 'I')
		return (-1);
	
	cio->chip.datasize = ENDIAN_LE32(*(uint32_t*)(databuffer + 80));
	cio->chip.sparesize = ENDIAN_LE16(*(uint16_t*)(databuffer + 84));
	if (cio->chip.nblks == 0)
		cio->chip.nblks = ENDIAN_LE32(*(uint32_t*)(databuffer + 96));
	cio->chip.pages2blk = ENDIAN_LE16(*(uint16_t*)(databuffer + 92));
	
	// Get ECC params
	cio->data_bch.ecc_strength = *(uint8_t *)(databuffer + 112);
	
	
	if (cio->data_bch.ecc_strength != 0xFF)
		cio->data_bch.ecc_size = 512;
	else {
		/* TODO: here we have to read extended parameter page to get ecc_bits and codeword size
		 * not implemented yet as we don't have a chip with extended parameter page
		 */
		fprintf(stderr, "%s: Can't get ECC strength and ECC codeword size params. Not implemented yet!\n", __FUNCTION__);
		return (-1);
	}

	return (0);
}

int nand_read_flashid( struct chipio * cio, uint8_t *id )
{
	uint32_t	id_val;
	// CMD0 = 0x90 = Read ID command; other commands unused
	nand_prepare_cmd(cio, NANDCMD_IDREAD, 0, 0, 0, 1);
	nand_set_page_addr(cio, 0, 0);
	nand_set_pkt_size(cio, ONFI_ID_LEN, 1);
	
	nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
	// Initiate command
	out32(cio->iobase + PROG_REG, PROG_READ_ID);
	
	if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
		return ( -1 );
	}
	else {
		// 2 bytes of ID code
		id_val = in32(cio->iobase + ID1_REG);
		id[0] = (id_val >> 8) & 0xFF;
		id[1] = (id_val >> 16) & 0xFF;
	}

	return ( 0 );
}

int nand_mark_badblk( struct chipio * cio )
{
	int					i;
	uint32_t			status;
	uint8_t				buf[PAGESIZE];
	
	if (cio->badblk[0] == -1) 
		return ( 0 );
	
	memset(buf, 0xFF, PAGESIZE);
	buf[DATASIZE] = 0;
	buf[DATASIZE + 1] = 0;
	
	for (i = 0; i < 16; i++) {
		if (cio->badblk[i] == -1) 
			break;
		
		fprintf( stderr, "%s: mark block %d as bad.\n", __FUNCTION__, cio->badblk[i] );
		
		if(nand_erase_page(cio, cio->badblk[i] * PAGES2BLK, &status) != 0) {
			fprintf(stderr, "%s: Timeout on ERASE blk %d\n", __FUNCTION__, cio->badblk[i]);
			return ( -1 );
		}
		
		if (nand_write_page(cio, cio->badblk[i] * PAGES2BLK, buf, PAGESIZE, &status) || 
			(status & NAND_PROGRAM_ERASE_ERROR)) {
			fprintf( stderr, "%s: Program Timeout/Error on page %d\n", __FUNCTION__, cio->badblk[i] * PAGES2BLK);
			return( -1 );
		}
	}
	return ( 0 );
}


int nand_read_status( struct chipio * cio, uint32_t *status )
{
	// CMD0 = 0x70 = Read status command; other commands unused
	nand_prepare_cmd(cio, NANDCMD_STATUSREAD, 0, 0, 0, 0);
	nand_set_pkt_size(cio, 1, 1);
	nand_set_page_addr(cio, 0, 0);
	
	nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
	// Initiate command
	out32(cio->iobase + PROG_REG, PROG_STATUS);
	
	if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_POST_USEC ) != 0 )
		return ( -1 );
	else {
		*status = in32( cio->iobase + FLASH_STS_REG );
		if ( *status & 1 ) {
			return ( -1 );
		}
	}
	
	return ( 0 ); 
}

// reset flash memory device.
int nand_reset( struct chipio *cio )
{
	nand_prepare_cmd(cio, NANDCMD_RESET, 0, 0, 0, 0);
	nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
	
	// Initiate command
	out32(cio->iobase + PROG_REG,  PROG_RESET);

	if ( nand_wait( cio, INTR_XFER_COMPLETE, MAX_ERASE_USEC ) != 0 ) {
		fprintf(stderr, "Reset TIMEOUT\n");
		return ( -1 );
	}
	else
		return ( 0 );
}

int nand_erase_page( struct chipio * cio, unsigned page, uint32_t *status )
{
	uint32_t	page_addr;
	uint32_t	column;
	
	nand_prepare_cmd(cio, NANDCMD_ERASE, NANDCMD_ERASECONFIRM, 0, 0, RADDR_CYCLES);
	column = page & 0xffff;
	page_addr = (page >> PG_ADDR_SHIFT) & 0xffff;
	nand_set_page_addr(cio, page_addr, column);
	
	nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
	// Initiate command
	out32(cio->iobase + PROG_REG,  PROG_ERASE);
	
	if ( nand_wait( cio, INTR_XFER_COMPLETE, MAX_ERASE_USEC ) != 0 ) {
		fprintf(stderr, "%s: nand_wait - error...\n", __FUNCTION__);
		return ( -1 );
	}
	else {
		nand_read_status( cio, (uint32_t *)status );
		if (*(uint32_t *)status & 1)
			fprintf(stderr, "%s: error status %x\n", __FUNCTION__, *(uint8_t *)status);
	}
	
	return (0);
}

int nand_read_page( struct chipio * cio, unsigned page, uint8_t *databuffer, int data_cycles, int op )
{
	int 				i;
	int 				j = 0;
	int 				pkt_num = 0;
	volatile uint32_t	val32;
	unsigned			rsize = op ? SPARESIZE : PAGESIZE;
	
	if (!op) {
		/* Prepare command to read Page Data */
		nand_prepare_cmd(cio, NANDCMD_READ, NANDCMD_READCONFIRM, cio->use_dma, DATASIZE, RADDR_CYCLES + CADDR_CYCLES);
		nand_set_page_addr(cio, page, 0);
		nand_set_pkt_size(cio, 512, 8);
	
		if (cio->use_dma) {
			out32(cio->iobase + DMA_BOUND_REG, DMA_BOUND_8K);
			out32(cio->iobase + DMA_ADDR0_REG, cio->dma_paddr);
			nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
			// Initiate command
			out32(cio->iobase + PROG_REG, PROG_PAGE_READ);
			
			if (nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
				fprintf(stderr, "%s: Read DMA Complete wait failed\n", __FUNCTION__);
				return ( -1 );
			}
			memcpy(databuffer, cio->dma_area, DATASIZE);
			j += DATASIZE;
		}
		else {
			nand_set_irq_masks(cio, INTR_READ_READY);
			// Initiate command
			out32(cio->iobase + PROG_REG, PROG_PAGE_READ);
		
			while (1) {
				if ( nand_wait( cio, INTR_READ_READY,  MAX_ERASE_USEC) !=  0) {
					fprintf(stderr, "%s: Read Ready wait failed\n", __FUNCTION__);
					return ( -1 );
				}
				else {
					pkt_num++;
					if (pkt_num == 8)
						nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
					for ( i = 0; i < 512; i += 4, j += 4 ) {
						val32 = in32( cio->iobase + DATA_PORT_REG );
						*(uint32_t *)(databuffer + j)  = ENDIAN_LE32(val32);
					}
				}
		
				if (pkt_num < 8)
					nand_set_irq_masks(cio, INTR_READ_READY);
				else {
					if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
						fprintf(stderr, "%s: Read Complete wait failed\n", __FUNCTION__);
						return ( -1 );
					}
					break;
				}
			}
		}
	}
	
	/* Prepare command to read Page SPARE */
	nand_prepare_cmd(cio, NANDCMD_READ, NANDCMD_READCONFIRM, cio->use_dma, 4096, RADDR_CYCLES + CADDR_CYCLES);
	nand_set_page_addr(cio, page, DATASIZE);
	nand_set_pkt_size(cio, SPARESIZE, 1);
	
	if (cio->use_dma) {
		out32(cio->iobase + DMA_BOUND_REG, DMA_BOUND_8K);
		out32(cio->iobase + DMA_ADDR0_REG, cio->dma_paddr);
		nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
		// Initiate command
		out32(cio->iobase + PROG_REG, PROG_PAGE_READ);
			
		if (nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Read DMA Complete wait failed\n", __FUNCTION__);
			return ( -1 );
		}
		memcpy(databuffer + j, cio->dma_area, SPARESIZE);
	}
	else {
		nand_set_irq_masks(cio, INTR_READ_READY);
		// Initiate command
		out32(cio->iobase + PROG_REG, PROG_PAGE_READ);
	
		if (nand_wait( cio, INTR_READ_READY,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Read Ready wait failed\n", __FUNCTION__);
			return ( -1 );
		}
		nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
		for ( i = 0; i < SPARESIZE; i += 4, j += 4 ) {
			val32 = in32( cio->iobase + DATA_PORT_REG );
			*(uint32_t *)(databuffer + j)  = ENDIAN_LE32(val32);
		}
	
		if (nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Read Complete wait failed\n", __FUNCTION__);
			return ( -1 );
		}
	}
	return ( rsize );
}


int nand_write_page( struct chipio * cio, unsigned page, uint8_t *databuffer, int data_cycles, uint32_t *status )
{
	int 				i;
	int 				j = 0;
	uint32_t 			pkt_num;
	
	
	/* Prepare command to read Page Data */
	nand_prepare_cmd(cio, NANDCMD_PROGRAM, NANDCMD_PROGRAMCONFIRM, cio->use_dma, DATASIZE, RADDR_CYCLES + CADDR_CYCLES);
	nand_set_page_addr(cio, page, 0);
	nand_set_pkt_size(cio, 512, 8);
	
	if (cio->use_dma) {
		memcpy(cio->dma_area, databuffer, DATASIZE);
		out32(cio->iobase + DMA_BOUND_REG, DMA_BOUND_8K);
		out32(cio->iobase + DMA_ADDR0_REG, cio->dma_paddr);
		nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
		out32(cio->iobase + PROG_REG, PROG_PAGE_PROG);
		if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Write DMA Complete wait failed\n", __FUNCTION__);
 			return ( -1 );
		}
		j += DATASIZE;
	}
	else {
		nand_set_irq_masks(cio, INTR_WRITE_READY);
		// Initiate command
		out32(cio->iobase + PROG_REG, PROG_PAGE_PROG);

		pkt_num = 0;
		j = 0;
		while (1) {
			if ( nand_wait( cio, INTR_WRITE_READY,  MAX_ERASE_USEC) !=  0) {
				fprintf(stderr, "%s: Write Ready wait failed\n", __FUNCTION__);
				return ( -1 );
			}
			else {
				pkt_num++;
				if (pkt_num == 8)
					nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
				for ( i = 0; i < 512; i += 4, j += 4 ) {
					out32(cio->iobase + DATA_PORT_REG, *(uint32_t *)(databuffer + j));
				}
			}
		
			if (pkt_num < 8)
				nand_set_irq_masks(cio, INTR_WRITE_READY);
			else {
				nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
				if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
					fprintf(stderr, "%s: Write Complete wait failed\n", __FUNCTION__);
					return ( -1 );
				}
				break;
			}
		}
	}

	/* Prepare command to read Page SPARE */
	nand_prepare_cmd(cio, NANDCMD_PROGRAM, NANDCMD_PROGRAMCONFIRM, cio->use_dma, DATASIZE, RADDR_CYCLES + CADDR_CYCLES);
	nand_set_page_addr(cio, page, DATASIZE);
	nand_set_pkt_size(cio, SPARESIZE, 1);

	if (cio->use_dma) {
		memcpy(cio->dma_area, databuffer + DATASIZE, SPARESIZE);
		out32(cio->iobase + DMA_BOUND_REG, DMA_BOUND_8K);
		out32(cio->iobase + DMA_ADDR0_REG, cio->dma_paddr);
		nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
		out32(cio->iobase + PROG_REG, PROG_PAGE_PROG);
		if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: SPARE Write DMA Complete  wait failed\n", __FUNCTION__);
 			return ( -1 );
		}
	}
	else {
		nand_set_irq_masks(cio, INTR_WRITE_READY);
		// Initiate command
		out32(cio->iobase + PROG_REG, PROG_PAGE_PROG);
	
		if (nand_wait( cio, INTR_WRITE_READY,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Write Ready wait failed\n", __FUNCTION__);
			return ( -1 );
		}
		nand_set_irq_masks(cio, INTR_XFER_COMPLETE);
		for ( i = 0; i < SPARESIZE; i += 4, j += 4 ) {
			out32(cio->iobase + DATA_PORT_REG, *(uint32_t *)(databuffer + j));
		}
	
		if ( nand_wait( cio, INTR_XFER_COMPLETE,  MAX_ERASE_USEC) !=  0) {
			fprintf(stderr, "%s: Write Complete wait failed\n", __FUNCTION__);
			return ( -1 );
		}	
	}
 	
	nand_read_status( cio, (uint32_t *)status );
	
 	return ( 0 );
}
