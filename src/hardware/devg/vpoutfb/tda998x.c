/*************************************************/
/*  NXP TDA998x I2C HDMI transmitter interface   */
/*************************************************/


/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   REGISTERS                   */
/*************************************************/


/* Registers access */
#define REGISTER( page, addr )          (((page) << 8) | (addr))
#define REGISTER_ADDRESS( reg )         ((reg) & 0xff)
#define REGISTER_PAGE( reg )            (((reg) >> 8) & 0xff)

/* Registers pages */
#define REGISTER_PAGE_HDMI_CTRL         (0x00)
#define REGISTER_PAGE_HDMI_PPL          (0x02)
#define REGISTER_PAGE_HDMI_EDID         (0x09)
#define REGISTER_PAGE_HDMI_INFO         (0x10)
#define REGISTER_PAGE_HDMI_AUDIO        (0x11)
#define REGISTER_PAGE_HDMI_HDCP_OTP     (0x12)
#define REGISTER_PAGE_HDMI_GAMUT        (0x13)

/* Page setup register */
#define REGISTER_CURPAGE                (0xff)                                              /* W */

/* General Control (page: 00h) */
#define REGISTER_VERSION_LSB            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x00 )           /* R */
#define REGISTER_MAIN_CNTRL0            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x01 )           /* RW */
#define     REGISTER_MAIN_CNTRL0_SCALER     (1 << 7)
#define     REGISTER_MAIN_CNTRL0_CEHS       (1 << 4)
#define     REGISTER_MAIN_CNTRL0_CECS       (1 << 3)
#define     REGISTER_MAIN_CNTRL0_DEHS       (1 << 2)
#define     REGISTER_MAIN_CNTRL0_DECS       (1 << 1)
#define     REGISTER_MAIN_CNTRL0_SR         (1 << 0)
#define REGISTER_VERSION_MSB            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x02 )           /* R */
#define REGISTER_SOFTRESET              REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x0a )           /* W */
#define     REGISTER_SOFTRESET_I2C_MASTER   (1 << 1)
#define     REGISTER_SOFTRESET_AUDIO        (1 << 0)
#define REGISTER_DDC_DISABLE            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x0b )           /* RW */
#define REGISTER_CCLK_ON                REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x0c )           /* RW */
#define REGISTER_I2C_MASTER             REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x0d )           /* RW */
#define     REGISTER_I2C_MASTER_APP_STRT_LAT (1 << 2)
#define     REGISTER_I2C_MASTER_DIS_FILT    (1 << 1)
#define     REGISTER_I2C_MASTER_DIS_MM      (1 << 0)
#define REGISTER_FEAT_POWERDOWN         REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x0e )           /* RW */
#define     REGISTER_FEAT_POWERDOWN_SPDIF   (1 << 3)
#define REGISTER_INT_FLAGS_0            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x0f )           /* RW */
#define REGISTER_INT_FLAGS_1            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x10 )           /* RW */
#define REGISTER_INT_FLAGS_2            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x11 )           /* RW */
#define     REGISTER_INT_FLAGS_2_EDID_BLK_RD (1 << 1)
#define REGISTER_ENA_ACLK               REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x16 )           /* RW */
#define REGISTER_ENA_VP_0               REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x18 )           /* RW */
#define REGISTER_ENA_VP_1               REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x19 )           /* RW */
#define REGISTER_ENA_VP_2               REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x1a )           /* RW */
#define REGISTER_ENA_AP                 REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x1e )           /* RW */
#define REGISTER_VIP_CNTRL_0            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x20 )           /* W */
#define     REGISTER_VIP_CNTRL_0_MIRR_A     (1 << 7)
#define     REGISTER_VIP_CNTRL_0_SWAP_A( x )(((x) & 7) << 4)
#define     REGISTER_VIP_CNTRL_0_MIRR_B     (1 << 3)
#define     REGISTER_VIP_CNTRL_0_SWAP_B( x )(((x) & 7) << 0)
#define REGISTER_VIP_CNTRL_1            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x21 )           /* W */
#define     REGISTER_VIP_CNTRL_1_MIRR_C     (1 << 7)
#define     REGISTER_VIP_CNTRL_1_SWAP_C( x )(((x) & 7) << 4)
#define     REGISTER_VIP_CNTRL_1_MIRR_D     (1 << 3)
#define     REGISTER_VIP_CNTRL_1_SWAP_D( x )(((x) & 7) << 0)
#define REGISTER_VIP_CNTRL_2            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x22 )           /* W */
#define     REGISTER_VIP_CNTRL_2_MIRR_E     (1 << 7)
#define     REGISTER_VIP_CNTRL_2_SWAP_E( x )(((x) & 7) << 4)
#define     REGISTER_VIP_CNTRL_2_MIRR_F     (1 << 3)
#define     REGISTER_VIP_CNTRL_2_SWAP_F( x )(((x) & 7) << 0)
#define REGISTER_VIP_CNTRL_3            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x23 )           /* W */
#define     REGISTER_VIP_CNTRL_3_EDGE       (1 << 7)
#define     REGISTER_VIP_CNTRL_3_DE_INT     (1 << 6)
#define     REGISTER_VIP_CNTRL_3_SYNC_HS    (1 << 5)
#define     REGISTER_VIP_CNTRL_3_SYNC_DE    (1 << 4)
#define     REGISTER_VIP_CNTRL_3_EMB        (1 << 3)
#define     REGISTER_VIP_CNTRL_3_V_TGL      (1 << 2)
#define     REGISTER_VIP_CNTRL_3_H_TGL      (1 << 1)
#define     REGISTER_VIP_CNTRL_3_X_TGL      (1 << 0)
#define REGISTER_VIP_CNTRL_4            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x24 )           /* W */
#define     REGISTER_VIP_CNTRL_4_TST_PAT    (1 << 7)
#define     REGISTER_VIP_CNTRL_4_TST_656    (1 << 6)
#define     REGISTER_VIP_CNTRL_4_656_ALT    (1 << 5)
#define     REGISTER_VIP_CNTRL_4_CCIR656    (1 << 4)
#define     REGISTER_VIP_CNTRL_4_BLANKIT( x )(((x) & 3) << 2)
#define     REGISTER_VIP_CNTRL_4_BLC( x )   (((x) & 3) << 0)
#define REGISTER_VIP_CNTRL_5            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x25 )           /* W */
#define     REGISTER_VIP_CNTRL_5_SP_CNT( x )(((x) & 3) << 1)
#define     REGISTER_VIP_CNTRL_5_CKCASE     (1 << 0)
#define REGISTER_MUX_AP                 REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x26 )           /* RW */
#define     REGISTER_MUX_AP_SELECT_I2S      (0x64)
#define     REGISTER_MUX_AP_SELECT_SPDIF    (0x40)
#define REGISTER_MUX_VP_VIP_OUT         REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x27 )           /* RW */
#define REGISTER_MAT_CONTRL             REGISTER( REGISTER_PAGE_HDMI_CTRL, 0x80 )           /* W */
#define     REGISTER_MAT_CONTRL_MAT_BP      (1 << 2)
#define     REGISTER_MAT_CONTRL_MAT_SC( x ) (((x) & 3) << 0)
#define REGISTER_VIDFORMAT              REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa0 )           /* W */
#define REGISTER_REFPIX_MSB             REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa1 )           /* W */
#define REGISTER_REFPIX_LSB             REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa2 )           /* W */
#define REGISTER_REFLINE_MSB            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa3 )           /* W */
#define REGISTER_REFLINE_LSB            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa4 )           /* W */
#define REGISTER_NPIX_MSB               REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa5 )           /* W */
#define REGISTER_NPIX_LSB               REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa6 )           /* W */
#define REGISTER_NLINE_MSB              REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa7 )           /* W */
#define REGISTER_NLINE_LSB              REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa8 )           /* W */
#define REGISTER_VS_LINE_STRT_1_MSB     REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xa9 )           /* W */
#define REGISTER_VS_LINE_STRT_1_LSB     REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xaa )           /* W */
#define REGISTER_VS_PIX_STRT_1_MSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xab )           /* W */
#define REGISTER_VS_PIX_STRT_1_LSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xac )           /* W */
#define REGISTER_VS_LINE_END_1_MSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xad )           /* W */
#define REGISTER_VS_LINE_END_1_LSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xae )           /* W */
#define REGISTER_VS_PIX_END_1_MSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xaf )           /* W */
#define REGISTER_VS_PIX_END_1_LSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb0 )           /* W */
#define REGISTER_VS_LINE_STRT_2_MSB     REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb1 )           /* W */
#define REGISTER_VS_LINE_STRT_2_LSB     REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb2 )           /* W */
#define REGISTER_VS_PIX_STRT_2_MSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb3 )           /* W */
#define REGISTER_VS_PIX_STRT_2_LSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb4 )           /* W */
#define REGISTER_VS_LINE_END_2_MSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb5 )           /* W */
#define REGISTER_VS_LINE_END_2_LSB      REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb6 )           /* W */
#define REGISTER_VS_PIX_END_2_MSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb7 )           /* W */
#define REGISTER_VS_PIX_END_2_LSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb8 )           /* W */
#define REGISTER_HS_PIX_START_MSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xb9 )           /* W */
#define REGISTER_HS_PIX_START_LSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xba )           /* W */
#define REGISTER_HS_PIX_STOP_MSB        REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xbb )           /* W */
#define REGISTER_HS_PIX_STOP_LSB        REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xbc )           /* W */
#define REGISTER_VWIN_START_1_MSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xbd )           /* W */
#define REGISTER_VWIN_START_1_LSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xbe )           /* W */
#define REGISTER_VWIN_END_1_MSB         REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xbf )           /* W */
#define REGISTER_VWIN_END_1_LSB         REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc0 )           /* W */
#define REGISTER_VWIN_START_2_MSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc1 )           /* W */
#define REGISTER_VWIN_START_2_LSB       REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc2 )           /* W */
#define REGISTER_VWIN_END_2_MSB         REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc3 )           /* W */
#define REGISTER_VWIN_END_2_LSB         REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc4 )           /* W */
#define REGISTER_DE_START_MSB           REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc5 )           /* W */
#define REGISTER_DE_START_LSB           REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc6 )           /* W */
#define REGISTER_DE_STOP_MSB            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc7 )           /* W */
#define REGISTER_DE_STOP_LSB            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xc8 )           /* W */
#define REGISTER_TBG_CNTRL_0            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xca )           /* W */
#define     REGISTER_TBG_CNTRL_0_SYNC_ONCE  (1 << 7)
#define     REGISTER_TBG_CNTRL_0_SYNC_MTHD  (1 << 6)
#define     REGISTER_TBG_CNTRL_0_FRAME_DIS  (1 << 5)
#define     REGISTER_TBG_CNTRL_0_TOP_EXT    (1 << 3)
#define     REGISTER_TBG_CNTRL_0_DE_EXT     (1 << 2)
#define     REGISTER_TBG_CNTRL_0_TOP_SEL    (1 << 1)
#define     REGISTER_TBG_CNTRL_0_TOP_TGL    (1 << 0)
#define REGISTER_TBG_CNTRL_1            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xcb )           /* W */
#define     REGISTER_TBG_CNTRL_1_DWIN_DIS   (1 << 6)
#define     REGISTER_TBG_CNTRL_1_V_EXT      (1 << 5)
#define     REGISTER_TBG_CNTRL_1_H_EXT      (1 << 4)
#define     REGISTER_TBG_CNTRL_1_X_EXT      (1 << 3)
#define     REGISTER_TBG_CNTRL_1_TGL_EN     (1 << 2)
#define     REGISTER_TBG_CNTRL_1_V_TGL      (1 << 1)
#define     REGISTER_TBG_CNTRL_1_H_TGL      (1 << 0)
#define REGISTER_ENABLE_SPACE           REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xd6 )           /* W */
#define REGISTER_HVF_CNTRL_0            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xe4 )           /* W */
#define     REGISTER_HVF_CNTRL_0_SM         (1 << 7)
#define     REGISTER_HVF_CNTRL_0_RWB        (1 << 6)
#define     REGISTER_HVF_CNTRL_0_PREFIL( x )(((x) & 3) << 2)
#define     REGISTER_HVF_CNTRL_0_INTPOL( x )(((x) & 3) << 0)
#define REGISTER_HVF_CNTRL_1            REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xe5 )           /* W */
#define     REGISTER_HVF_CNTRL_1_SEMI_PLANAR (1 << 6)
#define     REGISTER_HVF_CNTRL_1_PAD( x )   (((x) & 3) << 4)
#define     REGISTER_HVF_CNTRL_1_VQR( x )   (((x) & 3) << 2)
#define     REGISTER_HVF_CNTRL_1_YUVBLK     (1 << 1)
#define     REGISTER_HVF_CNTRL_1_FOR        (1 << 0)
#define REGISTER_RPT_CNTRL              REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xf0 )           /* W */
#define REGISTER_I2S_FORMAT             REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xfc )           /* RW */
#define     REGISTER_I2S_FORMAT_VALUE( x )  (((x) & 3) << 0)
#define REGISTER_AIP_CLKSEL             REGISTER( REGISTER_PAGE_HDMI_CTRL, 0xfd )           /* W */
#define     REGISTER_AIP_CLKSEL_AIP         (1 << 3)
#define     REGISTER_AIP_CLKSEL_AIP_I2S         (1 << 3)
#define     REGISTER_AIP_CLKSEL_AIP_SPDIF       (0 << 3)
#define     REGISTER_AIP_CLKSEL_FS          (3 << 0)
#define     REGISTER_AIP_CLKSEL_FS_FS64SPDIF    (2 << 0)
#define     REGISTER_AIP_CLKSEL_FS_MCLK         (1 << 0)
#define     REGISTER_AIP_CLKSEL_FS_ACLK         (0 << 0)

