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


// Portions Copyright 1999, Thierry Giron, All rights Reserved.

// Module Description:  Header for usb.c

#ifndef __USB_H_INCLUDED
#define __USB_H_INCLUDED

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/io-usb.h>

#include "local.h"
#include "error.h"
#include "ccache.h"
#include "../io-usb/data_cache.h"

//						USB SPECIFIED DEVICE DESCRIPTOR
typedef struct _st_USB_DEVICE_DESCRIPTOR {
    _uint8	bLength;
    _uint8	bDescriptorType;
    _uint16	bcdUSB;
    _uint8	bDeviceClass;
    _uint8	bDeviceSubClass;
    _uint8	bDeviceProtocol;
    _uint8	bMaxPacketSize0;
    _uint16	idVendor;
    _uint16	idProduct;
    _uint16	bcdDevice;
    _uint8	iManufacturer;
    _uint8	iProduct;
    _uint8	iSerialNumber;
    _uint8	bNumConfigurations;
} st_USB_DEVICE_DESCRIPTOR;

#define	DEV_DESC_BLENGTH 					0
#define	DEV_DESC_BDESCRIPTORTYPE 			1
#define	DEV_DESC_BCDUSB 					2
#define	DEV_DESC_BDEVICECLASS 				4
#define	DEV_DESC_BDEVICESUBCLASS 			5
#define	DEV_DESC_BDEVICEPROTOCOL 			6
#define	DEV_DESC_BMAXPACKETSIZE0 			7
#define	DEV_DESC_IDVENDOR 					8
#define	DEV_DESC_IDPRODUCT 					10
#define	DEV_DESC_BCDDEVICE 					12
#define	DEV_DESC_IMANUFACTURER 				14
#define	DEV_DESC_IPRODUCT 					15
#define	DEV_DESC_ISERIALNUMBER 				16
#define	DEV_DESC_BNUMCONFIGURATIONS 		17

#define	SIZE_DEVICE_DESCRIPTOR				18

//						USB SPECIFIED CONFIGURATION DESCRIPTOR
typedef struct _st_USB_CONFIGURATION_DESCRIPTOR {
    _uint8	bLength;
    _uint8	bDescriptorType;
	_uint16	wTotalLength;
    _uint8	bNumInterfaces;
    _uint8	bConfigurationValue;
    _uint8	iConfiguration;
    _uint8	bmAttributes;
    _uint8	MaxPower;
} st_USB_CONFIGURATION_DESCRIPTOR;

#define	CFG_DESC_BLENGTH					0
#define	CFG_DESC_BDESCRIPTORTYPE			1
#define	CFG_DESC_WTOTALLENGTH				2
#define	CFG_DESC_BNUMINTERFACES				4
#define	CFG_DESC_BCONFIGURATIONVALUE		5
#define	CFG_DESC_ICONFIGURATION				6
#define	CFG_DESC_BMATTRIBUTES				7
#define	CFG_DESC_MAXPOWER					8
#define	SIZE_CONFIGURATION_DESCRIPTOR		9

//						USB SPECIFIED INTERFACE DESCRIPTOR
typedef struct _st_USB_INTERFACE_DESCRIPTOR {
    _uint8 bLength;
    _uint8 bDescriptorType;
    _uint8 bInterfaceNumber;
    _uint8 bAlternateSetting;
    _uint8 bNumEndpoints;
    _uint8 bInterfaceClass;
    _uint8 bInterfaceSubClass;
    _uint8 bInterfaceProtocol;
    _uint8 iInterface;
} st_USB_INTERFACE_DESCRIPTOR;

#define	INT_DESC_BLENGTH					0
#define	INT_DESC_BDESCRIPTORTYPE			1
#define	INT_DESC_BINTERFACENUMBER			2
#define	INT_DESC_BALTERNATESETTING			3
#define	INT_DESC_BNUMENDPOINTS				4
#define	INT_DESC_BINTERFACECLASS			5
#define	INT_DESC_BINTERFACESUBCLASS			6
#define	INT_DESC_BINTERFACEPROTOCOL			7
#define	INT_DESC_IINTERFACE					8

