/*
 * $QNXLicenseC:  
 * Copyright 2005, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/


// This source code contains confidential information of QNX Software Systems
// Ltd. (QSSL). Any use, reproduction, modification, disclosure, distribution
// or transfer of this software, or any software which includes or is based
// upon any of this code, is only permitted under the terms of the QNX
// Confidential Source License version 1.0 (see licensing.qnx.com for details)
// or as otherwise expressly authorized by a written license agreement from
// QSSL. For more information, please email licensing@qnx.com.

// Module Description:  Header for ccache.c

#ifndef __CCACHE_H_INCLUDED
#define __CCACHE_H_INCLUDED

/* 
	structure to store mapped client buffer addresses
*/

typedef struct _ccache_buffer {
    struct _ccache_buffer 	*next;
    void                    *baddr;
    _uint32 				blen;
	void					*handle;	
    void 					*mapped_addr;
} ccache_buffer;


#endif


__SRCVERSION( "$URL: http://community.qnx.com/svn/repos/internal-outsourcing/trunk/services/usb/hcd/ccache.h $ $Rev: 373 $" )
