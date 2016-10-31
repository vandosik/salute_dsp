/*
 * $QNXtpLicenseC:  
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/


#ifndef __DWCOTG_H_INCLUDED
#define __DWCOTG_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <gulliver.h>
#include <malloc.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <sys/io-usb.h>
#include "hcd/usb.h"
#include <sys/fbma.h>
#include <sys/psched.h>
#include <sys/neutrino.h>


/* defines */

#define DWCOTG_SIZE                 0x1000
#define DWCOTG_N_ROOT_PORTS         1
#define DWCOTG_N_CHAN_DEFAULT       4
#define DWCOTG_N_CHAN_MAX           32


#define MAX_XFER_ELEM               128     // number of outstanding transfers

#define DWCOTG_FRAME_ROLLOVER       0x4000

/* Overridable defines */
#ifndef DWCOTG_DLL_NAME
    #define DWCOTG_DLL_NAME "devu-dwcotg.so"
#endif



/* registers */

// The reference manual for ppc reverses the bit ordering within a  register,
// so RBIT macro is use to make register definitions map to to the ref manual
#define RBIT(n)                         ( 31 - (n) )    // used to reverse bits

/* general register definitions */

#define DWCOTG_GOTGCTL                  0x0
    #define GOTGCTL_HNP_EN                  ( 1 << RBIT(21) )    /* ( HstSetHNPEn ) */
    #define GOTGCTL_CON_ID_STS              ( 1 << RBIT(15) )    /* ( ConIDSts ) */
    #define GOTGCTL_DBNC_TIME               ( 1 << RBIT(14) )    /* ( DbncTime ) */
    #define GOTGCTL_ASESSION_VLD            ( 1 << RBIT(13) )    /* ( ASesVld ) */

#define DWCOTG_GOTGINT                  0x4
    #define GOTGINT_SESSION_END             ( 1 << RBIT(29) )    /* ( SesEndDet ) */
    #define GOTGINT_SESSION_STS_CHNG        ( 1 << RBIT(23) )    /* ( SesReqSucStsChng ) */
    #define GOTGINT_HNP_SUCESS_CHNG         ( 1 << RBIT(22) )    /* ( HstNegSucStsChng ) */
    #define GOTGINT_HNP_DETECT              ( 1 << RBIT(14) )    /* ( HstNegDet ) */
    #define GOTGINT_ADEV_TIMEOUT            ( 1 << RBIT(13) )    /* ( ADevTOUTChg ) */ 
    #define GOTGINT_DBNCE_DONE              ( 1 << RBIT(12) )    /* ( DbnceDone ) */

#define DWCOTG_GAHBCFG                  0x8
    #define GAHBCFG_GLBL_INTR_MSK           ( 1 << RBIT(31) )    /* ( BlblIntrMsk ) */
    #define GAHBCFG_BURSTLEN_MSK            ( 0xf << RBIT(30) )  /* ( HBstLen ) */
    #define GAHBCFG_BURSTLEN_SINGLE         ( 0x0 << RBIT(30) )
    #define GAHBCFG_BURSTLEN_INCR           ( 0x1 << RBIT(30) )
    #define GAHBCFG_BURSTLEN_INCR4          ( 0x3 << RBIT(30) )
    #define GAHBCFG_BURSTLEN_INCR8          ( 0x5 << RBIT(30) )
    #define GAHBCFG_BURSTLEN_INCR16         ( 0x7 << RBIT(30) )
    #define GAHBCFG_DMA_EN                  ( 1 << RBIT(26) )    /* ( DmaEn ) */
    #define GAHBCFG_NP_TXFIFO_EMP_LVL       ( 1 << RBIT(24) )    /* ( NPTxFEmpLvl ) */
    #define GAHBCFG_P_TXFIFO_EMP_LVL        ( 1 << RBIT(23) )    /* ( PTxFEmpLvl ) */

