/*
 * $QNXtpLicenseC:  
 * Copyright 2009, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/


#include "dwcotg.h"

/*
 * TODO
 * 1.   Add Isoch support.
 * 2.   Add Scatter-Gather support
 * 3.   This code has partial hub supported, but it hasn't been tested. More
 *      work needs to be done:
 * 3a.  Add a 'NO-HUB Supported' flag when the controller only has a small
        number of channels e.g. 4
 * 3b.  Add/Test hub support on controller that has adqequate number of channels
        e.g. 16 
 * 4. The Driver doesn't do any channel sharing...Each enabled endpoint is 
 *    allocated a channel, so this driver only supported 4 enabled endpoints
 *    at any given time.  Channel sharing needs to be investigated.
 */

// ioport,irq,priority,pindex handled by io-usb
char *cmdline_opts[] = {
    #define DWCOTG_OPT_VERBOSITY        0
               "verbose",
    #define DWCOTG_OPT_SCHEDULER_PRIO   1
               "sched_prio",
    #define DWCOTG_OPT_NUMBER_CHANNELS  2
               "nchan",
               NULL};


static hc_methods_t dwcotg_controller_methods = {
                    9,
                    dwcotg_controller_init,
                    dwcotg_controller_shutdown,
                    dwcotg_set_bus_state,
                    NULL,
                    NULL,
                    dwcotg_controller_interrupt,
                    dwcotg_set_port_feature,
                    dwcotg_clear_port_feature,
                    dwcotg_check_port_status,
                    dwcotg_check_device_connected,
                    dwcotg_get_root_device_speed,
                    dwcotg_get_timer_from_controller
};

static hc_pipe_methods_t dwcotg_ctrl_pipe_methods = {
    dwcotg_ctrl_endpoint_enable,
    dwcotg_ctrl_endpoint_disable,
    dwcotg_ctrl_transfer,
    dwcotg_ctrl_transfer_abort,
    NULL
};

static hc_pipe_methods_t dwcotg_bulk_pipe_methods = {
    dwcotg_bulk_endpoint_enable,
    dwcotg_bulk_endpoint_disable,
    dwcotg_bulk_transfer,
    dwcotg_bulk_transfer_abort,
    NULL
};

static hc_pipe_methods_t dwcotg_int_pipe_methods = {
    dwcotg_int_endpoint_enable,
    dwcotg_int_endpoint_disable,
    dwcotg_int_transfer,
    dwcotg_int_transfer_abort,
    NULL
};

static hc_pipe_methods_t dwcotg_isoch_pipe_methods = {
    dwcotg_isoch_endpoint_enable,
    dwcotg_isoch_endpoint_disable,
    dwcotg_isoch_transfer,
    dwcotg_isoch_transfer_abort,
    NULL
};

io_usb_dll_entry_t io_usb_dll_entry = {
    DWCOTG_DLL_NAME,
    0xFFFFFFFF,  
    &dwcotg_init,
    &dwcotg_shutdown,
    &dwcotg_controller_methods,
    &dwcotg_ctrl_pipe_methods,
    &dwcotg_int_pipe_methods,
    &dwcotg_bulk_pipe_methods,
    &dwcotg_isoch_pipe_methods 
};

// local prototypes
static void start_data_transfer( hctrl_t *xhc, xelem_t * xelem );




void hc_slogf( hctrl_t *xhc, int level, const char *fmt, ... )
{
    va_list        arglist;
#if 1
    if( !xhc || ( xhc->verbosity >= level ) ) {
        va_start( arglist, fmt );
        vslogf( 12, level, fmt, arglist );
        va_end( arglist );
    }
#endif
}


// software scheduler thread...used to schedule interrupt/isoch transfers
void * scheduler( void * hdl ) {
	hctrl_t         *xhc = hdl;
	int             rcvid;
	struct _pulse   pulse;

	
    while ( 1 ) {
		if( ( rcvid = MsgReceivePulse( xhc->chid, &pulse, sizeof( pulse ), NULL ) ) == -1 ) {
			hc_slogf( xhc, _SLOG_ERROR, "%s:  bad pulse", __func__);
			continue;
		}
		switch (pulse.code) {
			case SCHED_DOSTART:
				pthread_mutex_lock( &xhc->mutex );
				while (xhc->ep_arr[pulse.value.sival_int].active_xfer)
					delay(10);
				start_data_transfer( xhc, xhc->ep_arr[pulse.value.sival_int].xelem );
				pthread_mutex_unlock( &xhc->mutex );
				break;				
			default:
				 hc_slogf( xhc, _SLOG_ERROR, "%s: unknown pulse type %d code %d", __func__, pulse.type, pulse.code);  
		}
		
    }
}

