/*
 * $QNXtpLicenseC:  
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

// Portions Copyright 1999, Thierry Giron, All rights Reserved.

// Module Description:  Error equivalences

#ifndef __ERROR_H_INCLUDED
#define __ERROR_H_INCLUDED

//                ERROR ASSOCIATED WITH USB CONTROLLER                                                       
#define ERROR_USB_NO_CONTROLLER			0x8000
#define	ERROR_USB_INIT_DATA				0x8001
#define	ERROR_USB_INIT					0x8100
#define	ERROR_USB_CONTROLLER_BAD		0x8101
#define	ERROR_USB_INIT_NO_CONTROLLER	0x8102

//                ERROR ASSOCIATED WITH CLIENT REQUESTS
#define ERROR_WRONG_DEVICE_NUMBER		0x8200
#define ERROR_NO_MORE_USBDEVICES  	 	0x8201
#define ERROR_DEVICE_NOT_CONFIGURED		0x8202
#define ERROR_TOO_MANY_DEVICES  	   	0x8203
#define ERROR_WRONG_DEVICE_CONFIG		0x8204
#define ERROR_WRONG_DEVICE_SELECTION	0x8205
#define ERROR_WRONG_DEVICE_INTERFACE	0x8206
#define ERROR_WRONG_DEVICE_ENDPOINT 	0x8207

//                ERROR ASSOCIATED WITH USB TRANSFERS
#define	ERROR_IN_TRANSFER				0x8300
#define	ERROR_IN_TRANSFER_SETUP			0x8301
#define	ERROR_IN_TRANSFER_DATA			0x8302
#define	ERROR_IN_TRANSFER_STATUS		0x8303
#define	ERROR_SIZE_TOO_BIG				0x8304

//                ERROR ASSOCIATED WITH MEMORY
#define ERROR_NO_MEMORY					0x9000
#define	ERROR_NOT_ENOUGH_TD				0x9001
#define	ERROR_NOT_ENOUGH_ED				0x9002
#define	ERROR_NO_CLASS_DRIVER_MEMORY	0x9003

//                ERROR ASSOCIATED WITH DEVICE CONFIGURATION
#define	ERROR_DEVICE_DESCRIPTOR			0x8400
#define	ERROR_DEVICE_NOT_ENUMERATED		0x8401
#define	ERROR_NO_CLASS_DRIVER       	0x8402
#define	ERROR_FIRST_ENUMERATION_FAIL	0x8403

//                	ERROR ASSOCIATED WITH HUB
#define ERROR_HUB_SET_POWER				0x8500

//                	ERROR ASSOCIATED WITH ISOCHRONOUS TRANSFERS
#define	ERROR_ISOCH_OUT_OF_SYNCH		0x8600
#define	ERROR_ISOCH_LAST_OUT_OF_SYNCH	0x8601

#endif

__SRCVERSION( "$URL: http://community.qnx.com/svn/repos/internal-outsourcing/trunk/services/usb/hcd/error.h $ $Rev: 373 $" )