#define	SIZE_INTERFACE_DESCRIPTOR			9

//						USB SPECIFIED ENDPOINT DESCRIPTOR
typedef struct _st_USB_ENDPOINT_DESCRIPTOR {
    _uint8	bLength;
    _uint8	bDescriptorType;
    _uint8	bEndpointAddress;
    _uint8	bmAttributes;
    _uint16	wMaxPacketSize;
    _uint8	bInterval;
} st_USB_ENDPOINT_DESCRIPTOR ;

#define	END_DESC_BLENGTH					0
#define	END_DESC_BDESCRIPTORTYPE			1
#define	END_DESC_BENDPOINTADDRESS			2
#define	END_DESC_BMATTRIBUTES				3
#define	END_DESC_WMAXPACKETSIZE				4
#define	END_DESC_BINTERVAL					6

#define	SIZE_ENDPOINT_DESCRIPTOR			7

//						USB SPECIFIED STRING DESCRIPTOR
typedef struct _st_USB_STRING_DESCRIPTOR {
    _uint8	bLength;
    _uint8	bDescriptorType;
    _uint16	bString[1];
} st_USB_STRING_DESCRIPTOR;

#define	STR_DESC_BLENGTH					0
#define	STR_DESC_BDESCRIPTORTYPE			1
#define	STR_DESC_BSTRING					2

#define	SIZE_STRING_DESCRIPTOR				4

//						USB SPECIFIED COMMON DESCRIPTOR
typedef struct _st_USB_COMMON_DESCRIPTOR {
    _uint8 bLength;
    _uint8 bDescriptorType;
} st_USB_COMMON_DESCRIPTOR;

#define COM_DESC_BLENGTH					0
#define COM_DESC_BDESCRIPTORTYPE			1
#define SIZE_COMMON_DESCRIPTOR				2

//						DRIVER CONFIGURATION DESCRIPTOR
typedef struct _st_CONFIGURATION_DESCRIPTOR {
	_uint32								DriverOwner;
	st_USB_CONFIGURATION_DESCRIPTOR 	ConfigurationDescriptor;
	void								*PtrNextConfig;
	void								*DevicePtrInterface;
	void								*PtrDevice;
} st_CONFIGURATION_DESCRIPTOR;

//						DRIVER ENDPOINT DESCRIPTOR
typedef struct _st_ENDPOINT_DESCRIPTOR {
	st_USB_ENDPOINT_DESCRIPTOR 	USBEd;
	void						*PtrNextEndpoint;
	void						*PtrUSBEndpoint;
	void						*PtrInterface;

	TAILQ_HEAD(,usbd_urb)		qlist;
	TAILQ_HEAD(,usbd_urb)		alist;
	_uint32						status;			// stalled, etc...
	_uint32						isoch_frame;	// next available starting frame
	ccache_buffer				*bcache;		// used to store cache of mapped client buffers
} st_ENDPOINT_DESCRIPTOR;

#define USB_DEV_PRESENT		1
#define USB_DEV_REMOVED		2

//								DRIVER DEVICE DESCRIPTOR
typedef struct _st_DEVICE_DESCRIPTOR {
	_uint32									DeviceAddress;
	_uint32									DeviceTempAddress;
	_uint32									DeviceStatus;
	_uint8									DeviceSpeed;
	_uint8									Status;
	_uint16									Generation;		// generation number
	_uint32									Openings;
	_uint32									DeviceSelectedConfiguration;
	void									*DeviceClassDriver;
	struct _st_DEVICE_DESCRIPTOR			*DeviceParent;
	_uint32									DevicePortAttachment;
	_uint32									DeviceFirstChild;
	_uint32									DeviceBrother;
	st_USB_DEVICE_DESCRIPTOR 				DeviceDescriptor;
	st_ENDPOINT_DESCRIPTOR					ControlEndpoint;
	void                 					*PtrEndpoint;
	st_CONFIGURATION_DESCRIPTOR				*PtrConfig;
	_uint32									ctrlno;
	usb_data_store_ctrl_t					*dstore;
} st_DEVICE_DESCRIPTOR;