static int scheduler_create( hctrl_t * xhc ) {
    psched_attr_t       *sattr;
    pthread_attr_t      tattr;
    struct sched_param  param;
//	struct sigevent     event;

    sattr = psched_attr_create();
    if ( sattr == NULL ) {
        goto fail;
    }

    /* Configure the scheduler attributes... Attributes that remain unset
     * will use defaults
     */
    psched_attr_flags_set( sattr, PSCHED_FLAGS_THREAD_SAFE) ;

    xhc->scheduler = psched_create( sattr );
    if ( xhc->scheduler == NULL ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: could not create scheduler",__func__);
        goto fail2;
    }
   
	// create channel
	if( ( xhc->chid = ChannelCreate( _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK ) ) == -1 ||
		( xhc->coid = ConnectAttach( 0, 0, xhc->chid, _NTO_SIDE_CHANNEL, 0 ) ) == -1 ) {
		hc_slogf( xhc, _SLOG_ERROR,  "%s:  ChannelCreate/ConnectAttach - %s", __func__, strerror( errno ) );
		goto fail2;;
	}	
	
#if 0
	// create timer
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_code   = SCHED_TIMER;
	event.sigev_coid   = xhc->coid;
	event.sigev_priority = xhc->scheduler_priority;

	if( timer_create( CLOCK_REALTIME, &event, &xhc->timerid ) == -1 ) {
		hc_slogf( xhc, _SLOG_ERROR,  "%s:  timer_create failed - %s", __func__, strerror( errno ) );
		goto fail2;
	}
#endif

	psched_attr_destroy( sattr );
	
    // create the scheduler thread
    pthread_attr_init( &tattr );
    pthread_attr_setschedpolicy( &tattr, SCHED_RR );
    param.sched_priority = xhc->scheduler_priority;
    pthread_attr_setschedparam( &tattr, &param );
    pthread_attr_setinheritsched( &tattr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setdetachstate( &tattr, PTHREAD_CREATE_DETACHED );
    if ( pthread_create( &xhc->scheduler_tid, &tattr, scheduler, xhc ) ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: could not create scheduler thread",__func__);
        goto fail3;
    }
    
    return EOK;
    
fail3:
    psched_destroy( xhc->scheduler );
fail2:    
    psched_attr_destroy( sattr );
fail:
    return -1;
}

static void scheduler_destroy( hctrl_t * xhc ) {
    pthread_detach( xhc->scheduler_tid );
    psched_destroy( xhc->scheduler );
}


static int xelem_pool_create( hctrl_t *xhc ) {
    fbma_attr_t     *attr;
    
    // create attribute struct
    attr = fbma_attr_create();
    if ( attr == NULL ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: could not create pool attributes",__func__);
        goto fail;
    }
    
    // set desired attributes for the pool
    fbma_attr_blksz_set( attr, sizeof( xelem_t ) );
    fbma_attr_nblk_set( attr, MAX_XFER_ELEM );
    fbma_attr_maxpool_set( attr, 1 );

    // create the allocator
    xhc->xelem_pool = fbma_create( attr );
    if ( xhc->xelem_pool == NULL ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: could not create pool ",__func__);
        goto fail2;
    }

    // we don't need attr struct once the allocator is created
    fbma_attr_destroy( attr );

    return EOK;
    
fail2:
    fbma_attr_destroy( attr );
fail:
    return ENOMEM;
}

static void xelem_pool_destroy( hctrl_t *xhc ) {
    fbma_destroy( xhc->xelem_pool );
}

static inline xelem_t * xelem_create( hctrl_t *xhc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,
            _uint8 *buffer, _uint32 length, _uint32 flags )
{
	xelem_t         *xelem;

	xelem = fbma_alloc( xhc->xelem_pool, NULL );

	if ( xelem == NULL ) {
		hc_slogf( xhc, _SLOG_INFO, "%s: Couldn't allocate XFER Element",__func__);
		return NULL;
	}
	
	xelem->urb =     urb;
	xelem->edesc =   edesc;
	xelem->buffer =  buffer;    
	xelem->length =  length;
	xelem->flags =   flags;
	
	return xelem;
}


static inline void xelem_destroy( hctrl_t *xhc, xelem_t * xelem ) {
    fbma_free( xhc->xelem_pool, xelem );
}


_uint32 dwcotg_get_timer_from_controller( st_USB_Hc *hc )
{
    hctrl_t   *xhc = hc->hc_data;
    uint32_t uframe;

    // HFNUM is specified in FRAMES, 
    // so shift by 3 for microframes (i.e. 8 microframes/frame)
    uframe = ( HW_Read32( xhc->IoBase, DWCOTG_HFNUM ) & HFNUM_FRAME_NUM_MSK) << 3;     
    hc_slogf( xhc, _SLOG_INFO, "%s: uframe = 0x%x",__func__,uframe);

    return uframe;
}


_uint32 dwcotg_check_device_connected( st_USB_Hc *hc, _uint32 port )
{
    hctrl_t   *xhc = hc->hc_data;

    if ( HW_Read32( xhc->IoBase, DWCOTG_HPRT ) & HPRT_CONNECT_STS ) {
        return EOK;
    } else {
        return -1;
    }
}

_uint32 dwcotg_get_root_device_speed( st_USB_Hc *hc, _uint32 port )
{
    hctrl_t   *xhc = hc->hc_data;
    uint32_t speed;

    switch ( HW_Read32( xhc->IoBase, DWCOTG_HPRT ) & HPRT_SPEED_MSK ) 
    {
        case HPRT_SPEED_HIGH:
             hc_slogf( xhc, _SLOG_INFO, "%s: DEVICE_HIGH_SPEED",__func__);
            speed = DEVICE_HIGH_SPEED;
            break;
            
        case HPRT_SPEED_FULL:
            hc_slogf( xhc, _SLOG_INFO, "%s: DEVICE_FULL_SPEED",__func__);
            speed = DEVICE_FULL_SPEED;
            break;
        
        case HPRT_SPEED_LOW:
            hc_slogf( xhc, _SLOG_INFO, "%s: DEVICE_LOW_SPEED",__func__);
            speed = DEVICE_LOW_SPEED;
            break;
        
        default:
            hc_slogf( xhc, _SLOG_ERROR, "%s: SPEED = RESERVED... returning DEVICE_FULL_SPEED ",__func__);
            speed = DEVICE_FULL_SPEED;  
            break;
    }

    return speed;
}


static void port_power_enable( st_USB_Hc *hc )
{
    hctrl_t   *xhc = hc->hc_data;
    
    HW_Write32Or( xhc->IoBase, DWCOTG_HPRT, HPRT_POWER );
}

static void port_power_disable( st_USB_Hc *hc )
{
    hctrl_t   *xhc = hc->hc_data;
    
    HW_Write32And( xhc->IoBase, DWCOTG_HPRT, ~HPRT_POWER );
}

#define PORT_RESET_TIMEOUT  1000
static _uint32 root_hub_reset( st_USB_Hc *hc, _uint32 port )
{
    hctrl_t     *xhc = hc->hc_data;
    uint32_t    retval = EOK;
    int         i = 0;
    
    // reset the 'PrtEnChng' ... 
    HW_Write32Or( xhc->IoBase, DWCOTG_HPRT, HPRT_PORT_ENABLE_CHANGE );
    
    // toggle the reset bit
    HW_Write32Or( xhc->IoBase, DWCOTG_HPRT, HPRT_RESET );
    delay( 10 );    // wait 10ms as per reference manual
    HW_Write32And( xhc->IoBase, DWCOTG_HPRT, ~HPRT_RESET );
    
    // wait for rtEnChng to go high
    while ( ( i++ < PORT_RESET_TIMEOUT ) && 
            ( !( HW_Read32( xhc->IoBase, DWCOTG_HPRT ) & HPRT_PORT_ENABLE_CHANGE ) ) ) {       
        delay( 1 );
    }
    
    if ( i >= PORT_RESET_TIMEOUT ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: USB BUS Reset Failed  DWCOTG_HPRT = 0x%x"
            ,__func__, HW_Read32( xhc->IoBase, DWCOTG_HPRT ) );
        retval = -1;
    }
    
     hc_slogf( xhc, _SLOG_ERROR, "%s: USB BUS DWCOTG_HPRT = 0x%x"
            ,__func__, HW_Read32( xhc->IoBase, DWCOTG_HPRT ) );
    return retval;
}

_uint32 dwcotg_set_port_feature( st_USB_Hc *hc, _uint32 port, _uint32 feature )
{
    hctrl_t     *xhc = hc->hc_data;    
    uint32_t status = EOK;
    
    if ( port >= DWCOTG_N_ROOT_PORTS ) {
        return ENODEV;
    }
    
    hc_slogf( xhc, _SLOG_DEBUG1, "%s: Entry port=%d feature=%d",__func__,port,feature);
    
    switch ( feature ) {
        case USB_PORT_RESET :
            status = root_hub_reset( hc, port );
            break;
        case USB_PORT_POWER :
            port_power_enable( hc );
            break;
        case USB_PORT_SUSPEND :
            break;
            
        default:
            break;

    }
    return EOK;
}

_uint32 dwcotg_clear_port_feature( st_USB_Hc *hc, _uint32 port, _uint32 state )
{
    hctrl_t     *xhc = hc->hc_data;    

    if ( port >= DWCOTG_N_ROOT_PORTS ) {
        return ENODEV;
    }
    
    hc_slogf( xhc, _SLOG_DEBUG1, "%s: Entry port=%d feature=%d",__func__,port,state);
    
    switch ( state ) {
        case USB_PORT_ENABLE :
            break;
        case USB_PORT_POWER :
            port_power_disable( hc );
            break;
        case USB_PORT_SUSPEND :
            break;
        default:
            break;
    }
    return EOK;
}


