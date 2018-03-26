/*
 * $QNXLicenseC:  
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/

// Module Description: data store 

#ifndef __HIDDEN_IO_USBDATACACHE_H_INCLUDED
#define __HIDDEN_IO_USBDATACACHE_H_INCLUDED

// private headers
#include <stdio.h>
#include <queue.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/mman.h>


/* structure to store data keys, buffers of data */

typedef struct _usb_data_key {
	uint32_t				flags;
	uint32_t				len;
	uint32_t				key_type;		// standard USB request (definable)
	uint32_t				value;
	uint32_t				index;
	uint32_t				qualifier;
	void					*data_buf;		// data pointer
	uint32_t				data_len;
} usb_data_key_t;

/* key flags */
#define DCACHE_FLAGS_DATA_SHORT_XFER		0x01

/* defined key types */
#define DCACHE_KEYTYPE_STANDARD_USB 		0x01


typedef struct _usb_ctrl_data_key {
	TAILQ_ENTRY( _usb_ctrl_data_key) 	link;
	usb_data_key_t 						data_key;
} usb_ctrl_data_key_t;


typedef struct _usb_data_store_ctrl {
	uint32_t							flags;
	TAILQ_HEAD(,_usb_ctrl_data_key)		data_keys;
} usb_data_store_ctrl_t;


/* prototypes */
extern int usb_data_store_init( usb_data_store_ctrl_t **dctrl_hdl, uint32_t flags );
extern int usb_free_key_data( usb_data_store_ctrl_t *dctrl );
extern int usb_store_data_key( usb_data_store_ctrl_t *dctrl, uint32_t flags, uint32_t type, uint32_t value, uint32_t index, uint32_t qualifier, uint8_t *buffer, uint32_t len );
extern int usb_remove_data_key( usb_data_store_ctrl_t *dctrl, usb_ctrl_data_key_t *key );
extern usb_ctrl_data_key_t *usb_find_data_key( usb_data_store_ctrl_t *rctrl, uint32_t type, uint32_t value, uint32_t index, uint32_t qualifier, uint8_t *buffer, uint32_t *len );
extern usb_ctrl_data_key_t *usb_find_key( usb_data_store_ctrl_t *rctrl, uint32_t type, uint32_t value, uint32_t index, uint32_t qualifier );
extern int usb_data_store_free( usb_data_store_ctrl_t *dctrl_hdl );

#endif


__SRCVERSION( "$URL: http://community.qnx.com/svn/repos/internal-outsourcing/trunk/services/usb/io-usb/data_cache.h $ $Rev: 373 $" )