#define DWCOTG_GUSBCFG                  0xc
    #define GUSBCFG_TIMEOUT_CAL_MSK         ( 0x7 << RBIT(31) )  /* ( TOutCal ) */
	#define GUSBCFG_PHYIF16                 ( 1 << RBIT(28) )    /* ( PHYIF16 ) */
    #define GUSBCFG_ULPI_UTMI_SEL           ( 1 << RBIT(27) )    /* ( must be 1 ) */
    #define GUSBCFG_PHY_SEL_SERIAL          ( 1 << RBIT(25) )    /* ( PhySel ) */ 
    #define GUSBCFG_SRP_CAP                 ( 1 << RBIT(23) )    /* ( SRPCap ) */
    #define GUSBCFG_HNP_CAP                 ( 1 << RBIT(22) )    /* ( HNPCap ) */
	#define GUSBCFG_PHY_LP_CLK_SEL          ( 1 << RBIT(16) )    /* ( PhyLPwrClkSel ) */
    #define GUSBCFG_FORCE_HOST_MODE         ( 1 << RBIT(2) )     /* ( ForceHstMode ) */
    #define GUSBCFG_FORCE_DEVICE_MODE       ( 1 << RBIT(1) )     /* ( ForceDevMode ) */
    
#define DWCOTG_GRSTCTL                  0x10
    #define GRSTCTL_SOFT_RST               ( 1 << RBIT(31) )   /* ( CSftRst ) */
    #define GRSTCTL_HCLK_SOFT_RST          ( 1 << RBIT(30) )   /* ( HSftRst ) */
    #define GRSTCTL_FRAME_RST              ( 1 << RBIT(29) )   /* ( FrmCntrrRst ) */
    #define GRSTCTL_INTOK_Q_FLSH           ( 1 << RBIT(28) )   /* ( INTknQFlsh ) */
    #define GRSTCTL_RXFIFO_FLSH            ( 1 << RBIT(27) )   /* ( RxFFlsh ) */
    #define GRSTCTL_TXFIFO_FLSH            ( 1 << RBIT(26) )   /* ( TxFFlsh ) */
    #define GRSTCTL_TXFIFO_MSK             ( 0x1f << RBIT(25) ) /* ( TxFNum ) */
    #define GRSTCTL_TXFIFO_NUM(n)          ( ( (n) << RBIT(25) ) & GRSTCTL_TXFIFO_MSK )
    #define GRSTCTL_TXFIFO_ALL             ( ( 0x10 << RBIT(25) ) & GRSTCTL_TXFIFO_MSK )
    
    #define GRSTCTL_DMA_REQ                ( 1 << RBIT(1) )     /* ( DMAReq ) */
    #define GRSTCTL_AHB_MASTER_IDLE        ( 1 << RBIT(0) )     /* ( AHBIdle ) */

#define DWCOTG_GINTSTS                  0x14
    #define GINTSTS_HOST_MODE               ( 1 << RBIT(31) )   /* ( CurMod ) */
    #define GINTSTS_MISMATCH                ( 1 << RBIT(30) )   /* ( ModeMis ) */
    #define GINTSTS_OTG                     ( 1 << RBIT(29) )   /* ( OTGInt ) */
    #define GINTSTS_SOF                     ( 1 << RBIT(28) )   /* ( Sof ) */
    #define GINTSTS_RXFIFO_NEMPTY           ( 1 << RBIT(27) )   /* ( RxFLvl ) */
    #define GINTSTS_NP_TXFIFO_EMPTY         ( 1 << RBIT(26) )   /* ( NPTxFEmp ) */
    #define GINTSTS_INCOMP_XFER             ( 1 << RBIT(10) )   /* ( incompIP ) */
    #define GINTSTS_HPORT                   ( 1 << RBIT(7) )    /* ( PrInt ) */
    #define GINTSTS_HCHAN                   ( 1 << RBIT(6) )    /* ( HChInt ) */
    #define GINTSTS_P_TXFIFO_EMPTY          ( 1 << RBIT(5) )    /* ( PTxFEmp ) */
    #define GINTSTS_CONN_STS                ( 1 << RBIT(3) )    /* ( ConIDStsChng ) */
    #define GINTSTS_DISCON_DETECT           ( 1 << RBIT(2) )    /* ( DisconnInt ) */
    #define GINTSTS_SESS_REQ                ( 1 << RBIT(1) )    /* ( SessReqInt ) */
    #define GINTSTS_RESUME                  ( 1 << RBIT(0) )    /* ( WkUpInt ) */

    