//						DRIVER INTERFACE DESCRIPTOR
typedef struct _st_INTERFACE_DESCRIPTOR {
	_uint8							InterfaceNumber;
	st_USB_INTERFACE_DESCRIPTOR 	InterfaceDescriptor;
	_uint32							SelectedAlternateSetting;
	void							*ClassDriver;
	void							*PtrNextInterface;
	void							*PtrEndpoint;
	void							*PtrConfig;
} st_INTERFACE_DESCRIPTOR;

//                             URB HEADER
typedef struct _st_URB_HEADER {
	_uint32 		flags;
	_uint32 		Semaphore;
	_uint32			Status;
	_uint32			StructType;
	_uint32 		Function;
	_uint32 		DeviceAddress;
	_uint32			ControllerNumber;
	_uint32 		Timeout;
} st_URB_HEADER;

//                      URB DEVICE DISCOVERY REQUEST HEADER
typedef struct _st_URB_DEVICE_INFO {
	st_URB_HEADER				Header;
	st_USB_DEVICE_DESCRIPTOR	DeviceDescriptor;
} st_URB_DEVICE_INFO;

typedef struct _st_URB_SELECT_CONFIG {
	st_URB_HEADER	Header;
	_uint32			SelectedConfiguration;
} st_URB_SELECT_CONFIG;

typedef struct _st_URB_SELECT_INTERFACE {
	st_URB_HEADER	Header;
	_uint32			SelectedConfiguration;
	_uint32			SelectedInterface;
	_uint32			SelectedAlternateSetting;
} st_URB_SELECT_INTERFACE;

typedef struct _st_URB_CONFIG_INFO {
	st_URB_HEADER					Header;
	_uint32							SelectedConfiguration;
	st_USB_CONFIGURATION_DESCRIPTOR	ConfigurationDescriptor;
} st_URB_CONFIG_INFO;

typedef struct _st_URB_INTERFACE_INFO {
	st_URB_HEADER				Header;
	_uint32						SelectedConfiguration;
	_uint32						SelectedInterface;
	st_USB_INTERFACE_DESCRIPTOR	InterfaceDescriptor;
} st_URB_INTERFACE_INFO;

typedef struct _st_URB_ENDPOINT_INFO {
	st_URB_HEADER				Header;
	_uint32						SelectedConfiguration;
	_uint32				 		SelectedInterface;
	_uint32 					SelectedEndpoint;
	_uint8			 			EndpointSelectionMethod;
	_uint8		 				EndpointNumber;
	_uint8			 			EndpointType;
	_uint8 						EndpointDirection;
	_uint16				 		EndpointMaxPacketSize;
	st_USB_ENDPOINT_DESCRIPTOR	EndpointDescriptor;
} st_URB_ENDPOINT_INFO;

typedef struct _st_URB_CLEAR_STALLED_ENDPOINT {
	st_URB_HEADER	Header;
	_uint32 		SelectedConfiguration;
	_uint32 		SelectedInterface;
	_uint32 		SelectedEndpoint;
	_uint32 		Direction;
} st_URB_CLEAR_STALLED_ENDPOINT;

//                      URB TRANSFER REQUEST HEADER
typedef struct _st_URB_TRANSFER {
	st_URB_HEADER	Header;
	_uint32 		SelectedConfiguration;
	_uint32 		SelectedInterface;
	_uint32 		SelectedEndpoint;
	void			*BufferAddress;
	_uint32			BufferLength;
	_uint32			ActualLength;
	_uint32			Direction;
	_uint32			Request;
	_uint32			Type;
	_uint32			Recipient;
	_uint32			ValueDesc;
	_uint32			ValueIndex;
	union {
		_uint32						Index;
		struct _st_URB_TRANSFER 	*urb_nxt_cmplte_process;
	};
	_uint32			IsochFrame;
	void			(*CallBackFunction) (struct _st_URB_TRANSFER *urb);
	void			*User1;				// Ptr used by the class driver
	void			*User2;				// Ptr used by the class driver
} st_URB_TRANSFER;

