/*
 * $QNXLicenseC:
 * Copyright 2010, QNX Software Systems.
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

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <gulliver.h>
#include <sys/slog.h>
#include <sys/neutrino.h>
#include <fs/etfs.h>
#include "devio.h"
#include "chipio.h"


// NAND part specific parameters
#define DATASIZE						(cio->datasize)
#define SPARESIZE						(cio->sparesize)
#define PAGESIZE						(DATASIZE + SPARESIZE)
#define PAGES2BLK						(cio->pages2blk)

#define DATA_ECCV_OFFSET				(sizeof(struct spare))
#define SPARE_ECCV_OFFSET				(sizeof(struct spare) + cio->data_ecc_sz + 4)
#define SPARE_CRC_OFFSET				(sizeof(struct spare) + cio->data_ecc_sz)
/*
 * NAND flash memory chip init/read/write/erease routines
 */

int devio_init(struct etfs_devio *dev) {
	uint8_t			id[2];
	uint32_t		status;
	CHIPIO			*cio = dev->cio;

	// allow IO operations
	if(ThreadCtl(_NTO_TCTL_IO, 0) != EOK) {
		dev->log(_SLOG_CRITICAL, "You must be root."); //exits
	}

	// configure device
	if(nand_init(dev) != 0) {
		dev->log(_SLOG_CRITICAL, "nand_init failed : %s", strerror(errno)); //exits
	}

	// reset the device
	if (nand_reset(cio) != 0) {
		dev->log(_SLOG_CRITICAL, "Timeout on RESET"); //exits
	}
	
	// read status
	if (nand_read_status(cio, &status) != 0)
		dev->log(_SLOG_ERROR, "Failed to get NAND chip status");

	// read ID
	if(nand_detect_chip(dev, cio, id) != EOK) {
		dev->log(_SLOG_CRITICAL, "Failed to identify nand chip\n");
		return (EINVAL);
	}

	if (nand_setup_ecc(cio) != 0) {
		dev->log(_SLOG_CRITICAL, "Failed to setup ECC\n");
		return (EINVAL);
	}
	
	nand_mark_badblk(cio);
	
	dev->numblks = cio->nblks;
		
	sprintf(dev->id, "NAND%2.2x%2.2x", id[0], id[1]);
	dev->memtype       = ETFS_MEMTYPE_NAND;
	
	// nand get chip info
	
	// We glue to physical pages at the driver and report back their combined size
	dev->clustersize   = cio->datasize;;
	dev->sparesize     = cio->sparesize;
	dev->clusters2blk  = cio->pages2blk;
	dev->blksize       = (dev->clustersize + dev->sparesize) * dev->clusters2blk;

	return(EOK);
}