#define DWCOTG_GINTMSK                  0x18
    #define GINTMSK_MISMATCH                ( 1 << RBIT(30) )    /* ( ModeMisMsk) */
    #define GINTMSK_OTG                     ( 1 << RBIT(29) )    /* ( OTGIntMsk ) */
    #define GINTMSK_SOF                     ( 1 << RBIT(28) )    /* ( SofMsk ) */
    #define GINTMSK_RXFIFO_NEMPTY           ( 1 << RBIT(27) )    /* ( RxFlvlMsk ) */
    #define GINTMSK_NP_TXFIFO_EMPTY         ( 1 << RBIT(26) )    /* ( NPTxFempMsk ) */
    #define GINTMSK_HPORT                   ( 1 << RBIT(7) )     /* ( PrtIntMsk ) */
    #define GINTMSK_HCHAN                   ( 1 << RBIT(6) )     /* ( HChIntMsk ) */
    #define GINTMSK_P_TXFIFO_EMPTY          ( 1 << RBIT(5) )     /* ( PTxFEmpMsk ) */
    #define GINTMSK_CONN_STS                ( 1 << RBIT(3) )     /* ( ConIDStsChngMsk) */
    #define GINTMSK_DISCON_DETECT           ( 1 << RBIT(2) )     /* ( DisconnIntMsk ) */
    #define GINTMSK_SESS_REQ                ( 1 << RBIT(1) )     /* ( SessReqIntMsk ) */
    #define GINTMSK_RESUME                  ( 1 << RBIT(0) )     /* ( WkUpIntMsk ) */

#define DWCOTG_GRXSTSR                  0x1c
#define DWCOTG_GRXSTSP                  0x20
    #define GRXSTSR_CHAN_NUM_MSK            ( 0xf << RBIT(31) )    /* ( ChNum ) */
    #define GRXSTSR_BCNT_POS                RBIT(27)               /* ( BCnt ) */ 
    #define GRXSTSR_BCNT_MSK                ( 0x7ff << GRXSTSR_BCNT_POS )
    #define GRXSTSR_DPID_MSK                ( 3 << RBIT(16) )      /* ( DPID) */
    #define GRXSTSR_DPID_DATA0              ( 0 << RBIT(16) )
    #define GRXSTSR_DPID_DATA1              ( 1 << RBIT(16) )
    #define GRXSTSR_DPID_DATA2              ( 2 << RBIT(16) )
    #define GRXSTSR_DPID_MDATA              ( 3 << RBIT(16) )
    #define GRXSTSR_PKTSTS_MSK              ( 0xf << RBIT(14) )    /* ( PktSts) */
    #define GRXSTSR_PKTSTS_INPKT_RX         ( 0x2 << RBIT(14) )
    #define GRXSTSR_PKTSTS_INXFER_CMP       ( 0x3 << RBIT(14) )
    #define GRXSTSR_PKTSTS_TOGGLE_ERR       ( 0x5 << RBIT(14) )
    #define GRXSTSR_PKTSTS_CH_HALT          ( 0x7 << RBIT(14) )

#define DWCOTG_GRXFSIZ                  0x24
    #define GRXFSIZ_FDEPTH_MSK               0xffff
    
#define DWCOTG_GNPTXFSIZ                0x28
    #define GNPTXFSIZ_START_ADDR_MSK    0xffff      /* ( NPTxFDep ) */
    #define GNPTXFSIZ_FDEPTH_POS        RBIT(15)
    #define GNPTXFSIZ_FDEPTH_MSK        ( 0xffff << GNPTXFSIZ_FDEPTH_POS )
    