static _uint32 endpoint_enable( st_USB_Hc *hc, st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t                 *xhc = hc->hc_data;
    uint32_t                 i;
    epctx_t                 *ep = NULL;
    uint32_t                v;
    st_DEVICE_DESCRIPTOR   	*dev;

	pthread_mutex_lock( &xhc->mutex );
    /* Allocate a channel for this endpoint if we don't already have one */         
    if ( edesc->PtrUSBEndpoint == NULL ) {
		hc_slogf( xhc, _SLOG_INFO, "%s: Endpoint type %d nchar %d",__func__ , edesc->USBEd.bmAttributes & 3, xhc->nchan);
		hc_slogf( xhc, _SLOG_INFO, "%s: HFIR %x HFNUM %x",__func__ , 
											HW_Read32( xhc->IoBase, DWCOTG_HFIR ), 
											HW_Read32( xhc->IoBase, DWCOTG_HFNUM ));
        for(i=0; i < xhc->nchan; i++ ) {
            if ( !( xhc->ep_arr[i].flags & EPFLAG_USED ) ) {
                ep = edesc->PtrUSBEndpoint = &xhc->ep_arr[i];
                memset( ep, 0, sizeof ( epctx_t ) );
                ep->chnum = i;
                ep->chmask = (1 << i);
                
                switch ( edesc->USBEd.bmAttributes & 3 ) {
                    
                    case INT_ENDPOINT:
                        break;  

                    case BULK_ENDPOINT:
                        break;

                    case ISOCH_ENDPOINT:
                        break;
                        
                    case CONTROL_ENDPOINT:
                        break;
                    
                }
                
                break;
            }
        }
        
        if ( ep == NULL ) {
            hc_slogf( xhc, _SLOG_ERROR, "%s: ***NO MORE ENDPOINTS***",__func__);
            pthread_mutex_unlock( &xhc->mutex );
            return ENODEV;
        }
    } else {
        // endpoint is already allocated, so bind to it
        ep = edesc->PtrUSBEndpoint;
    }
   
    
    /* Initialize Endpoint members */
    ep->ping_needed     = 0;
    ep->sendzlp         = 0;
	ep->do_split        = 0;
    ep->flags           = EPFLAG_USED;
	ep->pid             = HCTSIZ_PID_DATA1;
    /* 
     * Configure Channel (i.e. Endpoint) registers 
     */ 
    v =  ( edesc->USBEd.wMaxPacketSize & HCCHAR_MPS_MSK )                      |
         ( ( ( edesc->USBEd.bEndpointAddress & 0x7f ) << HCCHAR_EPNUM_POS ) )  |
         ( ( edesc->USBEd.bEndpointAddress & 0x80 ) ? HCCHAR_EPDIR_IN : 0 )    |
         ( ( ddesc->DeviceSpeed == DEVICE_LOW_SPEED ) ? HCCHAR_LOWSPEED : 0 )  |
         ( ( edesc->USBEd.bmAttributes & 3 ) << HCCHAR_EPTYPE_POS )            |
         ( HCCHAR_MCEC_3XFER )                                                 |
         ( ( ddesc->DeviceAddress << HCCHAR_DADDR_POS ) & HCCHAR_DADDR_MSK )   ;
	hc_slogf( xhc, _SLOG_INFO, "%s: Configure channel chnum %d val %x HPRT %x",__func__ , 
											ep->chnum, v, HW_Read32( xhc->IoBase, DWCOTG_HPRT ));
    HW_Write32( xhc->IoBase, DWCOTG_HCCHAR( ep->chnum ), v );
    HW_Write32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ), 0 );

    // Full-low speed device support
    if ( ddesc->DeviceSpeed != DEVICE_HIGH_SPEED  ) {

        // look for high speed parent in order to configure splitinfo
        for ( dev = ddesc; dev->DeviceParent != NULL; dev = dev->DeviceParent ) {
            
            if ( ( dev->DeviceParent->DeviceSpeed & 0x3) == DEVICE_HIGH_SPEED ) {
                v = ( (dev->DevicePortAttachment ) & HCSPLT_PORT_ADDR_MSK )        |
					HCSPLT_XACTPOSITION_ALL | 
//					(1 << 31) | 
                    ( ( (dev->DeviceParent->DeviceAddress) <<  HCSPLT_HUB_ADDR_POS ) & HCSPLT_HUB_ADDR_MSK );

				hc_slogf( xhc, _SLOG_INFO, "%s: Set splitinfo chnum %d val %x Reg val %x",__func__ ,
						  ep->chnum, v, HW_Read32( xhc->IoBase, DWCOTG_HCSPLT( ep->chnum ) ));
                HW_Write32( xhc->IoBase, DWCOTG_HCSPLT( ep->chnum ), v );
				ep->do_split        = 1;
            }
        }
    }

    /* Enable Channel Interrupts */
    
    HW_Write32( xhc->IoBase, DWCOTG_HCINT(ep->chnum) , 0xffffffff );    //clr all
    HW_Write32( xhc->IoBase, DWCOTG_HCINTMSK(ep->chnum) , HCINT_INTEREST );    
    HW_Write32Or( xhc->IoBase, DWCOTG_HAINTMSK , 1 << ep->chnum );    
    
   
    pthread_mutex_unlock( &xhc->mutex );

    return EOK;
    
}

_uint32 dwcotg_ctrl_endpoint_enable( st_USB_Hc *hc, st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    return endpoint_enable(hc, ddesc, edesc );
}

_uint32 dwcotg_bulk_endpoint_enable( st_USB_Hc *hc,st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    return endpoint_enable(hc, ddesc, edesc );
}

_uint32 dwcotg_int_endpoint_enable( st_USB_Hc *hc,st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    return endpoint_enable(hc, ddesc, edesc );
}

_uint32 dwcotg_isoch_endpoint_enable( st_USB_Hc *hc, st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    // NO SUPPORT YET
    hctrl_t   *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_ERROR, "%s: Not Supported",__func__);
    return ENOTSUP;
}


static _uint32 endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    epctx_t     *ep = edesc->PtrUSBEndpoint;
    hctrl_t     *xhc = hc->hc_data;
 
    pthread_mutex_lock( &xhc->mutex );

    /* disable interrupts, */
    HW_Write32And( xhc->IoBase, DWCOTG_HAINTMSK , ~(1 << ep->chnum) );    

    /* disable the endpoint.. need to hit the enable and disable bit at the same time
     * as described in the reference manual
     */
    HW_Write32Or( xhc->IoBase, DWCOTG_HCCHAR(ep->chnum) , HCCHAR_CHAN_ENABLE | HCCHAR_CHAN_DISABLE );    

    /* clear channel interrupts */
    HW_Write32( xhc->IoBase, DWCOTG_HCINTMSK(ep->chnum) , 0 );    
    HW_Write32( xhc->IoBase, DWCOTG_HCINT(ep->chnum) , 0xffffffff );    //clr all
    
    
    switch ( edesc->USBEd.bmAttributes & 3 ) {
        case INT_ENDPOINT:
            break;
            
        case BULK_ENDPOINT:
        case CONTROL_ENDPOINT:
        case ISOCH_ENDPOINT:
            break;
    }
    
    /* free endpoint */
    ep->flags &= ~EPFLAG_USED;  
    edesc->PtrUSBEndpoint = NULL;

    pthread_mutex_unlock( &xhc->mutex );
    
    return EOK;
}


_uint32 dwcotg_ctrl_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc )
{   
    hctrl_t     *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_INFO, "%s: Entry",__func__);
    return endpoint_disable( hc, edesc );
}

_uint32 dwcotg_bulk_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t     *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_INFO, "%s: Entry",__func__);
    return endpoint_disable( hc, edesc );
}

_uint32 dwcotg_int_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t     *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_INFO, "%s: Entry",__func__);    
    return endpoint_disable( hc, edesc );
}

_uint32 dwcotg_isoch_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t   *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_ERROR, "%s: Not Supported",__func__);
    return ENOTSUP;
}


// current abort code assumes 1 active xfer per endpoint... 
static _uint32 transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t     *xhc = hc->hc_data;
    epctx_t     *ep = edesc->PtrUSBEndpoint;

    pthread_mutex_lock( &xhc->mutex );

    if ( ep->flags & EPFLAG_ACTIVE_XFER ) {
        // endpoint is busy processing  the urb... abort the transfer

        // remove urb from periodic schedule ( urb is wrapped in xelem )
        if ( ( edesc->USBEd.bmAttributes & 3 ) == INT_ENDPOINT ) {
            // remove transfer element from schedule 
            ep->interval = 0;
        }
        
        // free  transfer element that was allocated in xfer function
        xelem_destroy(xhc, ep->xelem);
        
        // mark this endpoint as aborted so the interrupt handler can take
        // appropriate action
        ep->flags &= ~EPFLAG_ACTIVE_XFER;
        ep->flags |= EPFLAG_XFER_ABORT;    

         //disable the endpoint.. need to hit the enable and disable bit at the same time
         //as described in the reference manual
        HW_Write32Or( xhc->IoBase, DWCOTG_HCCHAR(ep->chnum) , HCCHAR_CHAN_ENABLE | HCCHAR_CHAN_DISABLE );    
    }

    pthread_mutex_unlock( &xhc->mutex );

    return EOK;
}


_uint32 dwcotg_ctrl_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t     *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_INFO, "%s: Entry",__func__);
    
    return transfer_abort( hc, urb, edesc);
}


_uint32 dwcotg_bulk_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc )
{
    return transfer_abort( hc, urb, edesc);
}

_uint32 dwcotg_int_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t     *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_INFO, "%s: Entry",__func__);    
    return transfer_abort( hc, urb, edesc);
}

_uint32 dwcotg_isoch_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t   *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_ERROR, "%s: Not Supported",__func__);    
    return ENOTSUP;
}

