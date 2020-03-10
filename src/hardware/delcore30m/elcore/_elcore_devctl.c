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
_elcore_devctl(resmgr_context_t *ctp, io_devctl_t *msg, elcore_ocb_t *ocb)
{printf("%s()\n", __func__);
	int			status;
    int			nbytes = 0;
	ELCORE_DEV		*drvhdl = (ELCORE_DEV *)ocb->hdr.attr;
	elcore_dev_t	*dev = drvhdl->hdl;
	void* devctl_data;
	
	status = iofunc_devctl_default(ctp, msg, &ocb->hdr);
	if (status != _RESMGR_DEFAULT)
		return status;

	devctl_data = _DEVCTL_DATA(msg->i);
	
	switch (msg->i.dcmd) {
		case DCMD_ELCORE_START:
		{
			status = dev->funcs->start_core(drvhdl, ocb->core);
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_STOP:
		{
			status = dev->funcs->stop_core(drvhdl, ocb->core);
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_PRINT:
		{
			status = dev->funcs->print(drvhdl);
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_SEND:
		{
			elcore_send_t	*send_cfg;
			void			*send_buf;

			send_cfg = (elcore_send_t*)devctl_data;
			send_buf = (void*)((uint8_t*)devctl_data + sizeof(elcore_send_t));
			
			ocb->core = send_cfg->core;
			
			status = dev->funcs->write(drvhdl, ocb->core, send_buf, (void*)send_cfg->offset, send_cfg->len);
			
			if (status >= 0)
			{
				status = EOK;
			}
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_RECV:
		{
			elcore_recv_t	*recv_cfg;
			void			*recv_buf;

			recv_cfg = (elcore_recv_t*)devctl_data;
			recv_buf = (void*)((uint8_t*)devctl_data + sizeof(elcore_recv_t));
			
			ocb->core = recv_cfg->core;
			
			status = dev->funcs->read(drvhdl, ocb->core, recv_buf, (void*)recv_cfg->offset, recv_cfg->len);
			
			nbytes = status + sizeof(elcore_recv_t);

			if (status >= 0)
			{
				status = EOK;
			}
			else
			{
				return EINVAL;
			}
			
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_DMASEND:
		{
			elcore_dmasend_t	*dma_send;

			dma_send = (elcore_dmasend_t*)devctl_data;
			ocb->core = dma_send->core;
			
            printf("%s: dma_send src: 0x%08x\n", __func__, dma_send->dma_src);
			status = dev->funcs->dma_send(drvhdl, ocb->core, dma_send->dma_src, dma_send->offset, dma_send->len);
            
			if (status >= 0)
			{
				status = EOK;
			}
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_DMARECV:
		{
			elcore_dmarecv_t	*dma_recv;

			dma_recv = (elcore_dmarecv_t*)devctl_data;
			ocb->core = dma_recv->core;
			
            printf("%s: dma_recv src: 0x%08x\n", __func__, dma_recv->dma_dst);
			status = dev->funcs->dma_recv(drvhdl, ocb->core, dma_recv->dma_dst, dma_recv->offset, dma_recv->len);
            
			nbytes = sizeof(elcore_dmarecv_t);

			if (status >= 0)
			{
				status = EOK;
			}
			else
			{
				return EINVAL;
			}
			
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_JOB_STATUS:
		{
			uint32_t    job_id = *((uint32_t*)devctl_data);
			/*TODO: need select job from some kind of list (queue)*/
			elcore_job_t*			cur_job;
			
			if ((cur_job = get_enqueued_by_id(drvhdl, job_id)) == NULL)
			{
				if ((cur_job = get_stored_by_id(drvhdl, job_id)) == NULL)
				{
					return EINVAL;
				}
			}
			

			*((uint32_t*)devctl_data) = cur_job->job_pub.status;
			
			status = EOK;
			nbytes = sizeof(uint32_t);
					

			break;
		}
		case DCMD_ELCORE_JOB_WAIT:
		{
			uint32_t    job_id = *((uint32_t*)devctl_data);
			/*TODO: need select job from some kind of list (queue)*/
			elcore_job_t*			cur_job;
			
			if ((cur_job = get_enqueued_by_id(drvhdl, job_id)) == NULL)
			{
				if ((cur_job = get_stored_by_id(drvhdl, job_id)) == NULL)
				{
					return EINVAL;
				}
			}

			if (cur_job->job_pub.status == ELCORE_JOB_RUNNING)
			{
				cur_job->rcvid = ctp->rcvid;
				
				return (_RESMGR_NOREPLY);
			}
			//if job is ready, we act like as NONBLOCK 
			*((uint32_t*)devctl_data) = cur_job->job_pub.rc;
				
			status = EOK;
			nbytes = sizeof(uint32_t);

			break;
		}
		default:
		{
			status = dev->funcs->ctl(drvhdl, msg->i.dcmd, devctl_data, 
			                 msg->i.nbytes, &nbytes, /*&info*/ NULL);
			if (status >= 0)
			{
				status = EOK;
			}
			else
			{
				return EINVAL;
			}

// 			memset(&msg->o, 0, sizeof(msg->o));
// 			msg->o.ret_val = info;
// 			msg->o.nbytes = nbytes;
// 			return _RESMGR_PTR(ctp, msg, sizeof(msg->o) + nbytes);

		}
	}
  memset(&msg->o, 0, sizeof(msg->o));
  msg->o.ret_val = status;
  msg->o.nbytes = nbytes;
  return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));

// 	return ENOSYS;
}