#define DWCOTG_GNPTXSTS                 0x2c
    #define GNPTXSTS_FIFO_SPACE_AVAIL_MSK       0xffff    /* ( NPTxFSpcAvail ) */
    #define GNPTXSTS_TX_Q_SPACE_AVAIL_MSK       RBIT(15)  /* ( NPTxQSpcAvail ) */
    #define GNPTXSTS_TX_Q_SPACE_AVAIL_POS       ( 0xff << GNPTXSTS_TX_Q_SPACE_AVAIL_MSK )
    #define GNPTXSTS_TOP_Q_POS                  RBIT(7)   /* ( NPTxQTop ) */  
    #define GNPTXSTS_TOP_Q_MSK                  ( 0x7f << GNPTXSTS_TX_TOP_Q_POS )
    
    
#define DWCOTG_GPVNDCTL                 0x34
    #define GPVNDCTL_DATA8_REG_MSK          0xff        /* ( RegData ) */
    #define GPVNDCTL_VENDORCTRL_ADDR_POS    RBIT(23)    /* ( VCtrl ) */
    #define GPVNDCTL_VENDORCTRL_ADDR_MSK    ( 0xff << GPVNDCTL_VENDORCTRL_ADDR_POS ) 
    #define GPVNDCTL_ADDR_REG_POS           RBIT(15)    /* ( RegAddr ) */
    #define GPVNDCTL_ADDR_REG_MSK           ( 0x3f << GPVNDCTL_ADDR_REG_POS )
    #define GPVNDCTL_WRITE                  ( 1 << RBIT(9) )    /* ( RegWr ) */
    #define GPVNDCTL_NEW_REQ                ( 1 << RBIT(6) )    /* ( NewRegReq ) */
    #define GPVNDCTL_VSTATUS_BUSY           ( 1 << RBIT(5) )    /* ( VStsBsy ) */
    #define GPVNDCTL_VSTATUS_DONE           ( 1 << RBIT(4) )    /* ( VStsDone ) */
    #define GPVNDCTL_ULPI_DISABLE           ( 1 << RBIT(0) )    /* ( DisUlpiDrvr ) */

#define DWCOTG_GSNPSID                  0x40

#define DWCOTG_GHWCFG1                  0x44
#define DWCOTG_GHWCFG2                  0x48
#define DWCOTG_GHWCFG3                  0x4C
#define DWCOTG_GHWCFG4                  0x50

/* host register definitions */

#define DWCOTG_HPTXFSIZ                 0x100
    #define HPTXFSIZ_START_ADDR_MSK         0xffff          /* ( PTxFStAddr ) */
    #define HPTXFSIZ_FDEPTH_POS             RBIT(15)        /* ( PTxFsize ) */
    #define HPTXFSIZ_FDEPTH_MSK             ( 0xffff << HPTXFSIZ_FDEPTH_POS )        

#if 0
#define DWCOTG_HCFG                     0x400
    #define HCFG_PHYCLK_MSK                 ( 3 << RBIT(0) ) /* ( FSLSPclkSel ) */    
    #define HCFG_PHYCLK_30_60               ( 0 << RBIT(0) )    
    #define HCFG_PHYCLK_48                  ( 1 << RBIT(0) )
    #define HCFG_FORCE_FULL_SPEED           ( 1 << RBIT(1) ) /* ( FSLSSupp ) */
#else
#define DWCOTG_HCFG                     0x400
    #define HCFG_PHYCLK_MSK                 ( 3 << (0) ) /* ( FSLSPclkSel ) */    
    #define HCFG_PHYCLK_30_60               ( 0 << (0) )    
    #define HCFG_PHYCLK_48                  ( 1 << (0) )
    #define HCFG_FORCE_FULL_SPEED           ( 1 << RBIT(1) ) /* ( FSLSSupp ) */
#endif

#define DWCOTG_HFIR                     0x404
    #define HFIR_FRAME_INT_MSK              0xffff            /* ( FrInt ) */
    #define HFIR_RLDCTRL                    ( 1 << RBIT(15) ) /* ( HFIRRldCtrl) */
    
