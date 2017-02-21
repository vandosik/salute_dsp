/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems.
 * 
 * Copyright 2016, CBD BC, http://www.kpda.ru  
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
#ifndef __DEVIO_H__
#define __DEVIO_H__

#include <stdint.h>
#include <arm/inout.h>
#include <fs/etfs.h>

/*************************************************************
  High-level data structures used by the devio_* routines
 *************************************************************/

// Approximately three percent of the overall flash array is reserved as a "spare area".
// Spare area is used for memory management purposes (block validity labeling, operating system flags, error recovery data)
// Spare area view:
// ---------------------------
//  struct spare                      - 36 bytes
//  eccv -       Area for data ECC    - variable length, depends form chip ecc params
//  crctrans     CRC for data ECC and 
//               struct spare         - 4 bytes
//  spare_eccv - Area for spare ECC   - variable length, depends form chip ecc params
struct spare {
	uint8_t		status;				// Factory marking for bad blks (0xff == GOOD)
	uint8_t		status2;			// For 16 bit wide parts
	uint8_t		align[6];
	uint32_t	erasesig[2];		// The erase signature created by devio_eraseblk

	// This must start on a 16 byte boundry because you can only write to a 16
	// byte region once for the new multilevel NAND parts.

	uint8_t		unused[6];
	uint8_t		nclusters;			// Number of clusters
	uint8_t		clusterhi;			// High order 8 bits of cluster
	uint32_t	sequence;			// Sequence number
	uint16_t	fid;				// File id
	uint16_t	clusterlo;			// Low order 16 bits of logical cluster
	uint32_t	crcdata;			// Crc for readcluster()
};

#define ERASESIG1	0xFFFFFFFF
#define ERASESIG2	0xFFFFFFFF


/*************************************************************
  NAND specific data structures
 *************************************************************/


// NAND flash memory part specific structure. Included in the chipio
// structure defined in the low level board driver.
struct _chipio {
	unsigned			nblks;
	unsigned 			datasize;
	unsigned 			sparesize;
	unsigned 			pages2blk;
	unsigned 			data_ecc_sz;
	unsigned 			spare_ecc_sz;
 };

#ifndef CHIPIO
#define CHIPIO struct _chipio
#endif

 
/***********************************************************************
  NAND part specific constants 
 ***********************************************************************/


// NAND flash memory device command set
#define NANDCMD_READ					0x00
#define NANDCMD_READCONFIRM				0x30
#define NANDCMD_PROGRAM					0x80
#define NANDCMD_PROGRAMCONFIRM			0x10
#define NANDCMD_ERASE					0x60
#define NANDCMD_ERASECONFIRM			0xD0
#define NANDCMD_IDREAD					0x90
#define NANDCMD_STATUSREAD				0x70
#define NANDCMD_PARAM					0xEC
#define NANDCMD_RESET					0xFF


// NAND device status register bits
#define	NAND_PROGRAM_ERASE_ERROR		0x01
#define NAND_DEVICE_READY				0x40
#define NAND_WRITE_PROTECT				0x80


#define RADDR_CYCLES					3
#define CADDR_CYCLES					2

// NAND operation timeouts (VERY generous)
#define MAX_RESET_USEC					600		// 600us
#define MAX_READ_USEC					100      //  50us
#define MAX_POST_USEC					2500    //   2ms
#define MAX_ERASE_USEC					30000	//  30ms

// NAND device status register bits
#define	NAND_PROGRAM_ERASE_ERROR		0x01

/*************************************************************
  Common definitions for devio and chipio
 *************************************************************/

// Entry into the ETFS filesystem
int etfs_main(int argc, char *argv[]);

// Prototypes for NAND chip interface. These are used by device I/O routines in the MTD.

int nand_init(struct etfs_devio *dev);
int nand_read_flashid(CHIPIO *cio, uint8_t *id);
int nand_read_onfi(CHIPIO * cio);
int nand_setup_ecc(CHIPIO * cio);
int nand_detect_chip(struct etfs_devio *dev, CHIPIO * cio, uint8_t *id);
int nand_mark_badblk( CHIPIO * cio );
int nand_wait_busy(CHIPIO *cio, uint32_t mask, uint32_t usec);
int nand_reset(CHIPIO *cio);
int nand_read_status(CHIPIO *cio, uint32_t *databuffer);
int nand_chip_select(CHIPIO  *cio, unsigned page);
int nand_erase_page(CHIPIO *cio, unsigned page, uint32_t *status);
int nand_read_page(CHIPIO *cio, unsigned page, uint8_t *databuffer, int data_cycles, int op);
int nand_write_page(CHIPIO *cio, unsigned page, uint8_t *databuffer, int data_cycles, uint32_t *status);

#endif // __DEVIO_H__