/* PLL settings (page: 02h) */
#define REGISTER_PLL_SERIAL_1           REGISTER( REGISTER_PAGE_HDMI_PPL, 0x00 )            /* RW */
#define     REGISTER_PLL_SERIAL_1_SRL_MAN_IZ  (1 << 6)
#define     REGISTER_PLL_SERIAL_1_SRL_IZ( x ) (((x) & 3) << 1)
#define     REGISTER_PLL_SERIAL_1_SRL_FDN     (1 << 0)
#define REGISTER_PLL_SERIAL_2           REGISTER( REGISTER_PAGE_HDMI_PPL, 0x01 )            /* RW */
#define     REGISTER_PLL_SERIAL_2_SRL_PR( x )   (((x) & 0xf) << 4)
#define     REGISTER_PLL_SERIAL_2_SRL_NOSC( x ) ((x) << 0)
#define REGISTER_PLL_SERIAL_3           REGISTER( REGISTER_PAGE_HDMI_PPL, 0x02 )            /* RW */
#define     REGISTER_PLL_SERIAL_3_SRL_PXIN_SEL (1 << 4)
#define     REGISTER_PLL_SERIAL_3_SRL_DE    (1 << 2)
#define     REGISTER_PLL_SERIAL_3_SRL_CCIR  (1 << 0)
#define REGISTER_SERIALIZER             REGISTER( REGISTER_PAGE_HDMI_PPL, 0x03 )            /* RW */
#define REGISTER_BUFFER_OUT             REGISTER( REGISTER_PAGE_HDMI_PPL, 0x04 )            /* RW */
#define REGISTER_PLL_SCG1               REGISTER( REGISTER_PAGE_HDMI_PPL, 0x05 )            /* RW */
#define REGISTER_PLL_SCG2               REGISTER( REGISTER_PAGE_HDMI_PPL, 0x06 )            /* RW */
#define REGISTER_PLL_SCGN1              REGISTER( REGISTER_PAGE_HDMI_PPL, 0x07 )            /* RW */
#define REGISTER_PLL_SCGN2              REGISTER( REGISTER_PAGE_HDMI_PPL, 0x08 )            /* RW */
#define REGISTER_PLL_SCGR1              REGISTER( REGISTER_PAGE_HDMI_PPL, 0x09 )            /* RW */
#define REGISTER_PLL_SCGR2              REGISTER( REGISTER_PAGE_HDMI_PPL, 0x0a )            /* RW */
#define REGISTER_AUDIO_DIV              REGISTER( REGISTER_PAGE_HDMI_PPL, 0x0e )            /* RW */
#define     REGISTER_AUDIO_DIV_SERCLK_32    (5)
#define     REGISTER_AUDIO_DIV_SERCLK_16    (4)
#define     REGISTER_AUDIO_DIV_SERCLK_8     (3)
#define     REGISTER_AUDIO_DIV_SERCLK_4     (2)
#define     REGISTER_AUDIO_DIV_SERCLK_2     (1)
#define     REGISTER_AUDIO_DIV_SERCLK_1     (0)
#define REGISTER_SEL_CLK                REGISTER( REGISTER_PAGE_HDMI_PPL, 0x11 )            /* RW */
#define     REGISTER_SEL_CLK_ENA_SC_CLK     (1 << 3)
#define     REGISTER_SEL_CLK_SEL_VRF_CLK( x )(((x) & 3) << 1)
#define     REGISTER_SEL_CLK_SEL_CLK1       (1 << 0)
#define REGISTER_ANA_GENERAL            REGISTER( REGISTER_PAGE_HDMI_PPL, 0x12 )            /* RW */