#define DWCOTG_HFNUM                    0x408
    #define HFNUM_FRAME_NUM_MSK             0x3fff          /* ( FrNum ) */
    #define HFNUM_FRAME_TIME_REM_POS        RBIT(15)        /* ( FrRem ) */
    #define HFNUM_FRAME_TIME_REM_MSK        ( 0xffff << HFNUM_FRAME_TIME_REM_POS )

#define DWCOTG_HPTXSTS                  0x410
    #define HPTXSTS_FIFO_SPACE_AVAIL_MSK    0xffff          /* ( PTxFSpcAvail ) */
    #define HPTXSTS_TX_Q_SPACE_AVAIL_POS    RBIT(15)        /* ( PTxQSpcAvail ) */
    #define HPTXSTS_TX_Q_SPACE_AVAIL_MSK    ( 0xff << HPTXSTS_TX_Q_SPACE_AVAIL_POS )
    #define HPTXSTS_TOP_Q_POS               RBIT(7)        /* ( PTxQTop ) */
    #define HPTXSTS_TOP_Q_MSK               ( 0xff << HPTXSTS_TOP_TX_Q_POS )

#define DWCOTG_HAINT                    0x414
#define DWCOTG_HAINTMSK                 0x418

#define DWCOTG_HPRT                     0x440
    #define HPRT_CONNECT_STS                ( 1 << RBIT(31) )   /* ( PrtConnSts ) */
    #define HPRT_CONNECT_DETECT             ( 1 << RBIT(30) )   /* ( PrtConnDet ) */
    #define HPRT_PORT_ENABLE                ( 1 << RBIT(29) )   /* ( PrtEna ) */
    #define HPRT_PORT_ENABLE_CHANGE         ( 1 << RBIT(28) )   /* ( PrtEnChng ) */
    #define HPRT_OVERCURRENT                ( 1 << RBIT(27) )   /* ( PrtOvrCurrAct ) */
    #define HPRT_OVERCURRENT_CHG            ( 1 << RBIT(26) )   /* ( PrtOvrCurrChng ) */
    #define HPRT_RESUME                     ( 1 << RBIT(25) )   /* ( PrtRes ) */
    #define HPRT_SUSPEND                    ( 1 << RBIT(24) )   /* ( PrtSusp ) */
    #define HPRT_RESET                      ( 1 << RBIT(23) )   /* ( PrtRst ) */
    #define HPRT_POWER                      ( 1 << RBIT(19) )   /* ( PrtPwr ) */
    #define HPRT_SPEED_MSK                  ( 3 << RBIT(14) )   /* ( PrtSpd ) */
    #define HPRT_SPEED_HIGH                 ( 0 << RBIT(14) )   /* ( PrtSpd ) */
    #define HPRT_SPEED_FULL                 ( 1 << RBIT(14) )   /* ( PrtSpd ) */
    #define HPRT_SPEED_LOW                  ( 2 << RBIT(14) )   /* ( PrtSpd ) */
    
