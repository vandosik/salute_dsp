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


iofunc_funcs_t			_elcore_ocb_funcs = { _IOFUNC_NFUNCS, _elcore_ocb_calloc, _elcore_ocb_free };
resmgr_io_funcs_t		_elcore_io_funcs;
resmgr_connect_funcs_t	_elcore_connect_funcs;
iofunc_mount_t			_elcore_mount = { 0, 0, 0, 0, &_elcore_ocb_funcs };

int _elcore_init_iofunc(void)
{printf("%s()\n", __func__);
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &_elcore_connect_funcs, _RESMGR_IO_NFUNCS, &_elcore_io_funcs);
	_elcore_io_funcs.read      = _elcore_read;
	_elcore_io_funcs.write     = _elcore_write;
// 	_elcore_io_funcs.devctl    = _elcore_devctl;
// 	_elcore_io_funcs.close_ocb = _elcore_close_ocb;
// 	_elcore_io_funcs.msg       = _elcore_iomsg;

	return EOK;
}