/* EDID Control (page: 09h) */
#define REGISTER_EDID_DATA_0            REGISTER( REGISTER_PAGE_HDMI_EDID, 0x00 )           /* R */
#define REGISTER_EDID_CTRL              REGISTER( REGISTER_PAGE_HDMI_EDID, 0xfa )           /* RW */
#define REGISTER_DDC_ADDR               REGISTER( REGISTER_PAGE_HDMI_EDID, 0xfb )           /* RW */
#define REGISTER_DDC_OFFS               REGISTER( REGISTER_PAGE_HDMI_EDID, 0xfc )           /* RW */
#define REGISTER_DDC_SEGM_ADDR          REGISTER( REGISTER_PAGE_HDMI_EDID, 0xfd )           /* RW */
#define REGISTER_DDC_SEGM               REGISTER( REGISTER_PAGE_HDMI_EDID, 0xfe )           /* RW */

/* Information frames and packets (page: 10h) */
#define REGISTER_IF1_HB0                REGISTER( REGISTER_PAGE_HDMI_INFO, 0x20 )           /* RW */
#define REGISTER_IF2_HB0                REGISTER( REGISTER_PAGE_HDMI_INFO, 0x40 )           /* RW */
#define REGISTER_IF3_HB0                REGISTER( REGISTER_PAGE_HDMI_INFO, 0x60 )           /* RW */
#define REGISTER_IF4_HB0                REGISTER( REGISTER_PAGE_HDMI_INFO, 0x80 )           /* RW */
#define REGISTER_IF5_HB0                REGISTER( REGISTER_PAGE_HDMI_INFO, 0xa0 )           /* RW */