//                      URB SETUP REQUEST TYPE
typedef struct _st_USB_SETUP_REQUEST {
	_uint8	RequestType;
	_uint8	Request;
	_uint8	DescriptorIndex;
	_uint8	DescriptorType;
	_uint16	Index;
	_uint16	Length;
} st_USB_SETUP_REQUEST;

#define	SETUP_DESC_REQUESTTYPE			0
#define	SETUP_DESC_REQUEST				1
#define	SETUP_DESC_DESCRIPTORINDEX		2
#define	SETUP_DESC_DESCRIPTORTYPE		3
#define	SETUP_DESC_INDEX				4
#define	SETUP_DESC_LENGTH				6

#define	SIZE_SETUP_REQUEST				8

//                      URB ABORT REQUEST TYPE
typedef struct _st_URB_ABORT_REQUEST {
	st_URB_HEADER 	Header;
	_uint32 		SelectedConfiguration;
	_uint32 		SelectedInterface;
	_uint32 		SelectedEndpoint;
	_uint32			Direction;
	void            *urb_hcd;             // Ptr used by the class driver
}st_URB_ABORT_REQUEST;

//						CONTROL ENDPOINT REQUEST
typedef struct _st_CONTROL_TRANSFER {
	_uint32 	flags;
	_uint32 	Semaphore;
	_uint32		Status;
	_uint32		StructType;
	_uint32 	EndpointNumber;
	_uint32		DeviceAddress;
	_uint32		Timeout;
	void		*BufferAddress;
	void		*CurrentBufferAddress;
	_uint32		BufferLength;
	_uint32		Direction;
	_uint32		Request;
	_uint32		Type;
	_uint32		Recipient;
	_uint32		ValueDesc;
	_uint32		ValueIndex;
	_uint32		Index;
	_uint32		PacketPayloadLength;
	_uint32		ActualLength;
	st_ENDPOINT_DESCRIPTOR *PtrEndpoint;
} st_CONTROL_TRANSFER;

//                      URB GET STRING REQUEST
typedef struct _st_URB_GET_STRING {
	st_URB_HEADER 	Header;
	_uint16		  	StringIndex;
	_uint32			MaxLength;
	_uint8			*StringBuffer;
	_uint32			StringDecodingFlag;
} st_URB_GET_STRING;

//                             USB HANDLES STRUCTURE
typedef struct _st_USB_HANDLE {
	_uint32  					DriverOwner;
	_uint32						HandleStatus;
	st_USB_ENDPOINT_DESCRIPTOR	*EndpointPtr;
	void						**NextHandlePtr;
} st_USB_HANDLE;

//                        USB IOCTL FUNCTIONS
#define	USB_GET_DEVICE_DESCRIPTOR				1
#define	USB_GET_CONFIGURATION_DESCRIPTOR		2
#define	USB_GET_INTERFACE_DESCRIPTOR			3
#define	USB_GET_ENDPOINT_DESCRIPTOR				4
#define	USB_SELECT_CONFIGURATION				5
#define	USB_SELECT_INTERFACE					6
#define	USB_SEND_COMMAND						7
#define	USB_TRANSFER							8
#define	USB_ABORT_TRANSFER						9
#define	USB_CLEAR_STALLED_ENDPOINT				10
#define	USB_GET_STRING							11