int dwcotg_get_frame_number(hctrl_t *xhc)
{
	uint32_t	 hfnum = HW_Read32(xhc->IoBase, DWCOTG_HFNUM);


	return (hfnum & HFNUM_FRAME_NUM_MSK);
}

static void start_data_transfer( hctrl_t *xhc, xelem_t * xelem  )
{
	epctx_t			*ep = xelem->edesc->PtrUSBEndpoint;
	uint32_t		pid;
	uint32_t		dir;
	uint32_t		v;
	uint32_t		do_ping;
	uint32_t		length;
	
	pthread_mutex_lock( &xhc->mutex );
	
	length = xelem->length;
	ep->xelem = xelem;
	
	ep->sendzlp = ( !xelem->length && ( xelem->flags & PIPE_FLAGS_TOKEN_OUT ) ) ? 1 : 0;

	/*
	 * Setup the transfer particulars.... re-use last pid ( toggle ) by default
	 */
	dir = 0;
	do_ping = 0;
	pid = HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ) ) & HCTSIZ_PID_MSK;

	switch ( xelem->edesc->USBEd.bmAttributes & 3 ) {
		case CONTROL_ENDPOINT:
			if ( xelem->flags & PIPE_FLAGS_TOKEN_SETUP ) {
				dir     = HCCHAR_EPDIR_OUT;
				pid     = HCTSIZ_PID_SETUP;
			}
			else if ( xelem->flags & PIPE_FLAGS_TOKEN_IN ) {
				dir     = HCCHAR_EPDIR_IN;

				if (ep->pid == HCTSIZ_PID_DATA1)
					pid     = HCTSIZ_PID_DATA1;
				else {
					pid = ep->pid;
					if (ep->comp_split)
						ep->pid = HCTSIZ_PID_DATA1;
				}
			}
			else 	{
				// must be PIPE_FLAGS_TOKEN_OUT
				dir = HCCHAR_EPDIR_OUT;
				pid = HCTSIZ_PID_DATA1;
				do_ping = ( ep->ping_needed ) ? HCTSIZ_DOPING : 0;
				ep->ping_needed = 0;
			}
			break;
	
		case BULK_ENDPOINT:
			if ( xelem->flags & PIPE_FLAGS_TOKEN_IN ) {
				dir     = HCCHAR_EPDIR_IN;
			} 
			else {
				do_ping = ( ep->ping_needed ) ? HCTSIZ_DOPING : 0;
				ep->ping_needed = 0;
			}
		break;

		case INT_ENDPOINT:
		case ISOCH_ENDPOINT:
			if ( xelem->flags & PIPE_FLAGS_TOKEN_IN ) {
				dir     = HCCHAR_EPDIR_IN;
			}
			break;
	}
	
	// set the transfer size, and the PID
	if ( !xelem->length  )
		xelem->npkt = 1;
	else 
		xelem->npkt = ( xelem->length / xelem->edesc->USBEd.wMaxPacketSize )  + 
					  ( ( xelem->length % xelem->edesc->USBEd.wMaxPacketSize ) ? 1 : 0 );
	
	if (ep->do_split) {
		if (ep->comp_split) {
			hc_slogf( xhc, _SLOG_DEBUG1, "%s: Complete split", __func__);
			if (!(xelem->flags & PIPE_FLAGS_TOKEN_IN))
				length = 0;
		}
		xelem->npkt = 1;
		if (length > xelem->edesc->USBEd.wMaxPacketSize)
			length = xelem->edesc->USBEd.wMaxPacketSize;

		 v = HW_Read32( xhc->IoBase, DWCOTG_HCSPLT( ep->chnum ) ) | (1 << 31);

		 if (ep->comp_split) {
			 v |= (1 << 16);
			 ep->comp_split = 0;
		}
		 else
			 v &= ~(1 << 16);
	
		 HW_Write32( xhc->IoBase, DWCOTG_HCSPLT( ep->chnum ), v );
	 
		v = ( length & HCTSIZ_XFER_SIZE_MSK )                        |
			( ( xelem->npkt << HCTSIZ_PKT_CNT_POS ) & HCTSIZ_PKT_CNT_MSK )  |
			pid | do_ping;
	}
	else
		v = ( xelem->length & HCTSIZ_XFER_SIZE_MSK )                        |
			( ( xelem->npkt << HCTSIZ_PKT_CNT_POS ) & HCTSIZ_PKT_CNT_MSK )  |
			pid | do_ping;	
		
   hc_slogf( xhc, _SLOG_DEBUG1, "%s:nchan %d xelem len %d npkt %d buffer %x flags %x maxpacket %d",
						__func__, ep->chnum, length, xelem->npkt, xelem->buffer, xelem->flags, 
						xelem->edesc->USBEd.wMaxPacketSize);

	HW_Write32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ), v );

	// set the dma address
	HW_Write32( xhc->IoBase, DWCOTG_HCDMA( ep->chnum ), (uint32_t) xelem->buffer );

	// set the direction, and enable the transfer
	v =  HW_Read32( xhc->IoBase, DWCOTG_HCCHAR( ep->chnum ) ) & ~HCCHAR_EPDIR_MSK & ~HCCHAR_CHAN_DISABLE;

#if 1
	if (ep->do_split && (xelem->edesc->USBEd.bmAttributes & 3) == INT_ENDPOINT) {
		v &=  ~HCCHAR_MCEC_MSK;
		v |= HCCHAR_MCEC_1XFER;
	}

	if ((xelem->edesc->USBEd.bmAttributes & 3)  == INT_ENDPOINT) {
		if (!(dwcotg_get_frame_number(xhc) & 0x1))
			v |= 1 << 29;
		else
			v &= ~(1 << 29);
	}
#endif

	HW_Write32( xhc->IoBase, DWCOTG_HCCHAR( ep->chnum ), v | dir | HCCHAR_CHAN_ENABLE );

//	ep->active_xfer = 1;
	
	pthread_mutex_unlock( &xhc->mutex );
}


static void continue_data_transfer( hctrl_t *xhc, epctx_t * ep )
{
    uint32_t        v;
    xelem_t         *xelem = ep->xelem;
    uint32_t        xfered_pkts;
    uint32_t        xfered_bytes;

    /* data transfer terminated for some reason ( nak, nyet, or xacterr), so we
     * need to start it up again... The dwcotg controller flushes all prefetched
     * data, and the channel pointers, transfer lengths are out of whack... 
     * recalculate these based on packet count
     */
	if (ep->do_split)
		return;
	
     // calculate how much data we have already transfered
     v = HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ) );
     xfered_pkts = xelem->npkt - ( ( v & HCTSIZ_PKT_CNT_MSK ) >> HCTSIZ_PKT_CNT_POS );
     xfered_bytes = xfered_pkts * xelem->edesc->USBEd.wMaxPacketSize;

     // set the new transfer length, and new dma address based on
     // previously transmitted data
     v &= ~HCTSIZ_XFER_SIZE_MSK;
     v |= ( xelem->length - xfered_bytes ) & HCTSIZ_XFER_SIZE_MSK;       
     HW_Write32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ), v );
     HW_Write32( xhc->IoBase, DWCOTG_HCDMA( ep->chnum ), ( (uint32_t) xelem->buffer )  + xfered_bytes );
     
    // re-enable the channel
    v = HW_Read32( xhc->IoBase, DWCOTG_HCCHAR( ep->chnum ) );    
    HW_Write32( xhc->IoBase, DWCOTG_HCCHAR( ep->chnum ), v | HCCHAR_CHAN_ENABLE );
}