/* Audio settings and content info packets (page: 11h) */
#define REGISTER_AIP_CNTRL_0            REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x00 )          /* RW */
#define     REGISTER_AIP_CNTRL_0_RST_CTS    (1 << 6)
#define     REGISTER_AIP_CNTRL_0_ACR_MAN    (1 << 5)
#define     REGISTER_AIP_CNTRL_0_LAYOUT     (1 << 2)
#define     REGISTER_AIP_CNTRL_0_SWAP       (1 << 1)
#define     REGISTER_AIP_CNTRL_0_RST_FIFO   (1 << 0)
#define REGISTER_CA_I2S                 REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x01 )          /* RW */
#define     REGISTER_CA_I2S_HBR_CHSTAT      (1 << 6)
#define     REGISTER_CA_I2S_CA_I2S( x )     (((x) & 31) << 0)
#define REGISTER_LATENCY_RD             REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x04 )          /* RW */
#define REGISTER_ACR_CTS_0              REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x05 )          /* RW */
#define REGISTER_ACR_CTS_1              REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x06 )          /* RW */
#define REGISTER_ACR_CTS_2              REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x07 )          /* RW */
#define REGISTER_ACR_N_0                REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x08 )          /* RW */
#define REGISTER_ACR_N_1                REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x09 )          /* RW */
#define REGISTER_ACR_N_2                REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x0a )          /* RW */
#define REGISTER_CTS_N                  REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x0c )          /* RW */
#define     REGISTER_CTS_N_M( x )           (((x) & 3) << 4)
#define     REGISTER_CTS_N_K( x )           (((x) & 7) << 0)
#define REGISTER_ENC_CNTRL              REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x0d )          /* RW */
#define     REGISTER_ENC_CNTRL_CTL_CODE( x )(((x) & 3) << 2)
#define     REGISTER_ENC_CNTRL_RST_SEL      (1 << 1)
#define     REGISTER_ENC_CNTRL_RST_ENC      (1 << 0)
#define REGISTER_DIP_FLAGS              REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x0e )          /* RW */
#define     REGISTER_DIP_FLAGS_GC           (1 << 1)
#define     REGISTER_DIP_FLAGS_ACR          (1 << 0)
#define REGISTER_DIP_IF_FLAGS           REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x0f )          /* RW */
#define     REGISTER_DIP_IF_FLAGS_IF5       (1 << 5)
#define     REGISTER_DIP_IF_FLAGS_IF4       (1 << 4)
#define     REGISTER_DIP_IF_FLAGS_IF3       (1 << 3)
#define     REGISTER_DIP_IF_FLAGS_IF2       (1 << 2)
#define     REGISTER_DIP_IF_FLAGS_IF1       (1 << 1)
#define REGISTER_CH_STAT_B( x )         REGISTER( REGISTER_PAGE_HDMI_AUDIO, 0x14 + (x) )    /* RW */

/* HDCP and OTP (page: 12h) */
#define REGISTER_TX3                    REGISTER( REGISTER_PAGE_HDMI_HDCP_OTP, 0x9a )       /* RW */
#define REGISTER_TX4                    REGISTER( REGISTER_PAGE_HDMI_HDCP_OTP, 0x9b )       /* RW */
#define     REGISTER_TX4_PD_RAM             (1 << 1)
#define REGISTER_TX33                   REGISTER( REGISTER_PAGE_HDMI_HDCP_OTP, 0xb8 )       /* RW */
#define     REGISTER_TX33_HDMI              (1 << 1)

/* CEC registers (not paged) */
#define REGISTER_CEC_INTSTATUS          REGISTER( 0x00, 0xee )                              /* R */
#define     REGISTER_CEC_INTSTATUS_HDMI     (1 << 1)
#define     REGISTER_CEC_INTSTATUS_CEC      (1 << 0)
#define REGISTER_CEC_FRO_IM_CLK_CTRL    REGISTER( 0x00, 0xfb )                              /* RW */
#define     REGISTER_CEC_FRO_IM_CLK_CTRL_GHOST_DIS  (1 << 7)
#define     REGISTER_CEC_FRO_IM_CLK_CTRL_ENA_OTP    (1 << 6)
#define     REGISTER_CEC_FRO_IM_CLK_CTRL_IMCLK_SEL  (1 << 1)
#define     REGISTER_CEC_FRO_IM_CLK_CTRL_FRO_DIV    (1 << 0)
#define REGISTER_CEC_RXSHPDINTENA       REGISTER( 0x00, 0xfc )                              /* RW */
#define REGISTER_CEC_RXSHPDINT          REGISTER( 0x00, 0xfd )                              /* R */
#define REGISTER_CEC_RXSHPDLEV          REGISTER( 0x00, 0xfe )                              /* R */
#define     REGISTER_CEC_RXSHPDLEV_HPD      (1 << 1)
#define     REGISTER_CEC_RXSHPDLEV_RXSENS   (1 << 0)
#define REGISTER_CEC_ENAMODS            REGISTER( 0x00, 0xff )                              /* RW */
#define     REGISTER_CEC_ENAMODS_DIS_FRO    (1 << 6)
#define     REGISTER_CEC_ENAMODS_DIS_CCLK   (1 << 5)
#define     REGISTER_CEC_ENAMODS_EN_RXSENS  (1 << 2)
#define     REGISTER_CEC_ENAMODS_EN_HDMI    (1 << 1)
#define     REGISTER_CEC_ENAMODS_EN_CEC     (1 << 0)


/*************************************************/
/*                   HARDWARE                    */
/*************************************************/


/* Device version */
#define TDA9989N2                       0x0101
#define TDA19989                        0x0201
#define TDA19989N2                      0x0202
#define TDA19988                        0x0301


/*************************************************/
/*                 I2C FUNCTIONS                 */
/*************************************************/


static void tda998x_cec_register_write8( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t reg, uint8_t val )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint16_t        cec_addr = 0x34 + (vpout->hdmi[port].device.tda998x.address & 0x03) /* CEC I2C address bound to TDA998x I2C addr by configuration pins */;
    int             ret      = 0;

    ret = i2c_write( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed, cec_addr, reg, &val, 1 );
    if ( ret < 0 )
        goto fail;

    return;

fail:
    disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller CEC register writing failed [I2C issue: port=%d, register=0x%x, status=%d]", port, reg, ret );
}


static uint8_t tda998x_cec_register_read8( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t reg )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint16_t        cec_addr = 0x34 + (vpout->hdmi[port].device.tda998x.address & 0x03) /* CEC I2C address bound to TDA998x I2C addr by configuration pins */;
    uint8_t         val      = 0;
    int             ret      = 0;

    ret = i2c_read( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed, cec_addr, reg, &val, 1 );
    if ( ret < 0 )
        goto fail;

    return val;

