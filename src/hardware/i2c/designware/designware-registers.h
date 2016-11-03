/* designware-registers.h
 *
 *  Created on: 14.10.2016
 *      Author: SWD Embedded Systems Ltd. */


#ifndef _DESIGNWARE_REGISTERS_
#define _DESIGNWARE_REGISTERS_


typedef struct i2c_regs
{
    /* i2c control register */
    uint32_t            ic_con;                             /* 0x00 */
    #define IC_CON_SLAVE_DISABLE                (1 << 6)
    #define IC_CON_RESTART_EN                   (1 << 5)
    #define IC_CON_10BITADDRMASTER              (1 << 4)
    #define IC_CON_10BITADDR_SLAVE              (1 << 3)
    #define IC_CON_SPEED_MSK                    (3 << 1)
    #define IC_CON_SPEED_STD                        (1 << 1)
    #define IC_CON_SPEED_FAST                       (2 << 1)
    #define IC_CON_SPEED_HIGH                       (3 << 1)
    #define IC_CON_MASTER_MODE                      (1 << 0)

    uint32_t            ic_tar;                             /* 0x04 */
    uint32_t            ic_sar;                             /* 0x08 */
    uint32_t            ic_hs_maddr;                        /* 0x0C */

    /* i2c data buffer and command register */
    uint32_t            ic_cmd_data;                        /* 0x10 */
    #define IC_STOP                             0x0200
    #define IC_CMD                              0x0100

    /* High and low times in different speed modes (in ns) */
    uint32_t            ic_ss_scl_hcnt;                     /* 0x14 */
    #define MIN_SS_SCL_HIGHTIME                 4000
    uint32_t            ic_ss_scl_lcnt;                     /* 0x18 */
    #define MIN_SS_SCL_LOWTIME                  4700
    uint32_t            ic_fs_scl_hcnt;                     /* 0x1C */
    #define MIN_FS_SCL_HIGHTIME                 600
    uint32_t            ic_fs_scl_lcnt;                     /* 0x20 */
    #define MIN_FS_SCL_LOWTIME                  1300
    uint32_t            ic_hs_scl_hcnt;                     /* 0x24 */
    #define MIN_HS_SCL_HIGHTIME                 60
    uint32_t            ic_hs_scl_lcnt;                     /* 0x28 */
    #define MIN_HS_SCL_LOWTIME                  160

    /* i2c interrupt status register */
    uint32_t            ic_intr_stat;                       /* 0x2C */
    uint32_t            ic_intr_mask;                       /* 0x30 */
    #define IC_GEN_CALL                         0x0800
    #define IC_START_DET                        0x0400
    #define IC_STOP_DET                         0x0200
    #define IC_ACTIVITY                         0x0100
    #define IC_RX_DONE                          0x0080
    #define IC_TX_ABRT                          0x0040
    #define IC_RD_REQ                           0x0020
    #define IC_TX_EMPTY                         0x0010
    #define IC_TX_OVER                          0x0008
    #define IC_RX_FULL                          0x0004
    #define IC_RX_OVER                          0x0002
    #define IC_RX_UNDER                         0x0001

    uint32_t            ic_raw_intr_stat;                   /* 0x34 */

    /* fifo threshold register */
    uint32_t            ic_rx_tl;                           /* 0x38 */
    uint32_t            ic_tx_tl;                           /* 0x3C */
    #define IC_TL0                              0x00
    #define IC_TL1                              0x01
    #define IC_TL2                              0x02
    #define IC_TL3                              0x03
    #define IC_TL4                              0x04
    #define IC_TL5                              0x05
    #define IC_TL6                              0x06
    #define IC_TL7                              0x07
    #define IC_RX_TL                            IC_TL0
    #define IC_TX_TL                            IC_TL0

    uint32_t            ic_clr_intr;                        /* 0x40 */
    uint32_t            ic_clr_rx_under;                    /* 0x44 */
    uint32_t            ic_clr_rx_over;                     /* 0x48 */
    uint32_t            ic_clr_tx_over;                     /* 0x4C */
    uint32_t            ic_clr_rd_req;                      /* 0x50 */
    uint32_t            ic_clr_tx_abrt;                     /* 0x54 */
    uint32_t            ic_clr_rx_done;                     /* 0x58 */
    uint32_t            ic_clr_activity;                    /* 0x5C */
    uint32_t            ic_clr_stop_det;                    /* 0x60 */
    uint32_t            ic_clr_start_det;                   /* 0x64 */
    uint32_t            ic_clr_gen_call;                    /* 0x68 */

    /* i2c enable register */
    uint32_t            ic_enable;                          /* 0x6C */
    #define             IC_ENABLE               (1 << 0)


    /* i2c status register */
    uint32_t            ic_status;                          /* 0x70 */
    #define IC_STATUS_SA                        0x0040
    #define IC_STATUS_MA                        0x0020
    #define IC_STATUS_RFF                       0x0010
    #define IC_STATUS_RFNE                      0x0008
    #define IC_STATUS_TFE                       0x0004
    #define IC_STATUS_TFNF                      0x0002
    #define IC_STATUS_ACT                       0x0001

    uint32_t            ic_txflr;                           /* 0x74 */
    uint32_t            ic_rxflr;                           /* 0x78 */
    uint32_t            ic_sda_hold;                        /* 0x7C */
    uint32_t            ic_tx_abrt_source;                  /* 0x80 */
    uint32_t            ic_slv_data_nack_only;              /* 0x84 */
    uint32_t            ic_dma_cr;                          /* 0x88 */
    uint32_t            ic_dma_tdlr;                        /* 0x8C */
    uint32_t            ic_dma_rdlr;                        /* 0x90 */
    uint32_t            ic_sda_setup;                       /* 0x94 */
    uint32_t            ic_ack_general_call;                /* 0x98 */

    uint32_t            ic_enable_status;                   /* 0x9C */
    #define IC_EN                               0x0001

    uint32_t            ic_fs_spklen;                       /* 0xA0 */
    uint32_t            ic_hs_spklen;                       /* 0xA4 */
    uint32_t            reserved[19];

    /* Synopsys registers */
    uint32_t            ic_comp_param_1;                    /* 0xF4 */
    uint32_t            ic_comp_version;                    /* 0xF8 */
    #define IC_SDA_HOLD_MIN_VERS                0x3131312A
    uint32_t            ic_comp_type;                       /* 0xFC */
    #define IC_COMP_TYPE_VALUE                  0x44570140
} designware_i2c_regs_t;


#define SWAP32( x )                             ((((x) & (uint32_t)0x000000ffUL) << 24) | \
                                                 (((x) & (uint32_t)0x0000ff00UL) <<  8) | \
                                                 (((x) & (uint32_t)0x00ff0000UL) >>  8) | \
                                                 (((x) & (uint32_t)0xff000000UL) >> 24))
#define READ(     register )                    (dev->accessor_flags & ACCESS_SWAP ? SWAP32( dev->registers->register ) : dev->registers->register)
#define WRITE(    register, val )               {if ( dev->accessor_flags & ACCESS_SWAP ) dev->registers->register = SWAP32( (val) ); else dev->registers->register = (val);}


#endif  /* _DESIGNWARE_REGISTERS_ */