static void complete_transfer( hctrl_t * xhc, int chidx, uint32_t intstatus ) 
{
	epctx_t				*ep = &xhc->ep_arr[chidx];
	st_URB_TRANSFER		*urb;
	uint32_t			cchar;
	int					done = 0;
	unsigned 			pid;

	if ( !ep->xelem ) {
		// no valid transfer on this endpoint, so ignore the interrupt
		return;
	}
	
	pthread_mutex_lock( &xhc->mutex );

	// do not comlete transfer on abort
	if ( ep->flags & EPFLAG_XFER_ABORT ) {
		ep->flags &=  ~EPFLAG_XFER_ABORT;
		pthread_mutex_unlock( &xhc->mutex );
		return;
	}
	ep->active_xfer = 0;
	urb = ep->xelem->urb;
	cchar = HW_Read32( xhc->IoBase, DWCOTG_HCCHAR(chidx) );
	hc_slogf( xhc, _SLOG_DEBUG1, "%s: chidx %d cchar %x intstatus %x",__func__,chidx, cchar, intstatus );
	if ( ( ( ep->xelem->edesc->USBEd.bmAttributes & 3 )  == CONTROL_ENDPOINT ) | 
		 ( ( ep->xelem->edesc->USBEd.bmAttributes & 3 )  == BULK_ENDPOINT    )  ){

		if ( cchar & HCCHAR_EPDIR_IN ) {
			// Control and Bulk IN 
			if ( intstatus & HCINT_CHAN_HALTED ) {
				if ( intstatus & HCINT_XFER_COMPLETE ) {
					// successful data transfer... read the xfer length
					if ( ( ep->xelem->flags & (PIPE_FLAGS_TOKEN_SETUP | PIPE_FLAGS_TOKEN_STATUS ) ) == 0 ) { 
						// update the ActualLength member in the Data Phase of the transfer only
						urb->ActualLength = urb->BufferLength - 
							( HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK ); 
						hc_slogf( xhc, _SLOG_DEBUG1, "%s: urb->BufferLength %x HCTSIZ %x",__func__, urb->BufferLength,
							 HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK);
					}
					if (ep->do_split) {
						if ((HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK ) != 0)
							ep->xelem->length = 0;
						
						 hc_slogf( xhc, _SLOG_DEBUG1, "%s: Read size %x req len %x MaxPktSz %x",__func__, 
							 HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK, ep->xelem->length,
							 ep->xelem->edesc->USBEd.wMaxPacketSize);
						 if (ep->xelem->length <= ep->xelem->edesc->USBEd.wMaxPacketSize)
							 ep->xelem->length = 0;
						 else {
							ep->xelem->length -= ep->xelem->edesc->USBEd.wMaxPacketSize;
							ep->xelem->buffer += ep->xelem->edesc->USBEd.wMaxPacketSize;
						 }
						
						 if (ep->xelem->length == 0) {
							ep->start_once = 0; 
							urb->Header.Status = EOK;
							done = 1;
						 }
						 else  {
							pid = HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ) ) & HCTSIZ_PID_MSK;

							if (pid == HCTSIZ_PID_DATA0)
								ep->pid = HCTSIZ_PID_DATA0;
							else
								ep->pid = HCTSIZ_PID_DATA1;

							// initiate transfer
							ep->start_once = 1;
						 }
					}
					else {
						urb->Header.Status = EOK;
						done = 1;
					}

				} else if ( intstatus & HCINT_STALL ) {
					urb->Header.Status = USBD_STATUS_STALL;
					done = 1;
				} else if ( intstatus & HCINT_BABBLE ) {
					urb->Header.Status = USBD_STATUS_DATA_OVERRUN;
					done = 1;
				} else if ( intstatus & HCINT_XACT_ERROR ) {
					if ( ep->xelem->errcnt >= 2 ) {
						urb->Header.Status = USBD_STATUS_DEV_NOANSWER;
						done = 1;
					} else {
						ep->xelem->errcnt++;
						continue_data_transfer( xhc, ep );
					}
				} else if (ep->do_split && ((intstatus & HCINT_TOGGLE_ERR) || (intstatus & HCINT_NAK))) {
					ep->xelem->errcnt = 0;
					
				} else  if ( ep->do_split && (intstatus & HCINT_ACK) ) {
					/*
					 * TODO: Make this enqueue code more generic once we support isoch 
					 * and/or support enqueue of multiple URBs on bulk enppoints
					 */
					ep->start_once = 0;
					ep->comp_split = 1;
 				}
			}
		} else {
			// Control and Bulk OUT 
			if ( intstatus & HCINT_CHAN_HALTED ) {
				if ( intstatus & HCINT_XFER_COMPLETE ) {
					// successful data transfer... read the xfer length
					if ( ( ep->xelem->flags & (PIPE_FLAGS_TOKEN_SETUP | PIPE_FLAGS_TOKEN_STATUS ) ) == 0 ) { 
						// update the ActualLength member in the Data Phase of the transfer only
						urb->ActualLength = urb->BufferLength - 
								( HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK ); 
					}
	
					if ( ( intstatus & HCINT_NYET ) && ( ep->xelem->flags & PIPE_FLAGS_TOKEN_OUT ) ) {
						// Last out tranfer responded with NYET... so
						// we need to enable the ping protocol before the next OUT transfer
						// on this endpoint
						ep->ping_needed = 1;
					}
                    
					if (ep->do_split) {
						 hc_slogf( xhc, _SLOG_DEBUG1, "%s: OUT Write size %x req len %x MaxPktSz %x",__func__, 
							 HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK, ep->xelem->length,
							 ep->xelem->edesc->USBEd.wMaxPacketSize);
						 ep->start_once = 0; 
					}

					urb->Header.Status = EOK;
					done = 1;
				} else if ( intstatus & HCINT_STALL ) {
					urb->Header.Status = USBD_STATUS_STALL;
					done = 1;
				} else if ( intstatus & ( HCINT_NAK | HCINT_XACT_ERROR | HCINT_NYET ) ) {
					if ( intstatus & HCINT_XACT_ERROR ) {
						hc_slogf( xhc, _SLOG_DEBUG1, "%s: XACT_ERROR chan %d errcnt %d",__func__,chidx, ep->xelem->errcnt);
						if ( ep->xelem->errcnt >= 2 ) {
							urb->Header.Status = USBD_STATUS_DEV_NOANSWER;
							done = 1;
						} else {
							ep->xelem->errcnt++;
							continue_data_transfer( xhc, ep );
						}
					} else {
						ep->xelem->errcnt = 0;
						continue_data_transfer( xhc, ep );
					}
				} else if ( ep->do_split && (intstatus & HCINT_ACK) ) {
					ep->start_once = 0; 
					ep->comp_split = 1;
 				}
			}
		}
	} else if ( ( ep->xelem->edesc->USBEd.bmAttributes & 3 )  == INT_ENDPOINT ) {
	
		if ( cchar & HCCHAR_EPDIR_IN ) {
			// interrupt IN
			if ( intstatus & HCINT_CHAN_HALTED ) {
				if ( intstatus & HCINT_XFER_COMPLETE ) {
					// successful data transfer... read the xfer length
					pid = HW_Read32( xhc->IoBase, DWCOTG_HFNUM) & HFNUM_FRAME_NUM_MSK;
					hc_slogf( xhc, _SLOG_DEBUG1, "%s: Interrupt IN COMPLETE chidx %x frame %x",__func__,chidx, pid);
					urb->ActualLength = urb->BufferLength - 
							( HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK ); 
					urb->Header.Status = EOK;
					done = 1;
					// remove transfer element from scheduler... transfer function
					// will add a new transfer
					ep->interval = 0;
				} else if ( intstatus & HCINT_STALL ) {
					urb->Header.Status = USBD_STATUS_STALL;
					done = 1;
				} else if ( intstatus & HCINT_BABBLE ) {
					urb->Header.Status = USBD_STATUS_DATA_OVERRUN;
					done = 1;
				} else if ( intstatus & ( HCINT_NAK | HCINT_TOGGLE_ERR ) ) {
					// Scheduler will reinitiate transfer at scheduled time
					ep->xelem->errcnt = 0;
					pid = HW_Read32( xhc->IoBase, DWCOTG_HFNUM) & HFNUM_FRAME_NUM_MSK;
					hc_slogf( xhc, _SLOG_DEBUG1, "%s: nchan %d NAK HCTSIZ %x Frame %x",__func__,  chidx,
							  HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK, pid);
                } else if ( intstatus & HCINT_XACT_ERROR ) {
					if ( ep->xelem->errcnt >= 2 ) {
						urb->Header.Status = USBD_STATUS_DEV_NOANSWER;
						done = 1;
					} else {
						// Scheduler will reinitiate transfer at scheduled time
						ep->xelem->errcnt++;
					}
				} else if ( intstatus & HCINT_NYET ) {
					hc_slogf( xhc, _SLOG_DEBUG1, "%s: Interrupt IN EP NYET chidx %x Frame %x",__func__,
							  chidx, HW_Read32( xhc->IoBase, DWCOTG_HFNUM) & HFNUM_FRAME_NUM_MSK);
					hc_slogf( xhc, _SLOG_DEBUG1, "%s: HCTSIZ %x",__func__, 
							  HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK);
					ep->xelem->errcnt = 0;
				} else if ( ep->do_split && (intstatus & HCINT_ACK) ) {
 					 pid = HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ( ep->chnum ) ) & HCTSIZ_PID_MSK;
					 hc_slogf( xhc, _SLOG_DEBUG1, "%s: Interrupt IN EP ACK chidx %x pid %x Frame %x",__func__,
							   chidx, pid, HW_Read32( xhc->IoBase, DWCOTG_HFNUM) & HFNUM_FRAME_NUM_MSK);
					ep->comp_split = 1;
				}
			}
		} else {
			// interrupt out
			if ( intstatus & HCINT_CHAN_HALTED ) {
				if ( intstatus & HCINT_XFER_COMPLETE ) {
					// successful data transfer... read the xfer length
					urb->ActualLength = urb->BufferLength - 
							( HW_Read32( xhc->IoBase, DWCOTG_HCTSIZ(chidx) ) & HCTSIZ_XFER_SIZE_MSK );
					urb->Header.Status = EOK;
					done = 1;
					// remove transfer element from scheduler... transfer function
					// will add a new transfer
					ep->interval = 0;	
				} else if ( intstatus & HCINT_STALL ) {
					urb->Header.Status = USBD_STATUS_STALL;
					done = 1;
				} else if ( intstatus & HCINT_NAK) {
					// Scheduler will reinitiate transfer at scheduled time
					ep->xelem->errcnt = 0;
				} else if ( intstatus & HCINT_XACT_ERROR ) {
					if ( ep->xelem->errcnt >= 2 ) {
						urb->Header.Status = USBD_STATUS_DEV_NOANSWER;
						done = 1;
					} else {
						// Scheduler will reinitiate transfer at scheduled time
						ep->xelem->errcnt++;
					}
				}
				else if ( ep->do_split && (intstatus & HCINT_ACK) ) {
					 hc_slogf( xhc, _SLOG_DEBUG1, "%s: Interrupt OUT EP ACK chidx %x",__func__,chidx);
					 ep->comp_split = 1; 
				}
			}
		}
	} else {
		// ISOCH ... no support yet
	}
	if ( done ) {
		ep->flags &= ~EPFLAG_ACTIVE_XFER;
		xelem_destroy( xhc, ep->xelem );

		if ( ( ep->xelem->edesc->USBEd.bmAttributes & 3 ) == CONTROL_ENDPOINT ) {
			pthread_sleepon_lock( );
			urb->Header.Semaphore = 1;
			pthread_sleepon_signal( &urb->Header.Semaphore );
			pthread_sleepon_unlock( );
		} else {
			urb->CallBackFunction( urb );
		}
	}

	pthread_mutex_unlock( &xhc->mutex );
}
                                    