fail:
    disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller CEC register reading failed [I2C issue: port=%d, register=0x%x, status=%d]", port, reg, ret );

    return (0);
}


static int tda998x_set_page( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t page )
{
    if ( page != vpout->hdmi[port].device.tda998x.current_page )
    {
        disp_adapter_t  *adapter = vpout->adapter;
        int             ret      = 0;

        ret = i2c_write( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed,
                         vpout->hdmi[port].device.tda998x.address, REGISTER_CURPAGE, &page, 1 );
        if ( ret < 0 )
        {
            disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller registers page setup failed [I2C issue: port=%d, status=%d]", port, ret );
            return ret;
        }

        vpout->hdmi[port].device.tda998x.current_page = page;
    }

    return (0);
}


static int tda998x_register_read8_range( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg, char *buf, int count )
{
    disp_adapter_t  *adapter = vpout->adapter;
    int             ret      = 0;

    ret = tda998x_set_page( vpout, vpout_draw, port, REGISTER_PAGE( reg ) );
    if ( ret < 0 )
        goto out;

    ret = i2c_read( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed, vpout->hdmi[port].device.tda998x.address,
                    REGISTER_ADDRESS( reg ), (uint8_t *)buf, count );
    if ( ret < 0 )
        goto fail;

    goto out;

fail:
    disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller register reading failed [I2C issue: port=%d, register=0x%x, status=%d]", port, reg, ret );

out:
    return ret;
}


static __attribute__((unused)) void tda998x_register_write8_range( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg,
                                                                   uint8_t *ptr, int count )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint8_t         *buf     = (uint8_t *)malloc( count * sizeof( uint8_t ) );
    int             ret      = 0;

    memcpy( buf, ptr, count );

    ret = tda998x_set_page( vpout, vpout_draw, port, REGISTER_PAGE( reg ) );
    if ( ret < 0 )
        goto out;

    ret = i2c_write( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed, vpout->hdmi[port].device.tda998x.address,
                     REGISTER_ADDRESS( reg ), buf, count );
    if ( ret < 0 )
        disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller register writing failed [I2C issue: port=%d, register=0x%x, status=%d]", port, reg, ret );

out:
    if ( buf )
        free( buf );

    return;
}


static int tda998x_register_read8( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg )
{
    uint8_t         val = 0;
    int             ret = 0;

    ret = tda998x_register_read8_range( vpout, vpout_draw, port, reg, (char *)&val, sizeof( val ) );
    if ( ret < 0 )
        return ret;

    return val;
}


static void tda998x_register_write8( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg, uint8_t val )
{
    disp_adapter_t  *adapter = vpout->adapter;
    int             ret      = 0;

    ret = tda998x_set_page( vpout, vpout_draw, port, REGISTER_PAGE( reg ) );
    if ( ret < 0 )
        goto out;

    ret = i2c_write( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed, vpout->hdmi[port].device.tda998x.address,
                     REGISTER_ADDRESS( reg ), &val, 1 );
    if ( ret < 0 )
        disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller register writing failed [I2C issue: port=%d, register=0x%x, status=%d]", port, reg, ret );

out:
    return;
}


static void tda998x_register_write16( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg, uint16_t val )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint8_t         buf[]    = { val >> 8, val };
    int             ret      = 0;

    ret = tda998x_set_page( vpout, vpout_draw, port, REGISTER_PAGE( reg ) );
    if ( ret < 0 )
        goto out;

    ret = i2c_write( vpout, vpout_draw, port, vpout->hdmi[port].device.tda998x.bus, vpout->hdmi[port].device.tda998x.speed, vpout->hdmi[port].device.tda998x.address,
                     REGISTER_ADDRESS( reg ), buf, 2 );
    if ( ret < 0 )
        disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller register writing failed [I2C issue: port=%d, register=0x%x, status=%d]", port, reg, ret );

out:
    return;
}


static void tda998x_register_set8( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg, uint8_t val )
{
    int             old_val;

    old_val = tda998x_register_read8( vpout, vpout_draw, port, reg );
    if ( old_val >= 0 )
        tda998x_register_write8( vpout, vpout_draw, port, reg, old_val | val );
}


static void tda998x_register_clear8( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t reg, uint8_t val )
{
    int             old_val;

    old_val = tda998x_register_read8( vpout, vpout_draw, port, reg );
    if ( old_val >= 0 )
        tda998x_register_write8( vpout, vpout_draw, port, reg, old_val & ~val );
}


/*************************************************/
/*              HARDWARE FUNCTIONS               */
/*************************************************/


static int tda998x_read_edid_block( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index, uint8_t *buf, uint32_t block, size_t length )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint8_t         offset   = (block & 1) ? 128 : 0,
                    segptr   = block / 2;
    int             ret, i;

    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_DDC_ADDR, 0xa0 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_DDC_OFFS, offset );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_DDC_SEGM_ADDR, 0x60 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_DDC_SEGM, segptr );

    /* Enable EDID reading sequence */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_EDID_CTRL, 0x1 );

    /* Clearing EDID reading sequence enable flag */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_EDID_CTRL, 0x0 );

    /* wait for block read to complete: */
    for ( i = 100; i > 0; i-- )
    {
        disp_usecspin( 1 * 1000 );
        ret = tda998x_register_read8( vpout, vpout_draw, port_index, REGISTER_INT_FLAGS_2 );
        if ( ret < 0 )
            return ret;
        if ( ret & REGISTER_INT_FLAGS_2_EDID_BLK_RD )
            break;
    }

    if ( i == 0 )
    {
        disp_printf( adapter, "[vpoutfb: tda998x] Error: I2C HDMI controller DDC transaction timeout [I2C issue: port=%d]", port_index );
        return (-ETIMEDOUT);
    }

    ret = tda998x_register_read8_range( vpout, vpout_draw, port_index, REGISTER_EDID_DATA_0, (char *)buf, length );
    if ( ret )
    {
        disp_printf( adapter, "[vpoutfb: tda998x] Error: I2C HDMI controller DDC transaction failed [I2C issue: port=%d]", port_index );
        return ret;
    }

    return (0);
}


