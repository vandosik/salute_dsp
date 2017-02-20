/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
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

/*
 * NAND flash memory chip interface
 */
#ifndef __CHIPIO_H__
#define __CHIPIO_H__

#include "devio.h"
#include "bch.h"

#ifndef CHIPIO
#define CHIPIO	struct chipio
#endif

/* BCH control structure used in low level board driver */
struct bch_nand_control {
	unsigned					ecc_steps;
	unsigned					ecc_size;
	unsigned					ecc_bytes;
	unsigned					ecc_strength;
	struct bch_control			*bch;
	unsigned int				*ecc_errloc;
	unsigned char				*ecc_mask;
};

#define BCH  struct bch_nand_control
#define ECC_SPARE     0
#define ECC_DATA      1

/* 
 * The chipio structure defined in the low level board driver. 
 */
struct chipio {
	struct _chipio				chip;
	uint64_t					phy_base; //base address of the ELB registers (physical)
	unsigned					iobase;   //base address of the ELB registers (virtual)
	unsigned 					use_polling;
	unsigned 					use_dma;
	uint8_t						*dma_area;
	off64_t						dma_paddr;
	int							chid;
	int							coid;
	unsigned 					irq_num;
	int							irq_id;
	int 						badblk[16]; // do bad block chek
	struct bch_nand_control		data_bch;
	struct bch_nand_control		spare_bch;
};

/* 
 * The chip_info structure describes NAND chip main params. 
 */
struct chip_info {
	uint32_t					vid;
	uint32_t					did;
	char						*model;
	unsigned					nblks;
	unsigned 					datasize;
	unsigned 					sparesize;
	unsigned 					pages2blk;
	unsigned 					ecc_bits;
	unsigned 					ecc_codeword_sz;
};

int nand_bch_init(BCH *cio);
void nand_bch_free(BCH *cio);
int nand_bch_calculate(CHIPIO *cio, int dtype, const unsigned char *buf, unsigned char *code);
int nand_bch_correct(CHIPIO *cio, int dtype, unsigned char *buf,unsigned char *read_ecc);

#endif // __CHIPIO_H__