static void channel_interrupt_handler( hctrl_t * xhc, int chidx ) 
{
    uint32_t    intstatus;

    intstatus = HW_Read32( xhc->IoBase, DWCOTG_HCINT(chidx) );
    HW_Write32( xhc->IoBase, DWCOTG_HCINT(chidx), intstatus );
    
    
    // transaction is complete, finish urb processing
    if ( intstatus & ( HCINT_XFER_COMPLETE | HCINT_CHAN_HALTED ) ) {
        complete_transfer( xhc, chidx, intstatus );
    }
}

_uint32 dwcotg_controller_interrupt( st_USB_Hc *hc )
{
	hctrl_t     *xhc = hc->hc_data;
	uint32_t    intstatus;
	int         i;
	uint32_t	frame;
	static int 	frame_count = 0;
	static int 	sframe_count = 0;
	

	intstatus = HW_Read32( xhc->IoBase, DWCOTG_GINTSTS ) & 
				HW_Read32( xhc->IoBase, DWCOTG_GINTMSK );


	if (intstatus & GINTMSK_SOF) {
		 for(i = 0; i < xhc->nchan; i++) {
			 // Periodic EP branch
			 if (xhc->ep_arr[i].interval) {
				 frame = HW_Read32( xhc->IoBase, DWCOTG_HFNUM) & HFNUM_FRAME_NUM_MSK;

				 if (xhc->ep_arr[i].comp_split) {
//					hc_slogf( xhc, _SLOG_DEBUG1, "%s: INT Do comp_split", __func__);
					if ( !xhc->ep_arr[i].active_xfer )
					if (MsgSendPulse( xhc->coid, xhc->scheduler_priority + 1, SCHED_DOSTART, i ) == -1)
						hc_slogf( xhc, _SLOG_ERROR, "%s: Filed to send DOSTART pulse", __func__ ); 
					break;
				 } 

				if (frame >= xhc->ep_arr[i].start_frame) {
					if ((frame & 0x3F00) != (xhc->ep_arr[i].start_frame & 0x3F00))
						continue;
//					hc_slogf( xhc, _SLOG_DEBUG1, "%s: Start frame %x Frame %x", __func__, 
//							  xhc->ep_arr[i].start_frame, frame ); 
					xhc->ep_arr[i].start_frame = (frame + xhc->ep_arr[i].interval) & HFNUM_FRAME_NUM_MSK;
					if ( !xhc->ep_arr[i].active_xfer )
					if (MsgSendPulse( xhc->coid, xhc->scheduler_priority + 1, SCHED_DOSTART, i ) == -1)
						hc_slogf( xhc, _SLOG_ERROR, "%s: Filed to send DOSTART pulse", __func__ ); 
					break;
				}
			 } 
			 else {
				// Non-Periodic EP branch
				if (xhc->ep_arr[i].comp_split || xhc->ep_arr[i].start_once) {
					frame_count++;
					if (frame_count >= 7) {
						if ( !xhc->ep_arr[i].active_xfer )
						if (MsgSendPulse( xhc->coid, xhc->scheduler_priority + 1, SCHED_DOSTART, i ) == -1)
							hc_slogf( xhc, _SLOG_ERROR, "%s: Filed to send DOSTART pulse", __func__ ); 
						frame_count = 0;
						break;
					}
				}
			}
		 }
		// clear Start of Frame interrupt
		HW_Write32( xhc->IoBase, DWCOTG_GINTSTS, GINTSTS_SOF ); 
	}
	
	// Channel interrupt
	if ( intstatus & GINTSTS_HCHAN ) {
		// find channel interrupt that has fired
		for(i=0; i < xhc->nchan; i++ ) {
			if ( HW_Read32( xhc->IoBase, DWCOTG_HAINT )     & 
				 HW_Read32( xhc->IoBase, DWCOTG_HAINTMSK )  & 
				 ( 1 << i )                                  ) {
	
				// Channel interrupt has fired... 
				channel_interrupt_handler( xhc, i );
			}
		}
	}

	// clear interrupt 
	HW_Write32( xhc->IoBase, DWCOTG_GINTSTS, intstatus );
 
	return EOK;
}



static _uint32 control_xfer_wait( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc )
{
    hctrl_t     *xhc = hc->hc_data;
    int         status;

	pthread_sleepon_lock( );
	while( !urb->Header.Semaphore ) {
		if( (status = pthread_sleepon_timedwait( &urb->Header.Semaphore, urb->Header.Timeout * USB_DELAY_MULT_MS * 10 )) ) {
			pthread_sleepon_unlock( );
			hc_slogf( xhc, _SLOG_ERROR, "%s Timeout on Control Transfer status = %d",__func__, status );
			dwcotg_ctrl_transfer_abort( hc, urb, edesc );
			urb->Header.Status = USBD_STATUS_NOT_ACCESSED;
			return( -1 );
		}
	}
	pthread_sleepon_unlock( );
	return( urb->Header.Status ? -1 : EOK );

}