static void tda998x_setup_dpms( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index, bool switch_on )
{
    if ( switch_on )
    {
        /* Enable video ports (audio lanes not enabled) */
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENA_VP_0, 0xff );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENA_VP_1, 0xff );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENA_VP_2, 0xff );

        /* Setup ports muxing */
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIP_CNTRL_0, vpout->hdmi[port_index].device.tda998x.vip_cntrl_0 );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIP_CNTRL_1, vpout->hdmi[port_index].device.tda998x.vip_cntrl_1 );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIP_CNTRL_2, vpout->hdmi[port_index].device.tda998x.vip_cntrl_2 );
    } else {
        /* Disable video ports */
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENA_VP_0, 0x00 );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENA_VP_1, 0x00 );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENA_VP_2, 0x00 );
    }
}


/*************************************************/
/*              INTERFACE FUNCTIONS              */
/*************************************************/


void tda998x_probe( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index )
{
    disp_adapter_t  *adapter = vpout->adapter;

    if ( i2c_init( vpout, vpout_draw, vpout->hdmi[port_index].device.tda998x.bus, port_index ) )
    {
        disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller initialization failed [I2C issue: port=%d]", port_index );
        return;
    }

    vpout->hdmi[port_index].device.tda998x.current_page = 0xff;

    vpout->hdmi[port_index].device.tda998x.vip_cntrl_0  = REGISTER_VIP_CNTRL_0_SWAP_A( 2 ) | REGISTER_VIP_CNTRL_0_SWAP_B( 3 );
    vpout->hdmi[port_index].device.tda998x.vip_cntrl_1  = REGISTER_VIP_CNTRL_1_SWAP_C( 0 ) | REGISTER_VIP_CNTRL_1_SWAP_D( 1 );
    vpout->hdmi[port_index].device.tda998x.vip_cntrl_2  = REGISTER_VIP_CNTRL_2_SWAP_E( 4 ) | REGISTER_VIP_CNTRL_2_SWAP_F( 5 );
}


void tda998x_reset( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index )
{
    /* reset audio and i2c master */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_SOFTRESET, REGISTER_SOFTRESET_AUDIO | REGISTER_SOFTRESET_I2C_MASTER );
    disp_usecspin( 50 * 1000 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_SOFTRESET, 0 );
    disp_usecspin( 50 * 1000 );

    /* reset transmitter */
    tda998x_register_set8( vpout, vpout_draw, port_index, REGISTER_MAIN_CNTRL0, REGISTER_MAIN_CNTRL0_SR );
    tda998x_register_clear8( vpout, vpout_draw, port_index, REGISTER_MAIN_CNTRL0, REGISTER_MAIN_CNTRL0_SR );

    /* PLL registers common configuration */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SERIAL_1, 0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SERIAL_2, REGISTER_PLL_SERIAL_2_SRL_NOSC( 1 ) );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SERIAL_3, 0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_SERIALIZER,   0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_BUFFER_OUT,   0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SCG1,     0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_AUDIO_DIV,    REGISTER_AUDIO_DIV_SERCLK_8 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_SEL_CLK,      REGISTER_SEL_CLK_SEL_CLK1 | REGISTER_SEL_CLK_ENA_SC_CLK );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SCGN1,    0xfa );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SCGN2,    0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SCGR1,    0x5b );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SCGR2,    0x00 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SCG2,     0x10 );

    /* Write the default value MUX register */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_MUX_VP_VIP_OUT, 0x24 );
}


void tda998x_remove( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index )
{
    if ( vpout->hdmi[port_index].device.tda998x.revision != 0 )
    {
        /* Switch ports OFF */
        tda998x_setup_dpms( vpout, vpout_draw, port_index, false );

        /* Disable IRQs */
        tda998x_cec_register_write8( vpout, vpout_draw, port_index, REGISTER_CEC_RXSHPDINTENA, 0 );
        tda998x_register_clear8( vpout, vpout_draw, port_index, REGISTER_INT_FLAGS_2, REGISTER_INT_FLAGS_2_EDID_BLK_RD );
    }

    i2c_fini( vpout, vpout_draw, port_index );
}


int tda998x_init( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index, uint32_t pixel_clock )
{
    disp_adapter_t  *adapter = vpout->adapter;
    int             ret      = 0;
    uint8_t         value    = 0;
    uint8_t         edid[256];

    /* Wake up the device */
    tda998x_cec_register_write8( vpout, vpout_draw, port_index, REGISTER_CEC_ENAMODS, REGISTER_CEC_ENAMODS_EN_RXSENS | REGISTER_CEC_ENAMODS_EN_HDMI );

    tda998x_reset( vpout, vpout_draw, port_index );

    /* Read device version */
    ret = tda998x_register_read8( vpout, vpout_draw, port_index, REGISTER_VERSION_LSB );
    if ( ret < 0 )
        goto fail;
    else
        vpout->hdmi[port_index].device.tda998x.revision |= ret;
    ret = tda998x_register_read8( vpout, vpout_draw, port_index, REGISTER_VERSION_MSB );
    if ( ret < 0 )
        goto fail;
    else
        vpout->hdmi[port_index].device.tda998x.revision |= ret << 8;

    /* Mask features bits: not-hdcp and not-scalar bit cleanup */
    vpout->hdmi[port_index].device.tda998x.revision &= ~0x30;

    /* Decode device version */
    switch ( vpout->hdmi[port_index].device.tda998x.revision )
    {
        case TDA9989N2:
            disp_printf( adapter, "[vpoutfb: tda998x] Info: TDA9989-2 I2C HDMI transmitter detected" );
            break;
        case TDA19989:
            disp_printf( adapter, "[vpoutfb: tda998x] Info: TDA19989 I2C HDMI transmitter detected" );
            break;
        case TDA19989N2:
            disp_printf( adapter, "[vpoutfb: tda998x] Info: TDA19989-2 I2C HDMI transmitter detected" );
            break;
        case TDA19988:
            disp_printf( adapter, "[vpoutfb: tda998x] Info: TDA19988 I2C HDMI transmitter detected" );
            break;
        default:
            disp_printf( adapter, "[vpoutfb: tda998x] Fatal: I2C HDMI controller initialization failed [found unsupported device: %04x]",
            vpout->hdmi[port_index].device.tda998x.revision );
            goto fail;
    }

    /* Enable DDC */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_DDC_DISABLE, 0x00 );

    /* Set clock on DDC channel */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_TX3, 39 );

    /* Disable multi-master mode */
    if ( vpout->hdmi[port_index].device.tda998x.revision == TDA19989 )
        tda998x_register_set8( vpout, vpout_draw, port_index, REGISTER_I2C_MASTER, REGISTER_I2C_MASTER_DIS_MM );

    tda998x_cec_register_write8( vpout, vpout_draw, port_index, REGISTER_CEC_FRO_IM_CLK_CTRL, REGISTER_CEC_FRO_IM_CLK_CTRL_GHOST_DIS |
                                                                                              REGISTER_CEC_FRO_IM_CLK_CTRL_IMCLK_SEL );

    /* Enable EDID IRQ */
    tda998x_register_set8( vpout, vpout_draw, port_index, REGISTER_INT_FLAGS_2, REGISTER_INT_FLAGS_2_EDID_BLK_RD );

    /* Apply optional video-ports properties */
    if ( vpout->hdmi[port_index].device.tda998x.video_ports != 0 )
    {
        vpout->hdmi[port_index].device.tda998x.vip_cntrl_0 = vpout->hdmi[port_index].device.tda998x.video_ports >> 16;
        vpout->hdmi[port_index].device.tda998x.vip_cntrl_1 = vpout->hdmi[port_index].device.tda998x.video_ports >> 8;
        vpout->hdmi[port_index].device.tda998x.vip_cntrl_2 = vpout->hdmi[port_index].device.tda998x.video_ports;
    }

    /* Check if monitor detected (hot-plug detection bit) */
    value = tda998x_cec_register_read8( vpout, vpout_draw, port_index, REGISTER_CEC_RXSHPDLEV );
    if ( value & REGISTER_CEC_RXSHPDLEV_HPD )
        disp_printf( adapter, "[vpoutfb: tda998x] Info: monitor connected" );

    /* Read EDID and detect HDMI sink */
    if ( tda998x_read_edid( vpout, vpout_draw, port_index, (uint8_t *)edid, sizeof( edid ) ) != sizeof( edid ) )
        disp_printf( adapter, "[vpoutfb: tda998x] Error: I2C HDMI controller DDC channel issue [EDID not found]" );
    vpout->hdmi[port_index].device.tda998x.is_hdmi_sink = drm_detect_hdmi_monitor( (struct edid *)edid );

    return (0);