#define DWCOTG_HCCHAR(n)                ( ((n)*0x20) + 0x500 )
    #define HCCHAR_MPS_MSK                  0x7ff               /* ( MPS ) */
    #define HCCHAR_EPNUM_POS                RBIT(20)            /* ( EPNum ) */
    #define HCCHAR_EPNUM_MSK                ( 0xf << HCCHAR_EPNUM_POS ) 
    #define HCCHAR_EPDIR_MSK                 ( 1 << RBIT(16) )   /* ( EPDir ) */
    #define HCCHAR_EPDIR_IN                 ( 1 << RBIT(16) )   /* ( EPDir ) */
    #define HCCHAR_EPDIR_OUT                ( 0 << RBIT(16) )   /* ( EPDir ) */    
    #define HCCHAR_LOWSPEED                 ( 1 << RBIT(14) )   /* ( LSpdDev ) */
    #define HCCHAR_EPTYPE_POS               RBIT(13)            /* ( LSpdDev ) */
    #define HCCHAR_EPTYPE_MSK               ( 3 << HCCHAR_EPTYPE_POS )  
    #define HCCHAR_EPTYPE_CONTROL           ( 0 << HCCHAR_EPTYPE_POS )  
    #define HCCHAR_EPTYPE_ISOCH             ( 1 << HCCHAR_EPTYPE_POS )  
    #define HCCHAR_EPTYPE_BULK              ( 2 << HCCHAR_EPTYPE_POS )  
    #define HCCHAR_EPTYPE_INT               ( 3 << HCCHAR_EPTYPE_POS )  
    #define HCCHAR_MCEC_POS                 RBIT(11)            
    #define HCCHAR_MCEC_MSK                 ( 3 << HCCHAR_MCEC_POS )
    #define HCCHAR_MCEC_1XFER               ( 1 << HCCHAR_MCEC_POS )
    #define HCCHAR_MCEC_2XFER               ( 2 << HCCHAR_MCEC_POS )
    #define HCCHAR_MCEC_3XFER               ( 3 << HCCHAR_MCEC_POS )
    #define HCCHAR_DADDR_POS                RBIT(9)            
    #define HCCHAR_DADDR_MSK                ( 0x7f <<  RBIT(9) )
    #define HCCHAR_ODD_FRAME                ( 1 << RBIT(2) ) /* ( OddFrm ) */
    #define HCCHAR_CHAN_DISABLE             ( 1 << RBIT(1) ) /* ( ChDis ) */
    #define HCCHAR_CHAN_ENABLE              ( 1 << RBIT(0) ) /* ( ChEna ) */

#define DWCOTG_HCSPLT(n)                ( ((n)*0x20) + 0x504 )
    #define HCSPLT_PORT_ADDR_MSK            0x7f                /* ( PrtAddr ) */
    #define HCSPLT_HUB_ADDR_POS             RBIT(24)            /* ( HubAddr ) */
    #define HCSPLT_HUB_ADDR_MSK             ( 0x7f << HCSPLT_HUB_ADDR_POS )
    #define HCSPLT_XACTPOSITION_POS         RBIT(17)            /* ( XactPos ) */
    #define HCSPLT_XACTPOSITION_MSK         ( 3 << RBIT(17) )
    #define HCSPLT_XACTPOSITION_ALL         ( 3 << RBIT(17) )
    #define HCSPLT_XACTPOSITION_BEG         ( 2 << RBIT(17) )
    #define HCSPLT_XACTPOSITION_MID         ( 0 << RBIT(17) )
    #define HCSPLT_XACTPOSITION_END         ( 1 << RBIT(17) )
    #define HCSPLT_COMPLETE                 ( 1 << RBIT(15) )   /* ( CompSplt ) */
    #define HCSPLT_DOSPLIT                  ( 1 << RBIT(0) )    /* ( SpltEna ) */

#define DWCOTG_HCINT(n)                 ( ((n)*0x20) + 0x508 )
#define DWCOTG_HCINTMSK(n)              ( ((n)*0x20) + 0x50c )
    #define HCINT_XFER_COMPLETE             ( 1 << RBIT(31) )   /* ( XferCompl ) */
    #define HCINT_CHAN_HALTED               ( 1 << RBIT(30) )   /* ( ChHltd ) */
    #define HCINT_AHB_ERROR                 ( 1 << RBIT(29) )   /* ( AHBErr ) */
    #define HCINT_STALL                     ( 1 << RBIT(28) )   /* ( STALL ) */
    #define HCINT_NAK                       ( 1 << RBIT(27) )   /* ( NAK ) */
    #define HCINT_ACK                       ( 1 << RBIT(26) )   /* ( ACK ) */
    #define HCINT_NYET                      ( 1 << RBIT(25) )   /* ( NYET ) */
    #define HCINT_XACT_ERROR                ( 1 << RBIT(24) )   /* ( XactErr ) */    
    #define HCINT_BABBLE                    ( 1 << RBIT(23) )   /* ( BblErr ) */
    #define HCINT_OVERRUN                   ( 1 << RBIT(22) )   /* ( FrmOvrun ) */
    #define HCINT_TOGGLE_ERR                ( 1 << RBIT(21) )   /* ( DataTglErr ) */

    #define HCINT_INTEREST                   ( HCINT_CHAN_HALTED    |   \
                                               0                        )