// Read a cluster of data.
// Verify crc for both the spare area and the entire page (data + spare).
// The passed buffer "buf" is larger than the cluster size. It can hold
// PAGESIZE bytes. This is for convenience when reading data from the
// device and calculating the crc. The work area after clustersize
//bytes is ignored by the caller.
int devio_readcluster(struct etfs_devio *dev, unsigned cluster, uint8_t *buf, struct etfs_trans *trp) {

	struct spare		*sp;
	int					err;
	unsigned			page = cluster;
	CHIPIO				*cio = dev->cio;
	unsigned 			crc_bytes;

	// read page from the device
	// for 1 cluster = 1 page
	
	if (nand_read_page(cio, page, buf, PAGESIZE, 0) < 0)
	    dev->log(_SLOG_CRITICAL, "Timeout on READ");
	
	//spare starts at offset 4096 (DATASIZE) in page
	sp = (struct spare *)(buf + DATASIZE);
	crc_bytes = DATA_ECCV_OFFSET + cio->data_ecc_sz;
	
	// try to correct
	if (nand_bch_correct(cio, ECC_SPARE, (uint8_t *)sp, (uint8_t *)sp + SPARE_ECCV_OFFSET) > 0)
		dev->log(_SLOG_ERROR, "devio_readcluster: Failed to correct SPARE page %d", page);

	// determine transaction codes
	if(sp->status != 0xff || sp->status2 != 0xff) {
		dev->log(_SLOG_ERROR, "devio_readcluster: readtrans BADBLK on cluster %d status[%02x:%02x]", 
												cluster, sp->status, sp->status2 );
		trp->tacode = ETFS_TRANS_BADBLK;
	}
	else if(((uint64_t *)sp)[2] == ~0ll  &&  ((uint64_t *)sp)[3] == ~0ll
			&& ((uint64_t *)sp)[4] == ~0ll  &&  ((uint64_t *)sp)[5] == ~0ll
				&& ((uint64_t *)sp)[6] == ~0ll  &&  ((uint64_t *)sp)[7] == ~0ll) {
			dev->log(_SLOG_ERROR, "devio_readcluster: cluster %d erased", cluster);
			trp->tacode = ETFS_TRANS_ERASED;
	}
	else if(dev->crc32((uint8_t *) sp, crc_bytes) != *(uint32_t *)((uint8_t *)sp + SPARE_CRC_OFFSET)) {
		dev->log(_SLOG_ERROR, "devio_readcluster: readcluster trans DATAERR on cluster %d", cluster);
		trp->tacode = ETFS_TRANS_DATAERR;
	}
	else
		trp->tacode = ETFS_TRANS_OK;

	// build transaction data from data in the spare area
	trp->sequence    = ENDIAN_LE32(sp->sequence);
	trp->fid         = ENDIAN_LE16(sp->fid);
	trp->cluster     = ENDIAN_LE16(sp->clusterlo) + (sp->clusterhi << 16);
	trp->nclusters   = sp->nclusters;

	trp->dacode = ETFS_TRANS_OK;
	if (trp->tacode == ETFS_TRANS_OK) {
		err = nand_bch_correct(cio, ECC_DATA, buf, (uint8_t *)sp + DATA_ECCV_OFFSET);
		if (err > 0 /*|| dev->crc32(buf, DATASIZE) != sp->crcdata*/) {
			dev->log(_SLOG_ERROR, "devio_readcluster: readcluster DATAERR on cluster %d err %d crcdata %x calccrc %x", 
						 cluster, err, sp->crcdata, dev->crc32(buf, DATASIZE));
			return(trp->dacode = ETFS_TRANS_DATAERR);
		}
	}

	return(trp->tacode);
}

//Read the spare area of a page (not the data) to return transaction information.
// This called is used heavily on startup to process the transactions. It is
// a cheaper call than readcluster() since it reads less data and has a smaller
// checksum to calculate.
int devio_readtrans(struct etfs_devio *dev, unsigned cluster, struct etfs_trans *trp) {

	struct spare        *sp;
	unsigned            page = cluster;
	CHIPIO              *cio = dev->cio;
	unsigned            crc_bytes;
	uint8_t             *spare = alloca(SPARESIZE);
	
	// read page from the device
	// for 1 cluster = 1 page
	if (nand_read_page(cio, page, spare, SPARESIZE, 1) < 0)
		dev->log(_SLOG_CRITICAL, "Timeout on READ");
	
	//spare starts at offset 2048 (DATASIZE) in page

	sp = (struct spare *)(spare);
	crc_bytes = DATA_ECCV_OFFSET + cio->data_ecc_sz;
	
	nand_bch_correct(cio, ECC_SPARE, (uint8_t *)sp, (uint8_t *)sp + SPARE_ECCV_OFFSET);
	
	// Check if bad block
	if(sp->status != 0xff || sp->status2 != 0xff) {
		dev->log(_SLOG_ERROR, "devio_readtrans: readtrans BADBLK on cluster %d", cluster);
		return(ETFS_TRANS_BADBLK);
	}

	//check if erased block
	if(((uint64_t *)sp)[2] == ~0ll  &&  ((uint64_t *)sp)[3] == ~0ll
				&& ((uint64_t *)sp)[4] == ~0ll  &&  ((uint64_t *)sp)[5] == ~0ll
					&& ((uint64_t *)sp)[6] == ~0ll  &&  ((uint64_t *)sp)[7] == ~0ll) {
			return(ETFS_TRANS_ERASED);
	}
	// Validate checksum
	
	if(dev->crc32((uint8_t *) sp, crc_bytes) != *(uint32_t *)((uint8_t *)sp + SPARE_CRC_OFFSET)) {
		// try to correct
		dev->log(_SLOG_ERROR, "devio_readtrans: readtrans DATAERR on cluster %d", cluster);
		return(ETFS_TRANS_DATAERR);
	}

	// Build transaction data

	trp->sequence    = ENDIAN_LE32(sp->sequence);
	trp->fid         = ENDIAN_LE16(sp->fid);
	trp->cluster     = ENDIAN_LE16(sp->clusterlo) + (sp->clusterhi << 16);
	trp->nclusters   = sp->nclusters;

	return(ETFS_TRANS_OK);
}