// Send a control packet to device
_uint32 dwcotg_ctrl_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,
            _uint8 *buffer, _uint32 length, _uint32 flags )
{
	epctx_t    *ep = edesc->PtrUSBEndpoint;    
	uint32_t   status = EOK;
	hctrl_t    *xhc = hc->hc_data;
	xelem_t    *xelem;

	urb->Header.Semaphore	= 0;
	
	// create a transfer element based on transfer function parameters
	xelem = xelem_create( xhc, urb, edesc, buffer, length, flags );
	if ( xelem == NULL ) {
		hc_slogf( xhc, _SLOG_ERROR, "%s: Failed to create xfer element",__func__);
		return -1;
	}
	// mark endpoint indicating that there is an active xfer on the endpoint
	ep->flags |= EPFLAG_ACTIVE_XFER;
	hc_slogf( xhc, _SLOG_DEBUG1, "%s: do transfer xelem %x buffer %x len %d",__func__, xelem, buffer, length);
	// do transfer
	start_data_transfer( xhc, xelem );    
	status = control_xfer_wait( hc, urb, edesc );
	if ( status ) {
		hc_slogf( xhc, _SLOG_ERROR, "%s:Control transfer failed. status = %d flags = 0x%x "
			,__func__, status, flags );
	}
		
	return( status );
}

_uint32 dwcotg_bulk_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,
            _uint8 *buffer, _uint32 length, _uint32 flags )
{
    epctx_t    *ep = edesc->PtrUSBEndpoint;    
    hctrl_t    *xhc = hc->hc_data;
    xelem_t    *xelem;
     
    if ( ep->flags & EPFLAG_ACTIVE_XFER ) {
        /* only support 1 bulk transfer at a time for now */
        return EBUSY;
    }
    // mark endpoint indicating that there is an active xfer on the endpoint
    ep->flags |= EPFLAG_ACTIVE_XFER;
    
	hc_slogf( xhc, _SLOG_DEBUG1, "%s: do BULK buffer %x len %d",__func__, buffer, length);
    // create a transfer element based on transfer function parameters
    xelem = xelem_create( xhc, urb, edesc, buffer, length, flags );
    if ( xelem == NULL ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: Failed to create xfer element",__func__);
        return -1;
    }
    
    // do transfer
    start_data_transfer( xhc, xelem );    

    return EOK;
}

static unsigned log2_rnd_local( unsigned v) {
    unsigned cnt=1;
    
    while ( v != 1 ) {
		cnt *= 2;
		v--;
    }
    
    return cnt;
}

// Send/Receive a INT packet to/from device
_uint32 dwcotg_int_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,
            _uint8 *buffer, _uint32 length, _uint32 flags )
{
	hctrl_t         *xhc = hc->hc_data;
	epctx_t         *ep = edesc->PtrUSBEndpoint;
	xelem_t         *xelem;

	if ( ep->flags & EPFLAG_ACTIVE_XFER )  {
		hc_slogf( xhc, _SLOG_INFO, "%s: Already have xfer enqueued on EP",__func__);
		return EBUSY;
	}
	// mark endpoint indicating that there is an active xfer on the endpoint
	ep->flags |= EPFLAG_ACTIVE_XFER;

	// create a transfer element based on transfer function parameters
	xelem = xelem_create( xhc, urb, edesc, buffer, length, flags );
	if ( xelem == NULL ) {
		hc_slogf( xhc, _SLOG_ERROR, "%s: Failed to create xfer element",__func__);
		return -1;
	}
	hc_slogf( xhc, _SLOG_INFO, "%s: do INT buffer %x len %d",__func__, buffer, length);
	/*
	 * TODO: Make this enqueue code more generic once we support isoch 
	 * and/or support enqueue of multiple URBs on bulk enppoints
	 */

	ep->xelem = xelem;
	ep->start_frame = ((HW_Read32( xhc->IoBase, DWCOTG_HFNUM) & HFNUM_FRAME_NUM_MSK) + 64) & ~(64 - 1) & 
																							HFNUM_FRAME_NUM_MSK;

	if (ep->do_split)
		ep->interval = (edesc->USBEd.bInterval * 1000) / 125;
	else
		ep->interval = log2_rnd_local(edesc->USBEd.bInterval);

	hc_slogf( xhc, _SLOG_INFO, "%s: do INT interval %d bInterval %d start frame %d",__func__,
			  ep->interval, edesc->USBEd.bInterval,ep->start_frame );

	return EOK;
}



_uint32 dwcotg_isoch_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,
            _uint8 *buffer, _uint32 length, _uint32 flags )
{
    hctrl_t   *xhc = hc->hc_data;
    hc_slogf( xhc, _SLOG_INFO, "%s: Entry",__func__);
    return EOK;
}


_uint32 dwcotg_check_port_status( st_USB_Hc *hc, _uint32 *change_bitmap)
{
    hctrl_t     *xhc = hc->hc_data;

    *change_bitmap = 0;
    
    if ( HW_Read32( xhc->IoBase, DWCOTG_HPRT ) & HPRT_CONNECT_DETECT ) {
        // Detected a device connect event
        *change_bitmap = 1;
        xhc->flags |= HCFLAG_PORT0_CONNECTED;
        HW_Write32Or( xhc->IoBase, DWCOTG_HPRT, HPRT_CONNECT_DETECT );
        hc_slogf( xhc, _SLOG_INFO, "%s: PORT CHANGE - CONNECTED ",__func__);

    } else if (  ( xhc->flags & HCFLAG_PORT0_CONNECTED ) && 
                 ( HW_Read32( xhc->IoBase, DWCOTG_HPRT ) & HPRT_CONNECT_STS ) == 0  ) {
        // no status bit to indicate a disconnect event, therefore, we look for
        //  a 1 -> 0 transition on CONNECT_STS                            
        *change_bitmap = 1;
        xhc->flags  &= ~HCFLAG_PORT0_CONNECTED;
        hc_slogf( xhc, _SLOG_INFO, "%s: PORT CHANGE - DISCONNECTED ",__func__);
    }
    
    return EOK;
}

_uint32 dwcotg_set_bus_state( st_USB_Hc *Hc, _uint32 bus_state )
{

    switch ( bus_state ) {
        
        case USB_BUS_STATE_START :
            break;
        case USB_BUS_STATE_STOP :
            break;
        case USB_BUS_STATE_RESET :
            break;
        case USB_BUS_STATE_SUSPENDED :
            break;
        case USB_BUS_STATE_RESUME :
            break;
        case USB_BUS_STATE_OPERATIONAL :
            break;
        default:
            break;
    }

    return EOK;
}


static void defaults_set( hctrl_t *xhc )
{
    xhc->verbosity              = _SLOG_ERROR;
    xhc->scheduler_priority     = 21;
    xhc->nchan                  = DWCOTG_N_CHAN_DEFAULT;
}
static void process_args( hctrl_t *xhc, char *options )
{
    char        *value;

    if ( !options || *options == 0 )
        return;

    while ( options && *options != '\0') {
        switch( getsubopt( &options, cmdline_opts, &value ) ) {
            case DWCOTG_OPT_VERBOSITY :
                if ( value )
                    xhc->verbosity = strtol( value, 0, 10 );
                else 
                    xhc->verbosity = _SLOG_INFO;
                break;
            case DWCOTG_OPT_SCHEDULER_PRIO :
                if ( value ) {
                    xhc->scheduler_priority = strtol( value, 0, 10 );
                }
                break;

            case DWCOTG_OPT_NUMBER_CHANNELS :
                if ( value ) {
                    xhc->nchan = strtol( value, 0, 10 );
                    
                    if ( xhc->nchan > DWCOTG_N_CHAN_MAX ) {
                        xhc->nchan = DWCOTG_N_CHAN_MAX;
                        hc_slogf( xhc, _SLOG_WARNING, "%s: setting number of channels to max = %d ",__func__,DWCOTG_N_CHAN_MAX);
                    }
                }
                break;
            default : 
                break;
        }
    }

}