#define DWCOTG_HCTSIZ(n)                    ( ((n)*0x20) + 0x510 )
    #define HCTSIZ_XFER_SIZE_MSK            0x7ffff                  /* (XferSize) */
    #define HCTSIZ_PKT_CNT_POS              RBIT(12)                /* ( PktCnt ) */
    #define HCTSIZ_PKT_CNT_MSK              ( 0x3ff << HCTSIZ_PKT_CNT_POS )
    #define HCTSIZ_PID_POS                  RBIT(2)                 /* ( PID ) */
    #define HCTSIZ_PID_MSK                  ( 3 << HCTSIZ_PID_POS )
    #define HCTSIZ_PID_DATA0                ( 0 << HCTSIZ_PID_POS )
    #define HCTSIZ_PID_DATA2                ( 1 << HCTSIZ_PID_POS )
    #define HCTSIZ_PID_DATA1                ( 2 << HCTSIZ_PID_POS )
    #define HCTSIZ_PID_MDATA                ( 3 << HCTSIZ_PID_POS )
    #define HCTSIZ_PID_SETUP                ( 3 << HCTSIZ_PID_POS )
    #define HCTSIZ_DOPING                   ( 1 << RBIT(0) )        /* ( DoPng) */
    
#define DWCOTG_HCDMA(n)                 ( ((n)*0x20) + 0x514 )

/* clock gating register definitions */

#define DWCOTG_PCGCTL                   0xe00



/* data structures */


typedef struct hctrl_ctx        hctrl_t;
typedef struct ep_ctx           epctx_t;
typedef struct xelem            xelem_t;

 
struct xelem {
    // transfer parameters
    st_URB_TRANSFER             *urb;
    st_ENDPOINT_DESCRIPTOR      *edesc;
    uint8_t                     *buffer;
    uint32_t                    length; 
    uint32_t                    flags;
    uint32_t                    errcnt;
    uint32_t                    npkt;
    
};


#define EPFLAG_USED                     ( 1 << 0 )
#define EPFLAG_ACTIVE_XFER              ( 1 << 1 )
#define EPFLAG_XFER_ABORT               ( 1 << 2 )

#define SCHED_TIMER                     (_PULSE_CODE_MINAVAIL + 3)
#define SCHED_CSPLIT                    (_PULSE_CODE_MINAVAIL + 4)
#define SCHED_DOSTART                   (_PULSE_CODE_MINAVAIL + 5)

struct ep_ctx {   
    uint32_t                            flags;
    uint32_t                            chnum;
    uint32_t                            chmask;
    psched_timeslot_t                   *timeslot;
    uint32_t                            sendzlp;
    uint32_t                            ping_needed;
	uint32_t                            do_split;
	uint32_t                            comp_split;
	int32_t                             pid;
	uint32_t                            interval;
	uint32_t                            start_once;
	uint32_t                            start_frame;
	uint32_t                            active_xfer;
    xelem_t                             *xelem;
};



#define HCFLAG_PORT0_CONNECTED        ( 1 << 0 )

struct hctrl_ctx {
    uint32_t                            flags;
    uint32_t                            verbosity;
    uint8_t                             *IoBase;
    pthread_mutex_t                     mutex;
    epctx_t                             ep_arr[DWCOTG_N_CHAN_MAX];
    fbma_ctx_t                          *xelem_pool;
    psched_t                            *scheduler;
    int                                 scheduler_ep_cnt;
    int                                 scheduler_tid;
    int                                 scheduler_priority;
    int                                 nchan;
    int                                 chid;
    int                                 coid;
    int                                 timerid;
};


