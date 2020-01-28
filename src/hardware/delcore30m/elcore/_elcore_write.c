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

int
_elcore_write(resmgr_context_t *ctp, io_write_t *msg, elcore_ocb_t *ocb)
{printf("%s()\n", __func__);
	uint8_t		*buf;
	int			nonblock;
	int			nbytes, status;
	int			nleft;
	ELCORE_DEV		*drvhdl = (ELCORE_DEV *)ocb->hdr.attr;
	elcore_dev_t	*dev = drvhdl->hdl;

    if ((status = iofunc_write_verify(ctp, msg, &ocb->hdr, &nonblock)) != EOK)
        return status;

    if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
        return ENOSYS;

    //check the "end" of the file
    nleft = ocb->hdr.attr->nbytes - ocb->hdr.offset; 
    nbytes = min (msg->i.nbytes, nleft);
	
    if (nbytes <= 0) {
//         _IO_SET_WRITE_NBYTES(ctp, 0);
//         return _RESMGR_NPARTS(0);
	return EIO;
    }

    /* check if message buffer is too short */
    if ((sizeof(msg->i) + nbytes) > ctp->msg_max_size) {
        if (dev->buflen < nbytes) {
            dev->buflen = nbytes;
			if (dev->buf)
            	free(dev->buf);
            if (NULL == (dev->buf = malloc(dev->buflen))) {
                dev->buflen = 0;
                return ENOMEM;
            }
        }

        status = resmgr_msgread(ctp, dev->buf, nbytes, sizeof(msg->i));
        if (status < 0)
            return errno;
        if (status < nbytes)
            return EFAULT;

        buf = dev->buf;
    }
	else
        buf = ((uint8_t *)msg) + sizeof(msg->i);

	nbytes =  dev->funcs->write(drvhdl, ocb->core, buf, (void*)((uintptr_t)(ocb->hdr.offset)), nbytes);
	
	ocb->hdr.offset += nbytes;

// 	if (nbytes == 0)
// 		return EAGAIN;

	if (nbytes > 0) {
		_IO_SET_WRITE_NBYTES(ctp, nbytes);
		return _RESMGR_NPARTS(0);
	}

	return EIO;

//     int     status;
//     char    *buf;
// 
//     if ((status = iofunc_write_verify(ctp, msg, &ocb->hdr, NULL)) != EOK)
//         return (status);
// 
//     if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
//         return(ENOSYS);
// 
//     /* set up the number of bytes (returned by client's write()) */
// 
//     _IO_SET_WRITE_NBYTES (ctp, msg->i.nbytes);
// 
//     buf = (char *) malloc(msg->i.nbytes + 1);
//     if (buf == NULL)
//         return(ENOMEM);
// 
//     /*
//      *  Reread the data from the sender's message buffer.
//      *  We're not assuming that all of the data fit into the
//      *  resource manager library's receive buffer.
//      */
// 
//     resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i));
//     buf [msg->i.nbytes] = '\0'; /* just in case the text is not NULL terminated */
//     printf ("Received %d bytes = '%s'\n", msg -> i.nbytes, buf);
//     free(buf);
// 
//     if (msg->i.nbytes > 0)
//         ocb->hdr.attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
// 
//     return (_RESMGR_NPARTS (0));
}