fail:
    return (-1);
}


void tda998x_encoder_mode_set( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index, disp_crtc_settings_t *settings )
{
    uint16_t        ref_pix, ref_line, n_pix, n_line, hs_pix_s, hs_pix_e, vs1_pix_s, vs1_pix_e, vs1_line_s, vs1_line_e, vs2_pix_s, vs2_pix_e, vs2_line_s,
                    vs2_line_e, vwin1_line_s, vwin1_line_e, vwin2_line_s, vwin2_line_e, de_pix_s, de_pix_e;
    uint8_t         reg, div, rep,
                    h_sync_pol = settings->sync_polarity & DISP_SYNC_POLARITY_H_POS ? 1 : 0,
                    v_sync_pol = settings->sync_polarity & DISP_SYNC_POLARITY_V_POS ? 1 : 0;

    /* Internally TDA998x is using ITU-R BT.656 style sync but we get VESA style sync. TDA998x is using a reference pixel relative to ITU to sync to the input
     * frame and for output sync generation. Currently, we are using reference detection from HS/VS, i.e. REFPIX/REFLINE denote frame start sync point which is
     * position of rising VS with coincident rising HS. Now there is some issues to take care of:
     * - HDMI data islands require sync-before-active
     * - TDA998x register values must be > 0 to be enabled
     * - REFLINE needs an additional offset of +1
     * - REFPIX needs an addtional offset of +1 for UYUV and +3 for RGB
     * So we add +1 to all horizontal and vertical register values, plus an additional +3 for REFPIX as we are using RGB input only. */
    n_pix        = settings->h_total;
    n_line       = settings->v_total;

    hs_pix_e     = (settings->h_sync_start + settings->h_sync_len) - settings->xres;
    hs_pix_s     = settings->h_sync_start - settings->xres;
    de_pix_e     = settings->h_total;
    de_pix_s     = settings->h_total - settings->xres;
    ref_pix      = 3 + hs_pix_s;

#if 0
    /* Attached LCD controllers may generate broken sync. Allow those to adjust the position of the rising VS edge by adding HSKEW to ref_pix. */
    if ( adjusted_mode->flags & DRM_MODE_FLAG_HSKEW )
        ref_pix += adjusted_mode->hskew;
#else
    if ( VPOUT_LCD_SYNC_FIX( vpout ) )
    {
        ref_pix += settings->h_sync_len;
        h_sync_pol = 1 - h_sync_pol;
    }
#endif

    if ( (settings->sync_polarity & DISP_SYNC_INTERLACED) == 0 )
    {
        ref_line     = 1 + settings->v_sync_start - settings->yres;
        vwin1_line_s = settings->v_total - settings->yres - 1;
        vwin1_line_e = vwin1_line_s + settings->yres;
        vs1_pix_s    = vs1_pix_e = hs_pix_s;
        vs1_line_s   = settings->v_sync_start - settings->yres;
        vs1_line_e   = vs1_line_s + settings->v_sync_len;
        vwin2_line_s = vwin2_line_e = 0;
        vs2_pix_s    = vs2_pix_e  = 0;
        vs2_line_s   = vs2_line_e = 0;
    } else {
        ref_line     = 1 + (settings->v_sync_start - settings->yres) / 2;
        vwin1_line_s = (settings->v_total - settings->yres) / 2;
        vwin1_line_e = vwin1_line_s + settings->yres / 2;
        vs1_pix_s    = vs1_pix_e = hs_pix_s;
        vs1_line_s   = (settings->v_sync_start - settings->yres) / 2;
        vs1_line_e   = vs1_line_s + settings->v_sync_len / 2;
        vwin2_line_s = vwin1_line_s + settings->v_total / 2;
        vwin2_line_e = vwin2_line_s + settings->yres / 2;
        vs2_pix_s    = vs2_pix_e = hs_pix_s + settings->h_total / 2;
        vs2_line_s   = vs1_line_s + settings->v_total / 2 ;
        vs2_line_e   = vs2_line_s + settings->v_sync_len / 2;
    }

    div = 148500 / settings->pixel_clock;
    if ( div != 0 )
    {
        div--;
        if ( div > 3 )
            div = 3;
    }

    /* Mute audio FIFO */
    tda998x_register_set8( vpout, vpout_draw, port_index, REGISTER_AIP_CNTRL_0, REGISTER_AIP_CNTRL_0_RST_FIFO );

    /* HDMI HDCP mode off */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_TBG_CNTRL_1, REGISTER_TBG_CNTRL_1_DWIN_DIS );
    tda998x_register_clear8( vpout, vpout_draw, port_index, REGISTER_TX33, REGISTER_TX33_HDMI );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENC_CNTRL, REGISTER_ENC_CNTRL_CTL_CODE( 0 ) );

    /* No pre-filter/interpolator */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_HVF_CNTRL_0, REGISTER_HVF_CNTRL_0_PREFIL( 0 ) | REGISTER_HVF_CNTRL_0_INTPOL( 0 ) );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIP_CNTRL_5, REGISTER_VIP_CNTRL_5_SP_CNT( 0 ) );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIP_CNTRL_4, REGISTER_VIP_CNTRL_4_BLANKIT( 0 ) | REGISTER_VIP_CNTRL_4_BLC( 0 ) );

    tda998x_register_clear8( vpout, vpout_draw, port_index, REGISTER_PLL_SERIAL_1, REGISTER_PLL_SERIAL_1_SRL_MAN_IZ );
    tda998x_register_clear8( vpout, vpout_draw, port_index, REGISTER_PLL_SERIAL_3, REGISTER_PLL_SERIAL_3_SRL_CCIR | REGISTER_PLL_SERIAL_3_SRL_DE );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_SERIALIZER, 0 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_HVF_CNTRL_1, REGISTER_HVF_CNTRL_1_VQR( 0 ) );

    /* TODO: enable pixel repeat for pixel rates less than 25Msamp/s */
    rep = 0;
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_RPT_CNTRL, 0 );
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_SEL_CLK, REGISTER_SEL_CLK_SEL_VRF_CLK( 0 ) | REGISTER_SEL_CLK_SEL_CLK1 |
                                                                              REGISTER_SEL_CLK_ENA_SC_CLK );

    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_PLL_SERIAL_2, REGISTER_PLL_SERIAL_2_SRL_NOSC( div ) | REGISTER_PLL_SERIAL_2_SRL_PR( rep ) );

    /* Set color matrix bypass flag */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_MAT_CONTRL, REGISTER_MAT_CONTRL_MAT_BP | REGISTER_MAT_CONTRL_MAT_SC( 1 ) );

    /* Set BIAS tmds value */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ANA_GENERAL, 0x09 );

    /* Sync on rising HSYNC/VSYNC */
    reg = REGISTER_VIP_CNTRL_3_SYNC_HS;

    /* TDA19988 requires high-active sync at input stage, so invert low-active sync provided by master encoder here */
    if ( h_sync_pol == 0 )
        reg |= REGISTER_VIP_CNTRL_3_H_TGL;
    if ( v_sync_pol == 0 )
        reg |= REGISTER_VIP_CNTRL_3_V_TGL;
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIP_CNTRL_3, reg );

    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_VIDFORMAT, 0x00 );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_REFPIX_MSB, ref_pix );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_REFLINE_MSB, ref_line );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_NPIX_MSB, n_pix );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_NLINE_MSB, n_line );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_LINE_STRT_1_MSB, vs1_line_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_PIX_STRT_1_MSB, vs1_pix_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_LINE_END_1_MSB, vs1_line_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_PIX_END_1_MSB, vs1_pix_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_LINE_STRT_2_MSB, vs2_line_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_PIX_STRT_2_MSB, vs2_pix_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_LINE_END_2_MSB, vs2_line_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VS_PIX_END_2_MSB, vs2_pix_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_HS_PIX_START_MSB, hs_pix_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_HS_PIX_STOP_MSB, hs_pix_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VWIN_START_1_MSB, vwin1_line_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VWIN_END_1_MSB, vwin1_line_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VWIN_START_2_MSB, vwin2_line_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_VWIN_END_2_MSB, vwin2_line_e );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_DE_START_MSB, de_pix_s );
    tda998x_register_write16( vpout, vpout_draw, port_index, REGISTER_DE_STOP_MSB, de_pix_e );

    if ( vpout->hdmi[port_index].device.tda998x.revision == TDA19988 )
        /* Let incoming pixels fill the active space (if any) */
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENABLE_SPACE, 0x00 );

    /* Always generate sync polarity relative to input sync and revert input stage toggled sync at output stage */
    reg = REGISTER_TBG_CNTRL_1_DWIN_DIS | REGISTER_TBG_CNTRL_1_TGL_EN;
    if ( h_sync_pol == 0 )
        reg |= REGISTER_TBG_CNTRL_1_H_TGL;
    if ( v_sync_pol == 0 )
        reg |= REGISTER_TBG_CNTRL_1_V_TGL;
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_TBG_CNTRL_1, reg );

    /* Must be last register set */
    tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_TBG_CNTRL_0, 0 );

    /* Only setup the info frames if the sink is HDMI */
    if ( vpout->hdmi[port_index].device.tda998x.is_hdmi_sink )
    {
#if 0
        /* We need to turn HDMI HDCP stuff on to get audio through */
        reg &= ~REGISTER_TBG_CNTRL_1_DWIN_DIS;
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_TBG_CNTRL_1, reg );
        tda998x_register_write8( vpout, vpout_draw, port_index, REGISTER_ENC_CNTRL, REGISTER_ENC_CNTRL_CTL_CODE( 1 ) );
        tda998x_register_set8( vpout, vpout_draw, port_index, REGISTER_TX33, REGISTER_TX33_HDMI );

        tda998x_write_avi( vpout, vpout_draw, port_index, adjusted_mode );
#endif
    }

    /* Switch ports ON */
    tda998x_setup_dpms( vpout, vpout_draw, port_index, true );
}


int tda998x_read_edid( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index, uint8_t *buf, int size_ )
{
    int             size = 0,
                    ret  = 0;

    if ( size_ != 256 )
        return (-1);

    if ( vpout->hdmi[port_index].device.tda998x.revision == TDA19988 )
        tda998x_register_clear8( vpout, vpout_draw, port_index, REGISTER_TX4, REGISTER_TX4_PD_RAM );

    ret   = tda998x_read_edid_block( vpout, vpout_draw, port_index, &buf[0], 0 /* EDID block #0 */, 128 /* EDID block size */ );
    size += 128;
    if ( ret < 0 )
        goto done;
    ret   = tda998x_read_edid_block( vpout, vpout_draw, port_index, &buf[128], 1 /* EDID block #1 */, 128 /* EDID block size */ );
    size += 128;
    if ( ret < 0 )
        goto done;
    ret = size;

done:
    if ( vpout->hdmi[port_index].device.tda998x.revision == TDA19988 )
        tda998x_register_set8( vpout, vpout_draw, port_index, REGISTER_TX4, REGISTER_TX4_PD_RAM );

    return ret;
}
