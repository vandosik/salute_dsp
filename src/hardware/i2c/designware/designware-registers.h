/* designware-registers.h
 *
 *  Created on: 14.10.2016
 *      Author: SWD Embedded Systems Ltd. */


#ifndef _DESIGNWARE_REGISTERS_
#define _DESIGNWARE_REGISTERS_


typedef struct i2c_regs
{
    /* i2c control register */
    uint32_t            ic_con;
    #define IC_CON_SD                           (1 << 6)
    #define IC_CON_RE                           (1 << 5)
    #define IC_CON_10BITADDRMASTER              (1 << 4)
    #define IC_CON_10BITADDR_SLAVE              (1 << 3)
    #define IC_CON_SPD_MSK                      (3 << 1)
    #define IC_CON_SPD_SS                           (1 << 0)
    #define IC_CON_SPD_FS                           (2 << 1)
    #define IC_CON_SPD_HS                           (3 << 1)
    #define IC_CON_MM                           (1 << 0)

    uint32_t            ic_tar;
    uint32_t            ic_sar;
    uint32_t            ic_hs_maddr;

    /* i2c data buffer and command register */
    uint32_t            ic_cmd_data;
    #define IC_STOP                             0x0200
    #define IC_CMD                              0x0100

    /* High and low times in different speed modes (in ns) */
    uint32_t            ic_ss_scl_hcnt;
    #define MIN_SS_SCL_HIGHTIME                 4000
    uint32_t            ic_ss_scl_lcnt;
    #define MIN_SS_SCL_LOWTIME                  4700
    uint32_t            ic_fs_scl_hcnt;
    #define MIN_FS_SCL_HIGHTIME                 600
    uint32_t            ic_fs_scl_lcnt;
    #define MIN_FS_SCL_LOWTIME                  1300
    uint32_t            ic_hs_scl_hcnt;
    #define MIN_HS_SCL_HIGHTIME                 60
    uint32_t            ic_hs_scl_lcnt;
    #define MIN_HS_SCL_LOWTIME                  160

    /* i2c interrupt status register */
    uint32_t            ic_intr_stat;
    uint32_t            ic_intr_mask;
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

    uint32_t            ic_raw_intr_stat;

    /* fifo threshold register */
    uint32_t            ic_rx_tl;
    uint32_t            ic_tx_tl;
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

    uint32_t            ic_clr_intr;
    uint32_t            ic_clr_rx_under;
    uint32_t            ic_clr_rx_over;
    uint32_t            ic_clr_tx_over;
    uint32_t            ic_clr_rd_req;
    uint32_t            ic_clr_tx_abrt;
    uint32_t            ic_clr_rx_done;
    uint32_t            ic_clr_activity;
    uint32_t            ic_clr_stop_det;
    uint32_t            ic_clr_start_det;
    uint32_t            ic_clr_gen_call;

    /* i2c enable register */
    uint32_t            ic_enable;
    #define             IC_ENABLE_0B            (1 << 0)


    /* i2c status register */
    uint32_t            ic_status;
    #define IC_STATUS_SA                        0x0040
    #define IC_STATUS_MA                        0x0020
    #define IC_STATUS_RFF                       0x0010
    #define IC_STATUS_RFNE                      0x0008
    #define IC_STATUS_TFE                       0x0004
    #define IC_STATUS_TFNF                      0x0002
    #define IC_STATUS_ACT                       0x0001

    uint32_t            ic_txflr;
    uint32_t            ix_rxflr;
    uint32_t            reserved_1;
    uint32_t            ic_tx_abrt_source;
} designware_i2c_regs_t;


#define REGISTER( register )                    dev->registers->register
#define READ(     register )                    (dev->registers->register)
#define WRITE(    register, val )               (dev->registers->register = val)


#endif  /* _DESIGNWARE_REGISTERS_ */