//                        USB COMMANDS FUNCTIONS
#define	GET_STATUS					0
#define	CLEAR_FEATURE				1
#define	SET_FEATURE					3
#define	SET_ADDRESS					5
#define	GET_DESCRIPTOR				6
#define	SET_DESCRIPTOR				7
#define	GET_CONFIGURATION			8
#define	SET_CONFIGURATION			9
#define	GET_INTERFACE				10
#define	SET_INTERFACE				11
#define	SYNCH_FRAME					12

#define	GET_CLASS_DESCRIPTOR		0
#define	GET_REPORT					1
#define	GET_IDLE					2
#define	GET_PROTOCOL				3
#define	SET_CLASS_DESCRIPTOR		8
#define	SET_REPORT					9
#define	SET_IDLE					10
#define	SET_PROTOCOL				11

#define	TYPE_INPUT					1
#define	TYPE_OUTPUT					2
#define	TYPE_FEATURE				3

#define	DEVICE_DESCRIPTOR			1
#define	CONFIG_DESCRIPTOR			2
#define	STRING_DESCRIPTOR			3
#define	INTERFACE_DESCRIPTOR		4
#define	ENDPOINT_DESCRIPTOR			5
#define	DEVICE_QUALIFIER			6
#define	OTHER_SPEED_CONFIGURATION	7
#define	INTERFACE_POWER				8

#define	REQ_TYPE				0
#define	REST 					1
#define	REQ_VALUE				2
#define	REQ_INDEX				4
#define	REQ_LEN 				6

#define	DEV_LEN 				0
#define	DEV_TYPE				1
#define	DEV_VER 				2
#define	DEV_CLASS				4
#define	DEV_SUB 				5
#define	DEV_PROTOCOL			6
#define	DEV_MAX0				7
#define	DEV_NUM 				17

#define	CONFIG_LEN				0
#define	CONFIG_TYPE				1
#define	CONFIG_TOTAL			2
#define	CONFIG_NUM				4
#define	CONFIG_VALUE			5
#define	CONFIG_INDEX			6
#define	CONFIG_ATTR				7
#define	CONFIG_MAXP				8

#define	INTER_LEN				0
#define	INTER_TYPE				1
#define	INTER_ID				2
#define	INTER_ALT				3
#define	INTER_ENDN				4
#define	INTER_CLASS				5
#define	INTER_SUBCLASS			6
#define	INTER_PROTO				7
#define	INTER_INDEX				8

//                  GENERIC DEFAULT PIPE DESTINATION EQUIVALENCES
#define	SIZE_CONTROL_ENDPOINT 8L
#define	CLASS_DEVICE			0
#define	CLASS_INTERFACE			1
#define	CLASS_ENDPOINT			2
#define	CLASS_OTHER				3

#define	RECIPIENT_DEVICE		0
#define	RECIPIENT_INTERFACE		1
#define	RECIPIENT_ENDPOINT		2
#define	RECIPIENT_OTHER			3

#define	TYPE_STANDARD			0
#define	TYPE_CLASS   			1
#define	TYPE_VENDOR				2
#define	TYPE_RESERVED			3

#define	CLASS					0x20
#define	VENDOR					0x40

#define	OUT_ENDPOINT			0
#define	IN_ENDPOINT				0x80

#define	CONTROL_ENDPOINT		0x00
#define	ISOCH_ENDPOINT			0x01
#define	BULK_ENDPOINT			0x02
#define	INT_ENDPOINT			0x03

//                          USB TRANSFER TYPES
#define	TRANSFER_CONTROL		0
#define	TRANSFER_ISOCHRONOUS	1
#define	TRANSFER_BULK			2
#define	TRANSFER_INTERRUPT		3

#define	TRANSFER_BULKIN			10
#define	TRANSFER_BULKOUT		11
#define	TRANSFER_ISOCHIN		12
#define	TRANSFER_ISOCHOUT		13

//                        USB COMMAND BUFFER PARAMETERS
#define HOST_TO_DEVICE			0
#define DEVICE_TO_HOST			0x80

