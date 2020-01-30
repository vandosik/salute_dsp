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
// 		case DCMD_SPI_SET_CONFIG:
// 		{
// 			spi_cfg_t	*cfg;
// 			uint32_t	device;
// 
// 			if (msg->i.nbytes < (sizeof(spi_cfg_t) + sizeof(uint32_t)))
// 				return EINVAL;
// 
// 			device = *(uint32_t *)_DEVCTL_DATA(msg->i);
// 			cfg    = (spi_cfg_t *)((uint8_t *)_DEVCTL_DATA(msg->i) + sizeof(uint32_t));
// 
// 			if ((device & SPI_DEV_ID_MASK) == SPI_DEV_ID_NONE)
// 				return EINVAL;
// 
// 			/*
// 			 * Client wants to change default device
// 			 */
// 			if (device & SPI_DEV_DEFAULT)
// 				ocb->chip = device;
// 
// 			/*
// 			 * Wants to set configuration for this device?
// 			 */
// 			if (cfg->mode != 0) {
// 				if (dev->funcs->setcfg == NULL)
// 					return ENOTSUP;
// 			
// 				status = dev->funcs->setcfg(drvhdl, device, cfg);
// 				if (status != EOK)
// 					return EINVAL;
// 			}
// 
// 			memset(&msg->o, 0, sizeof(msg->o));
// 			msg->o.ret_val = 0;
// 			msg->o.nbytes = 0;
// 			return _RESMGR_PTR(ctp, msg, sizeof(msg->o));
// 		}
// 
// 		case DCMD_SPI_GET_DEVINFO:
// 		{
// 			uint32_t		device;
// 			spi_devinfo_t	*info;
// 			spi_lock_t		*lock;
// 
// 			if (msg->i.nbytes < sizeof(uint32_t))
// 				return EINVAL;
// 
// 			if (dev->funcs->devinfo == NULL)
// 				return ENOTSUP;
// 
// 			device = *((uint32_t *)_DEVCTL_DATA(msg->i));
// 
// 			info = (spi_devinfo_t *)_DEVCTL_DATA(msg->o);
// 			status = dev->funcs->devinfo(drvhdl, device, info);
// 			if (status != EOK)
// 				return EINVAL;
// 
// 			if ((lock = _spi_islock(ctp, device, ocb)) != NULL) {
// 				if (lock->owner != ocb)
// 					info->cfg.mode |= SPI_MODE_LOCKED;
// 			}
// 
// 			msg->o.nbytes = sizeof(spi_devinfo_t);
// 
// 			return _RESMGR_PTR(ctp, msg, sizeof(msg->o) + sizeof(spi_devinfo_t));
// 		}
// 
// 		case DCMD_SPI_GET_DRVINFO:
// 		{
// 			spi_drvinfo_t	*info;
// 
// 			if (dev->funcs->drvinfo == NULL)
// 				return ENOTSUP;
// 
// 			info = (spi_drvinfo_t *)_DEVCTL_DATA(msg->o);
// 
// 			status = dev->funcs->drvinfo(drvhdl, info);
// 			if (status != EOK)
// 				return EINVAL;
// 
// 			msg->o.nbytes = sizeof(spi_drvinfo_t);
// 			return _RESMGR_PTR(ctp, msg, sizeof(msg->o) + sizeof(spi_drvinfo_t));
// 		}
	}
  memset(&msg->o, 0, sizeof(msg->o));
  msg->o.ret_val = status;
  msg->o.nbytes = nbytes;
  return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));

// 	return ENOSYS;
}