static void capabilities_set( st_USB_Hc *Hc )
{
    
    Hc->capabilities =  USBD_HCD_CAP_CNTL           | 
                        USBD_HCD_CAP_INTR           |
                        USBD_HCD_CAP_BULK           | 
                        USBD_HCD_CAP_HIGH_SPEED     | 
                       //USBD_HCD_CAP_BULK_SG       | 
                       //USBD_HCD_CAP_ISOCH         |
                       //USBD_HCD_CAP_ISOCH_STREAM  |
                        0                           ;
    
	Hc->MaxTransferSize 			= HCTSIZ_XFER_SIZE_MSK;
	Hc->MaxUnalignedTransferSize 	= HCTSIZ_XFER_SIZE_MSK;
	Hc->buff_alignment_mask 		= 1;
}


#define RESET_TIMEOUT    1000
static int reset_controller( hctrl_t *xhc ) 
{
    int         i = 0;
    int         retval = EOK;
    uint32_t    reset_mask =    GRSTCTL_SOFT_RST | GRSTCTL_HCLK_SOFT_RST | 
                                GRSTCTL_FRAME_RST | GRSTCTL_RXFIFO_FLSH | 
                                GRSTCTL_TXFIFO_FLSH;
    
    
    // Reset all components listed in 'reset_mask', and all TXFIFOs should
    // be affected
    HW_Write32( xhc->IoBase, DWCOTG_GRSTCTL, reset_mask | GRSTCTL_TXFIFO_ALL );
    
    // poll to make sure reset is complete
    while (  ( i++ < RESET_TIMEOUT ) && 
             ( HW_Read32( xhc->IoBase, DWCOTG_GRSTCTL ) & reset_mask ) ) {
        usleep(1000);
    }
    
    if ( i >= RESET_TIMEOUT ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: Could not reset the controller  DWCOTG_GRSTCTL = 0x%x"
            ,__func__, HW_Read32( xhc->IoBase, DWCOTG_GRSTCTL ) );
        retval = -1;
    }
    
    return retval;
}

_uint32 dwcotg_controller_init( st_USB_Hc *hc, _uint32 flags, char *args )
{
	pthread_mutexattr_t     mattr;
    hctrl_t                 *xhc;
    int                     status = EOK;
 
    hc->cname = "DWCOTG";
    
    // allocate host controller context
    xhc = hc->hc_data = calloc( 1, sizeof( hctrl_t ) );
    if ( xhc == NULL ) {
        hc_slogf( NULL, _SLOG_ERROR, "%s: Could not alloc memory for ctx",__func__);
        status = ENOMEM;
        goto error;
    }

    // init driver mutex
	pthread_mutexattr_init( &mattr );
	pthread_mutexattr_setrecursive( &mattr, PTHREAD_RECURSIVE_ENABLE );
	status = pthread_mutex_init( &xhc->mutex, &mattr );
    if( status != EOK ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: Could not init mutex. status = %d",__func__,status);
        goto error2;
	}
    
    // map controller registers
    xhc->IoBase =  mmap_device_memory(  0, 
                                        DWCOTG_SIZE, 
                                        PROT_READ | PROT_WRITE | PROT_NOCACHE,
                                        MAP_SHARED | MAP_PHYS,
                                        PCI_MEM_ADDR( hc->pci_inf->CpuBaseAddress[0] ) );
    if ( xhc->IoBase == MAP_FAILED ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: could not mmap controller registers",__func__);
        status = ENODEV;
        goto error3;        
    }
    
    // set driver defaults, host controller capabilities and override with 
    // command line options
    capabilities_set( hc );
    defaults_set( xhc );
    process_args( xhc, args );

    //Initiallize transfer descriptor pool     
    status = xelem_pool_create( xhc );
    if ( status != EOK ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: couldn't create transfer element pool",__func__);
        goto error4;
    }

    // create software scheduler
    if ( scheduler_create( xhc ) != EOK ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: couldn't create the software scheduler",__func__);
        goto error5;
    }
    
    /*
    *  Initializing the Controller... 
    */
//	port_power_disable( hc );
	
	// reset controller
    status = reset_controller( xhc );
    if ( status ) {
        hc_slogf( xhc, _SLOG_ERROR, "%s: couldn't reset the controller",__func__);
        goto error6;
    }
	hc_slogf( xhc, _SLOG_ERROR, "Verbose level %d", xhc->verbosity );
	hc_slogf( xhc, _SLOG_INFO, "Verbose level %d", xhc->verbosity );
	hc_slogf( xhc, _SLOG_ERROR, "%s: GUSBCFG %x",__func__,
															HW_Read32(xhc->IoBase, DWCOTG_GUSBCFG));
	
    // configure as AHB Master... TODO: try burst MODES
    HW_Write32Or( xhc->IoBase, DWCOTG_GAHBCFG , GAHBCFG_DMA_EN /*| GAHBCFG_BURSTLEN_INCR16*/ );	
	
	// Force HOST mode
    HW_Write32Or( xhc->IoBase, DWCOTG_GUSBCFG , GUSBCFG_FORCE_HOST_MODE /*  | GUSBCFG_PHY_SEL_SERIAL | GUSBCFG_PHYIF16 */);
	delay(500);
	
	hc_slogf( xhc, _SLOG_ERROR, "%s: HCFG %x GUSBCFG %x GSNPSID %x",__func__,
																   HW_Read32(xhc->IoBase, DWCOTG_HCFG),
																   HW_Read32(xhc->IoBase, DWCOTG_GUSBCFG),
																   HW_Read32(xhc->IoBase, DWCOTG_GSNPSID));
	
	hc_slogf( xhc, _SLOG_ERROR, "%s:HWCFG2 %x HWCFG3 %x HWCFG4 %x",__func__,
																   HW_Read32(xhc->IoBase, DWCOTG_GHWCFG2),
																   HW_Read32(xhc->IoBase, DWCOTG_GHWCFG3),
																   HW_Read32(xhc->IoBase, DWCOTG_GHWCFG4));	
	hc_slogf( xhc, _SLOG_ERROR, "%s: GPVNDCTL %x", __func__, HW_Read32(xhc->IoBase, DWCOTG_GPVNDCTL));

    // Configure Interrupts
    HW_Write32( xhc->IoBase, DWCOTG_GINTSTS, 0xffffffff); 
    HW_Write32( xhc->IoBase, DWCOTG_GINTMSK , GINTMSK_HCHAN | GINTMSK_SOF );
    HW_Write32( xhc->IoBase, DWCOTG_HAINTMSK , 0 );
    HW_Write32Or( xhc->IoBase, DWCOTG_GAHBCFG , GAHBCFG_GLBL_INTR_MSK );
    
    // Enable the port Power
    port_power_enable( hc );

    
    return EOK;
  
error6:
    scheduler_destroy( xhc );
error5:
    xelem_pool_destroy( xhc );
error4:    
    munmap_device_memory( xhc->IoBase, DWCOTG_SIZE);
error3:
    pthread_mutex_destroy( &xhc->mutex );
error2:
    free( xhc );
error:
    return status;
}

_uint32 dwcotg_controller_shutdown( st_USB_Hc *Hc )
{
    hctrl_t                 *xhc = Hc->hc_data;

    // mask interrupts just in case this is a shared interrupt
    HW_Write32( xhc->IoBase, DWCOTG_GINTMSK , 0 );
    HW_Write32And( xhc->IoBase, DWCOTG_GAHBCFG , ~GAHBCFG_GLBL_INTR_MSK );
    
    // destroy resources
    scheduler_destroy( xhc );
    xelem_pool_destroy( xhc );
    munmap_device_memory( xhc->IoBase, DWCOTG_SIZE);
    pthread_mutex_destroy( &xhc->mutex );
    free( xhc );

    return EOK;
}


int dwcotg_init( void *dll_hdl, dispatch_t *dpp, io_usb_self_t *iousb, char *options )
{
    return EOK;
}

int dwcotg_shutdown( void *dll_hdl )
{
    return EOK;
}