/* Endian */

#define DWCOTG_REGISTERS_LITTLE_ENDIAN

#ifdef DWCOTG_REGISTERS_LITTLE_ENDIAN
    #define ENDIAN32(n) ENDIAN_LE32(n)
    #define ENDIAN16(n) ENDIAN_LE16(n)
#endif


/* Regsiters Access Macros */


#define HW_Read32(  base, off ) \
     ( ENDIAN32( *(( volatile uint32_t *)( base + off )) ) )

#define HW_Write32( base, off,  v ) \
    *(( volatile uint32_t *)( base + off ) ) =  ENDIAN32( v )


#define HW_Write32Or( base, off, v ) \
    HW_Write32( base, off,  HW_Read32(base,off) | v )
        
#define HW_Write32And( base, off, v ) \
    HW_Write32( base, off,  HW_Read32(base,off) & v )


#define HW_Read16(  base, off ) \
     ( ENDIAN16( *(( volatile uint16_t *)( base + off )) ) )

#define HW_Write16( base, off,  v ) \
    *(( volatile uint16_t *)( base + off ) ) =  ENDIAN16( v ) 

#define HW_Read8(  base, off ) \
     ( *( base + off ) )

#define HW_Write8( base, off,  v ) \
    *( base + off ) =  v 




// predefine functions for contoller and pipe methods

int             dwcotg_init(void *dll_hdl, dispatch_t *dpp, io_usb_self_t *iousb, char *options);
int             dwcotg_shutdown(void *dll_hdl);
_uint32         dwcotg_controller_init( st_USB_Hc *hc, _uint32 flags, char *args );
_uint32         dwcotg_set_bus_state( st_USB_Hc *Hc, _uint32 bus_state );
_uint32         dwcotg_controller_shutdown( st_USB_Hc *hc );
_uint32         dwcotg_controller_interrupt( st_USB_Hc *hc );
_uint32         dwcotg_set_port_feature( st_USB_Hc *hc, _uint32 port, _uint32 feature );
_uint32         dwcotg_clear_port_feature( st_USB_Hc *hc, _uint32 port, _uint32 feature );
_uint32         dwcotg_enable_root_hub( st_USB_Hc *hc, _uint32 port );
_uint32         dwcotg_reset_root_hub( st_USB_Hc *hc, _uint32 port );
_uint32         dwcotg_check_port_status( st_USB_Hc *hc, _uint32 *change_bitmap);
_uint32         dwcotg_check_device_connected( st_USB_Hc *hc, _uint32 port );
_uint32         dwcotg_get_root_device_speed( st_USB_Hc *hc, _uint32 port );
_uint32         dwcotg_get_timer_from_controller( st_USB_Hc *hc );

_uint32         dwcotg_ctrl_endpoint_enable( st_USB_Hc *hc,st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_ctrl_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_ctrl_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,_uint8 *buffer, _uint32 length, _uint32 flags );
_uint32         dwcotg_ctrl_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc );

_uint32         dwcotg_int_endpoint_enable( st_USB_Hc *hc,st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_int_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_int_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,_uint8 *buffer, _uint32 length, _uint32 flags );
_uint32         dwcotg_int_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc );

_uint32         dwcotg_bulk_endpoint_enable( st_USB_Hc *hc,st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_bulk_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_bulk_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,_uint8 *buffer, _uint32 length, _uint32 flags );
_uint32         dwcotg_bulk_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc );

_uint32         dwcotg_isoch_endpoint_enable( st_USB_Hc *hc,st_DEVICE_DESCRIPTOR *ddesc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_isoch_endpoint_disable( st_USB_Hc *hc, st_ENDPOINT_DESCRIPTOR *edesc );
_uint32         dwcotg_isoch_transfer( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc,_uint8 *buffer, _uint32 length, _uint32 flags );
_uint32         dwcotg_isoch_transfer_abort( st_USB_Hc *hc, st_URB_TRANSFER *urb, st_ENDPOINT_DESCRIPTOR *edesc );


#endif


