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

#define ELCORE_MAX_TRANSFER				0x10000

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

	switch (msg->i.dcmd) 
	{
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
			int				send_len;

			send_cfg = (elcore_send_t*)devctl_data;
			send_buf = (void*)((uint8_t*)devctl_data + sizeof(elcore_send_t));
			send_len = send_cfg->len;
			
			ocb->core = send_cfg->core;
			
			dev->funcs->write(drvhdl, ocb->core, send_buf, (void*)send_cfg->offset, &send_len);
			
			if (send_len >= 0)
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
		case DCMD_ELCORE_RECV:
		{
			elcore_recv_t	*recv_cfg;
			void			*recv_buf;
			int				recv_len;

			recv_cfg = (elcore_recv_t*)devctl_data;
			recv_buf = (void*)((uint8_t*)devctl_data + sizeof(elcore_recv_t));
			recv_len = recv_cfg->len;
			
			ocb->core = recv_cfg->core;
			
			status = dev->funcs->read(drvhdl, ocb->core, recv_buf, (void*)recv_cfg->offset, &recv_len);

			if (recv_len >= 0)
			{
				status = EOK;
			}
			else
			{
				return EINVAL;
			}
			
			nbytes = recv_len + sizeof(elcore_recv_t);
			
// 			msg->o.ret_val = status;
// 			msg->o.nbytes = nbytes;
			break;
		}
		case DCMD_ELCORE_DMASEND:
		{
			elcore_dmasend_t	*dma_send;

			dma_send = (elcore_dmasend_t*)devctl_data;
			ocb->core = dma_send->core;
			int send_len = dma_send->len;
			
            printf("%s: dma_send src: 0x%08x\n", __func__, dma_send->dma_src);
			dev->funcs->dma_send(drvhdl, ocb->core, dma_send->dma_src, dma_send->offset, &send_len);
            
			if (send_len >= 0)
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
		case DCMD_ELCORE_DMARECV:
		{
			elcore_dmarecv_t	*dma_recv;

			dma_recv = (elcore_dmarecv_t*)devctl_data;
			ocb->core = dma_recv->core;
			int recv_len = dma_recv->len;
			
            printf("%s: dma_recv src: 0x%08x\n", __func__, dma_recv->dma_dst);
			status = dev->funcs->dma_recv(drvhdl, ocb->core, dma_recv->dma_dst, dma_recv->offset, &recv_len);
            
			nbytes = sizeof(elcore_dmarecv_t);

			if (recv_len >= 0)
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
		case DCMD_ELCORE_JOB_CREATE:
		{
			ELCORE_JOB		*new_pub_job = (ELCORE_JOB*)devctl_data;
			elcore_job_t	*new_job;
			int				it;
			
			//FIXME: check this at another place
			if (new_pub_job->inum > MAX_INPUTS || new_pub_job->onum > MAX_OUTPUTS || new_pub_job->core > 2)
			{
				return EINVAL;
			}
			
			if ((new_job = alloc_job(drvhdl, new_pub_job)) == NULL)
			{
				return ENOMEM;
			}

			if ((status = dev->funcs->set_prog(drvhdl, new_job)) != EOK)
			{
			    return status;
			}
			
			if ((status = dev->funcs->set_data(drvhdl, new_job)) != EOK)
			{
			    return status;
			}
			
			*((ELCORE_JOB*)devctl_data) = new_job->job_pub;
			
			status = EOK;
			nbytes = sizeof(ELCORE_JOB);
			
			break;
		}
		case DCMD_ELCORE_JOB_ENQUEUE:
		{
			uint32_t			job_id = *((uint32_t*)devctl_data);
			elcore_job_t*		cur_job;
			
			if ((cur_job = get_stored_by_id(drvhdl, job_id)) == NULL)
			{
				return EINVAL;
			}
			
			if (job_enqueue( drvhdl, cur_job ))
			{
				return EINVAL;
			}
			
			//this func gets firs job in the queue
			dev->funcs->start_core(drvhdl, cur_job->job_pub.core);
			
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
			{	//do not respond to client, it blocks waiting on finishing job
				cur_job->rcvid = ctp->rcvid;
				
				return (_RESMGR_NOREPLY);
			}
			//if job is ready, we act like as NONBLOCK 
			*((uint32_t*)devctl_data) = cur_job->job_pub.rc;
				
			status = EOK;
			nbytes = sizeof(uint32_t);

			break;
		}
		case DCMD_ELCORE_JOB_RESULTS:
		{
			uint32_t    job_id = *((uint32_t*)devctl_data);
			/*TODO: need select job from some kind of list (queue)*/
			elcore_job_t*			cur_job;
			int						it;
			
			if ((cur_job = get_enqueued_by_id(drvhdl, job_id)) == NULL)
			{
				if ((cur_job = get_stored_by_id(drvhdl, job_id)) == NULL)
				{
					return EINVAL;
				}
			}

			if (cur_job->job_pub.status == ELCORE_JOB_RUNNING)
			{
//                 status = EOK;
//                 nbytes
				return EBUSY;
			}
			
			status = dev->funcs->get_data(drvhdl, cur_job);

			break;
		}
		case DCMD_ELCORE_JOB_CANCEL:
		{
			uint32_t    job_id = *((uint32_t*)devctl_data);
			/*TODO: need select job from some kind of list (queue)*/
			elcore_job_t*			cur_job;
			
			if ((cur_job = get_enqueued_by_id(drvhdl, job_id)) == NULL)
			{
				return EINVAL;
			}

			if (cur_job->job_pub.status == ELCORE_JOB_RUNNING)
			{
				dev->funcs->reset_core(drvhdl, cur_job->job_pub.core);
			}

			job_remove_from_queue(drvhdl, cur_job);
			//if cancel running job - try run new
			if (cur_job->job_pub.status == ELCORE_JOB_RUNNING)
			{
				dev->funcs->start_core(drvhdl, cur_job->job_pub.core);
			}
			
			cur_job->job_pub.rc = ELCORE_JOB_CANCELLED;
			cur_job->job_pub.status = ELCORE_JOB_IDLE;
			

			break;
		}
		case DCMD_ELCORE_JOB_RELEASE:
		{
			uint32_t    job_id = *((uint32_t*)devctl_data);
			/*TODO: need select job from some kind of list (queue)*/
			elcore_job_t*			cur_job;
			
			if ((cur_job = get_enqueued_by_id(drvhdl, job_id)) != NULL)
			{	//first remove from queue
				return EBUSY;
			}

			
			if ((cur_job = get_stored_by_id(drvhdl, job_id)) == NULL)
			{
				return EINVAL;
			}
			//free mem resources on dsp
			dev->funcs->release_mem(drvhdl, cur_job);
			
			//TODO: if cancel running job - try run new
			if (release_job(drvhdl, cur_job))
			{
				return EINVAL;
			}
			

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
