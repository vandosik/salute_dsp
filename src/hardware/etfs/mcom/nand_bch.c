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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <gulliver.h>

struct  chipio;
#define CHIPIO	struct chipio
#include "devio.h"
#include "chipio.h"

/* 
 * Interface between QNX etfs driver and the Bose-Chaudhuri-Hocquenghem (BCH) codes GNU implementation 
 */
extern struct chipio chipio;


static int nand_bch_calculate_ecc(struct bch_nand_control *bnc, const unsigned char *buf,
			   unsigned char *code)
{
	unsigned int i;

	memset(code, 0, bnc->ecc_bytes);
	encode_bch(bnc->bch, buf, bnc->ecc_size, code);

	/* apply mask so that an erased page is a valid codeword */
	for (i = 0; i < bnc->ecc_bytes; i++)
		code[i] ^= bnc->ecc_mask[i];

	return 0;
}

int nand_bch_calculate(struct chipio *cio, int dtype, const unsigned char *buf, unsigned char *code)
{
	struct bch_nand_control *bnc;
	int 				i, j;
	unsigned 			eccsteps;
	
	if (dtype == ECC_SPARE)
		bnc = &cio->spare_bch;
	else
		bnc = &cio->data_bch;
	
	eccsteps = bnc->ecc_steps;

	for (i = 0, j = 0; eccsteps; eccsteps--, i += bnc->ecc_bytes, j += bnc->ecc_size)
		nand_bch_calculate_ecc(bnc, &buf[j], &code[i]);
	
	return 0;
}

static int nand_bch_correct_data(struct bch_nand_control *bnc, unsigned char *buf,
			  unsigned char *read_ecc, unsigned char *calc_ecc)
{
	unsigned int *errloc = bnc->ecc_errloc;
	int i, count;

	count = decode_bch(bnc->bch, buf, bnc->ecc_size, read_ecc, calc_ecc,
			   NULL, errloc);
	if (count > 0) {
		for (i = 0; i < count; i++) {
			if (errloc[i] < (bnc->ecc_size*8))
				/* error is located in data, correct it */
				buf[errloc[i] >> 3] ^= (1 << (errloc[i] & 7));
			/* else error in ecc, no action needed */
		}
	} 
	else 
		if (count < 0) {
//			fprintf( stderr, "nand_bch_correct_data: ecc unrecoverable error count %d\n", count);
			count = -EBADMSG;
		}
		
	return count;
}

int nand_bch_correct(struct chipio *cio, int dtype, unsigned char *buf, unsigned char *read_ecc)
{
	struct bch_nand_control *bnc;
	int				err = 0;
	int 			ret;
	int 			i, j;
	unsigned		eccsteps;
	unsigned char	*calc_ecc = alloca(cio->chip.sparesize);
	
	if (dtype == ECC_SPARE)
		bnc = &cio->spare_bch;
	else
		bnc = &cio->data_bch;
	
	eccsteps = bnc->ecc_steps;
	nand_bch_calculate(cio, dtype, buf, &calc_ecc[0]);
	
	for (i = 0, j = 0; eccsteps; eccsteps--, i += bnc->ecc_bytes, j += bnc->ecc_size) {
		ret = nand_bch_correct_data(bnc, &buf[j], &read_ecc[i], &calc_ecc[i]);
		if (ret < 0)
			err++;
	}
		
	return err;
}

void nand_bch_free(struct bch_nand_control *bnc)
{
	if (bnc->bch) {
		free_bch(bnc->bch);
		free(bnc->ecc_errloc);
		free(bnc->ecc_mask);
	}
}

int nand_bch_init(struct bch_nand_control *bnc)
{
	unsigned		m, t, i;
	unsigned char	*erased_page;
	unsigned		eccsize = bnc->ecc_size;
	unsigned		eccbytes = bnc->ecc_bytes;
	unsigned		eccstrength = bnc->ecc_strength;

	if (!eccbytes && eccstrength) {
		eccbytes = DIV_ROUND_UP(eccstrength * fls(8 * eccsize), 8);
		bnc->ecc_bytes = eccbytes;
	}

	if (!eccsize || !eccbytes) {
		fprintf( stderr, "nand_bch_control: ecc parameters not supplied\n");
		goto fail;
	}

	m = fls(8 * eccsize + 1);
	t = (eccbytes * 8) / m;

	bnc->bch = init_bch(m, t, 0);
	if (!bnc->bch)
		goto fail;

	/* verify that eccbytes has the expected value */
	if (bnc->bch->ecc_bytes != eccbytes) {
		fprintf( stderr, "invalid eccbytes %u, should be %u\n",
		       eccbytes, bnc->bch->ecc_bytes);
		goto fail;
	}


	/* sanity checks */
	if (8 * (eccsize + eccbytes) >= (1 << m)) {
		fprintf( stderr, "eccsize %u is too large\n", eccsize);
		goto fail;
	}


	bnc->ecc_mask = malloc(eccbytes);
	bnc->ecc_errloc = malloc(t*sizeof(*bnc->ecc_errloc));
	if (!bnc->ecc_mask || !bnc->ecc_errloc)
		goto fail;
	/*
	 * compute and store the inverted ecc of an erased ecc block
	 */
	erased_page = malloc(eccsize);
	if (!erased_page)
		goto fail;

	memset(erased_page, 0xff, eccsize);
	memset(bnc->ecc_mask, 0, eccbytes);
	encode_bch(bnc->bch, erased_page, eccsize, bnc->ecc_mask);
	free(erased_page);

	for (i = 0; i < eccbytes; i++)
		bnc->ecc_mask[i] ^= 0xff;

	return 0;
	
fail:
	nand_bch_free(bnc);
	return -1;
}
