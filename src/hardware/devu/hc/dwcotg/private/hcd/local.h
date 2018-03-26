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


#ifndef _HCD_H_LOCALS_HEADER
#define _HCD_H_LOCALS_HEADER

#include <gulliver.h>
#include <sys/slogcodes.h>
#include <sys/queue.h>
#include <pthread.h>

#define USB_HCD_VERSION     0x0101

extern  pthread_mutex_t     usb_mutex;
extern  pthread_rwlock_t    usb_rwlock;


typedef void (*VOIDFUNCPTR)();
typedef void (*FUNCPTR)();

#define SUCCESS                 0
#define ERROR                   -1

#define USB_NB_DEVICES 					20
#define	USB_NB_CONTROL_ED_PER_DEVICE	2
#define	USB_NB_BULK_ED_PER_DEVICE		4
#define	USB_NB_INT_ED_PER_DEVICE		1
#define	USB_NB_CONFIG_PER_DEVICE		2

#define	USB_NB_INTERFACE_PER_CONFIG		2
#define	USB_NB_ENDPOINT_PER_INTERFACE	4

#define	USB_NB_CONFIGS	 				USB_NB_DEVICES*USB_NB_CONFIG_PER_DEVICE
#define	USB_NB_INTERFACES				USB_NB_CONFIGS*USB_NB_INTERFACE_PER_CONFIG
#define	USB_NB_ENDPOINTS				USB_NB_INTERFACES*USB_NB_ENDPOINT_PER_INTERFACE

#define MAX_ROOT_HUB    				16
#define USB_NB_CLASS_DRIVERS            8

#define URB_DATA_PHYS           0x00020000

// Useful TAILQ manifests possibly not in sys/queue.h
#ifndef TAILQ_EMPTY
    #define TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#endif
#ifndef TAILQ_FIRST
    #define TAILQ_FIRST(head) ((head)->tqh_first)
#endif
#ifndef TAILQ_LAST
    #define TAILQ_LAST(head) ((head)->tqh_last)
#endif
#ifndef TAILQ_NEXT
    #define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#endif
#ifndef TAILQ_PREV
    #define TAILQ_PREV(elm, field) ((elm)->field.tqe_prev)
#endif

#ifndef TAILQ_HEAD
#define TAILQ_HEAD(name, type)                      \
struct name {                               \
    struct type *tqh_first; /* first element */         \
    struct type **tqh_last; /* addr of last next element */     \
}

#endif

#endif


__SRCVERSION( "$URL: http://community.qnx.com/svn/repos/internal-outsourcing/trunk/services/usb/hcd/local.h $ $Rev: 373 $" )