//                        USB ENDPOINTS TYPE
#define ED_STATIC				0x00000001
#define	ED_CONTROL				0x00000002
#define ED_INTERRUPT			0x00000004	
#define	ED_BULK					0x00000008
#define	ED_ISOCH				0x00000010
#define	ED_FIRST_ISOCH_T		0x00000020

//                 ENDPOINT TRANSFER DESCRIPTOR STATUS
#define FREE_ED					0x00000000
#define USED_ED					0x80000000
#define ALLOCATED_ED	 		0x40000000
#define LAST_ED					0x20000000

#define FREE_TD					0x00000000
#define USED_TD					0x80000000

#define ISOCH_NB_BP				8

//         			DEVICE SPEED TYPE
#define	DEVICE_FULL_SPEED		0
#define	DEVICE_LOW_SPEED		1
#define	DEVICE_HIGH_SPEED		2
#define	DEVICE_MASK_SPEED		3

//           		USB SPECIFIC TIMINGS
#define WAIT_ROOT_HUB_IN_RESET 		50
#define WAIT_ROOT_HUB_RESET			150
#define WAIT_ROOT_DEVICE_INSERT		50
#define	WAIT_DEVICE_SETTLE_ADDRESS	20

//					ENDPOINT SELECTION VALUES
#define	ENDPOINT_SELECTION_VALUE	0
#define	ENDPOINT_SELECTION_INDEX	1

//					STRING RELATED EQUIVALENCES
#define	USB_STRING_NO_DECODE		0
#define	USB_STRING_DECODE_ASCII		1

//					GUARANTEED BANDWIDTH VALUE
#define	USB_MAX_GUARANTEED_BANDWIDTH	1000

#define USB_DFLT_TIMEOUT				2000
#define USB_DFLT_CNTL_TIMEOUT			2000

#ifdef __QNXNTO__
#define USB_DELAY_MULT_MS				1000000LL
#define USB_DELAY_MULT_S				1000000000LL
#else
#define USB_DELAY_MULT_MS				1
#define USB_DELAY_MULT_S				1000
#endif

extern _uint32 USB_SendCommand( st_URB_TRANSFER *Urb );
extern _uint32 USB_GetEndpointDescriptor( st_URB_ENDPOINT_INFO *Urb );
extern _uint32 USB_Transfer( st_URB_TRANSFER *Urb );
extern _uint32 USB_AbortTransfer( st_URB_ABORT_REQUEST *Urb );
extern st_DEVICE_DESCRIPTOR *USB_CheckDevice( _uint32 hc_num, _uint32 DeviceNb );
extern st_INTERFACE_DESCRIPTOR *USB_GetInterface( _uint32 ctrlno, _uint32 addr, _uint32 cfg, _uint32 iface );
extern st_ENDPOINT_DESCRIPTOR *USB_GetEndpoint( st_INTERFACE_DESCRIPTOR *idesc, _uint32 eno );
extern _uint32 USB_SelectConfiguration( st_URB_TRANSFER *Urb );
extern _uint32 USB_SelectInterface( st_URB_SELECT_INTERFACE *Urb );
extern _uint32 USB_ClearStalledEndpoint( st_URB_CLEAR_STALLED_ENDPOINT *SUrb );
extern int USB_Frame( _uint32 ctrlno );
extern int USB_ControllerType( );
extern _uint32 USB_ResetPort( st_USB_Hc *Hc, st_DEVICE_DESCRIPTOR *ddesc );
extern _uint32 USB_SuspendPort( st_USB_Hc *Hc, st_DEVICE_DESCRIPTOR *ddesc );
extern _uint32 USB_ResumePort( st_USB_Hc *Hc, st_DEVICE_DESCRIPTOR *ddesc );
extern _uint32 USB_SuspendBus( st_USB_Hc *Hc );
extern _uint32 USB_ResumeBus( st_USB_Hc *Hc );

#endif

__SRCVERSION( "$URL: http://community.qnx.com/svn/repos/internal-outsourcing/trunk/services/usb/hcd/usb.h $ $Rev: 843 $" )
