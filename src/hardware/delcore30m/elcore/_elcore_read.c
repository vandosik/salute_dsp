/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
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




#include "proto.h"


static uint8_t                     *buffer = "Hello world\n";

int
_elcore_read(resmgr_context_t *ctp, io_read_t *msg, elcore_ocb_t *ocb)
{
	uint8_t		*buf;
	int			nbytes;
    int			nleft;
	int			nonblock = 0;
	int			status;
	ELCORE_DEV		*drvhdl = (ELCORE_DEV *)ocb->hdr.attr;
	elcore_dev_t	*dev = drvhdl->hdl;

    if ((status = iofunc_read_verify(ctp, msg, &ocb->hdr, &nonblock)) != EOK)
        return status;

    if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
        return ENOSYS;
    
    nleft = ocb->hdr.attr->nbytes - ocb->hdr.offset;
    nbytes = min (msg->i.nbytes, nleft);

    if (msg->i.nbytes <= 0) {
        _IO_SET_READ_NBYTES(ctp, 0);
        return _RESMGR_NPARTS(0);
    }

    
    /* check if message buffer is too short */
    nbytes = msg->i.nbytes;
    if (nbytes > ctp->msg_max_size) {
        if (dev->buflen < nbytes) {
            dev->buflen = nbytes;
			if (dev->buf)
            	free(dev->buf);
            if (NULL == (dev->buf = malloc(dev->buflen))) {
                dev->buflen = 0;
                return ENOMEM;
            }
        }
        buf = dev->buf;
		ocb->hdr.offset += nbytes;
    }
	else {
        buf = (uint8_t *)msg;
    }

   
    
    /*nbytes*/ = strlen(buffer)+1;
	nbytes = dev->funcs->read(drvhdl, buf, (void*)((uintptr_t)(ocb->hdr.offset)));

	if (nbytes > 0) {
		_IO_SET_READ_NBYTES(ctp, nbytes);
		return _RESMGR_PTR(ctp, buf, nbytes);
	}

	return EIO;
    
//         int         nleft;
//         int         nbytes;
//         int         nparts;
//         int         status;
// 
//         if ((status = iofunc_read_verify (ctp, msg, &ocb->hdr, NULL)) != EOK)
//             return (status);
//             
//         if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
//             return (ENOSYS);
// 
//         /*
//          *  On all reads (first and subsequent), calculate
//          *  how many bytes we can return to the client,
//          *  based upon the number of bytes available (nleft)
//          *  and the client's buffer size
//          */
// 
//         nleft = ocb->hdr.attr->nbytes - ocb->hdr.offset;
//         nbytes = min (msg->i.nbytes, nleft);
// 
//         if (nbytes > 0) {
//             /* set up the return data IOV */
//             SETIOV (ctp->iov, buffer + ocb->hdr.offset, nbytes);
// 
//             /* set up the number of bytes (returned by client's read()) */
//             _IO_SET_READ_NBYTES (ctp, nbytes);
// 
//             /*
//              * advance the offset by the number of bytes
//              * returned to the client.
//              */
// 
//             ocb->hdr.offset += nbytes;
//             
//             nparts = 1;
//         } else {
//             /*
//              * they've asked for zero bytes or they've already previously
//              * read everything
//              */
//             
//             _IO_SET_READ_NBYTES (ctp, 0);
//             
//             nparts = 0;
//         }
// 
//         /* mark the access time as invalid (we just accessed it) */
// 
//         if (msg->i.nbytes > 0)
//             ocb->hdr.attr->flags |= IOFUNC_ATTR_ATIME;
// 
//         //return (_RESMGR_PTR(ctp, buffer, nbytes));
// 		return (_RESMGR_NPARTS (nparts));
}