// Post a cluster of data.
// Set crc for both the spare area and the entire page (data + spare).
// The passed buffer "buf" is larger than the cluster size. It can hold
// PAGESIZE bytes. This is for convenience writing data to the device and
// calculating the crc. The work area after clustersize bytes is ignored
// by the caller
int devio_postcluster(struct etfs_devio *dev, unsigned cluster, uint8_t *buf, struct etfs_trans *trp) {

	struct spare		*sp;
	uint32_t			device_status;
	unsigned			page = cluster;
	CHIPIO				*cio = dev->cio;

	// Build spare area
	sp = (struct spare *) (buf + DATASIZE);
	memset((void *)sp, 0xff, SPARESIZE);

	if(trp) {
		sp->sequence   = ENDIAN_LE32(trp->sequence);
		sp->fid        = ENDIAN_LE16((uint16_t) trp->fid);
		sp->clusterlo  = ENDIAN_LE16((uint16_t) trp->cluster);
		sp->clusterhi  = trp->cluster >> 16;
		sp->nclusters  = trp->nclusters;
		sp->status     = 0xff;
		sp->status2    = 0xff;
		sp->crcdata    = dev->crc32(buf, DATASIZE);

		nand_bch_calculate(cio, ECC_DATA, buf, (uint8_t *)sp + DATA_ECCV_OFFSET);
		*(uint32_t *)((uint8_t *)sp + SPARE_CRC_OFFSET) = dev->crc32((uint8_t *) sp, DATA_ECCV_OFFSET + cio->data_ecc_sz);

		if(cluster % dev->clusters2blk == 0) {
		    sp->erasesig[0] = ~0;	// Can only punch bits down once and we did it
			sp->erasesig[1] = ~0;	// on the erase
		}
		
		nand_bch_calculate(cio, ECC_SPARE, (uint8_t *)sp, (uint8_t *)sp + SPARE_ECCV_OFFSET);
	}
	//write page
    if (nand_write_page(cio, page, buf, PAGESIZE, &device_status) ||
	   (device_status & NAND_PROGRAM_ERASE_ERROR)) {
	       dev->log(_SLOG_CRITICAL, "Timeout / Program Error on POST DATA1");
		   return(ETFS_TRANS_DEVERR);
	}
	
	return(ETFS_TRANS_OK);
}


// Erase a block
int devio_eraseblk(struct etfs_devio *dev, unsigned blk) {

	uint32_t		device_status = 0;
	CHIPIO			*cio = dev->cio;
	uint32_t		page = blk * PAGES2BLK;
	
	if(nand_erase_page(cio, page, &device_status) != 0)
		dev->log(_SLOG_CRITICAL, "Timeout on ERASE");

	// Check for erase error
	if(device_status & NAND_PROGRAM_ERASE_ERROR) {
		dev->log(_SLOG_ERROR, "devio_eraseblk: erase error on blk %d (0x%x)", blk, device_status);
		return(ETFS_TRANS_DEVERR);
	}

	return(0);
}


// Called to allow the driver to flush any cached data that
// has not been written to the device. The NAND class driver does not need it
 int devio_sync(struct etfs_devio *dev) {
	return(-1);
}

