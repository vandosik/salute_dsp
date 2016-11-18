/*************************************************/
/*               COMMON REGISTERS                */
/*    Elvees 1892VM14YA ARMv7 SoC (VPOUT DC)     */
/*************************************************/


/*************************************************/
/*                Registers base                 */
/*************************************************/


/* Synchronization interface controller (CMCTR) */
#define CMCTR_REGISTERS_BASE            0x38094000
#define CMCTR_REGISTERS_SIZE            0x1000

/* GPIO interface */
#define GPIO_REGISTERS_SIZE             0x1000


/*************************************************/
/*              Registers interface              */
/*************************************************/


/* Common registers interface */
#define MMIO64( offset )                ((uint64_t *)(vpout_draw->registers + (offset)))
#define MMIO32( offset )                ((uint32_t *)(vpout_draw->registers + (offset)))

/* CMCTR registers interface */
#define CMCTR_MMIO64( offset )          ((uint64_t *)(vpout->cmctr_registers + (offset)))
#define CMCTR_MMIO32( offset )          ((uint32_t *)(vpout->cmctr_registers + (offset)))

/* GPIO registers interface */
#define GPIO_MMIO64( base, offset )     ((uint64_t *)(base + (offset)))
#define GPIO_MMIO32( base, offset )     ((uint32_t *)(base + (offset)))


/*************************************************/
/*      Display (LCD) controller registers       */
/*************************************************/


#define LCDCSR                          (0x00)                                  /* LCD control and status */
#define     CSR_CLR                         (1 << 3)
#define     CSR_INIT                        (1 << 2)
#define     CSR_RUN                         (1 << 1)
#define     CSR_EN                          (1 << 0)

#define LCDDIV                          (0x04)                                  /* ACLK pixel clock divider */

#define LCDMODE                         (0x08)
#define     LCDMODE_CLK_ON                  (1 << 17)
#define     LCDMODE_VDEF                    (1 << 16)
#define     LCDMODE_HDEF                    (1 << 15)
#define     LCDMODE_DEN_EN                  (1 << 14)
#define     LCDMODE_INSYNC                  (1 << 13)
#define     LCDMODE_CCM                     (1 << 12)
#define     LCDMODE_PINV                    (1 << 11)
#define     LCDMODE_DINV                    (1 << 10)
#define     LCDMODE_VINV                    (1 << 9)
#define     LCDMODE_HINV                    (1 << 8)
#define     LCDMODE_BUF_NUMB                (1 << 7)
#define     LCDMODE_BUF_MODE                (1 << 6)
#define     LCDMODE_HWC_MODE                (1 << 5)                            /* Hardware cursor mode */
#define     LCDMODE_HWC_MODE_32x32              (0 << 5)
#define     LCDMODE_HWC_MODE_64x64              (1 << 5)
#define     LCDMODE_HWCEN                   (1 << 4)                            /* Enable hardware cursor */
#define     LCDMODE_INSIZE                  (0xf << 0)
#define     LCDMODE_INSIZE_PAL8                 (0 << 0)
#define     LCDMODE_INSIZE_RGB444               (1 << 0)
#define     LCDMODE_INSIZE_ARGB1555             (2 << 0)
#define     LCDMODE_INSIZE_RGB565               (3 << 0)
#define     LCDMODE_INSIZE_18bpp                (4 << 0)
#define     LCDMODE_INSIZE_RGB888               (5 << 0)
#define     LCDMODE_INSIZE_ARGB8888             (6 << 0)
#define     LCDMODE_INSIZE_INDEXED_1BIT         (8 << 0)
#define     LCDMODE_INSIZE_INDEXED_2BIT         (9 << 0)
#define     LCDMODE_INSIZE_INDEXED_4BIT         (10 << 0)
#define     LCDMODE_INSIZE_INDEXED_8BIT         (11 << 0)

#define LCDHT0                          (0x0c)                                  /* LCD horizontal timings */
#define     LCDHT0_HGDEL_MASK               (0xffff << 16)
#define     LCDHT0_HGDEL_SHIFT              (16)
#define     LCDHT0_HSW_MASK                 (0xffff << 0)
#define     LCDHT0_HSW_SHIFT                (0)

#define LCDHT1                          (0x10)                                  /* LCD horizontal timings */
#define     LCDHT1_HLEN_MASK                (0xffff << 16)
#define     LCDHT1_HLEN_SHIFT               (16)
#define     LCDHT1_HGATE_MASK               (0xffff << 0)
#define     LCDHT1_HGATE_SHIFT              (0)

#define LCDVT0                          (0x14)                                  /* LCD vertical timings */
#define     LCDHT1_VGDEL_MASK               (0xffff << 16)
#define     LCDHT1_VGDEL_SHIFT              (16)
#define     LCDHT1_VSW_MASK                 (0xffff << 0)
#define     LCDHT1_VSW_SHIFT                (0)

#define LCDVT1                          (0x18)                                  /* LCD vertical timings */
#define     LCDHT1_VLEN_MASK                (0xffff << 16)
#define     LCDHT1_VLEN_SHIFT               (16)
#define     LCDHT1_VGATE_MASK               (0xffff << 0)
#define     LCDHT1_VGATE_SHIFT              (0)

#define LCDAB0                          (0x2c)                                  /* LCD primary buffer address */

#define LCDAB1                          (0x30)                                  /* LCD secondary buffer address */

#define LCDOF0                          (0x34)                                  /* LCD primary buffer stride (64b aligned) */

#define LCDOF1                          (0x38)                                  /* LCD secondary buffer stride (64b aligned) */

#define LCDINT                          (0x44)                                  /* LCD interrupt control */
#define LCDINTMASK                      (0x48)                                  /* LCD interrupt mask */
#define     INTERRUPT_SYNC_DONE             (1 << 5)                            /* VSYNC IRQ */
#define     INTERRUPT_OUT_FIFO_EMPTY        (1 << 3)
#define     INTERRUPT_OUT_FIFO              (1 << 2)
#define     INTERRUPT_DMA_FIFO_EMPTY        (1 << 1)
#define     INTERRUPT_DMA_DONE              (1 << 0)


/*************************************************/
/*                CMCTR registers                */
/*************************************************/


#define GATE_CORE_CTR                   (0x48)                                  /* Frequency disable register for CMCTR_CORE */
#define     VPOUT_EN                        (1 << 4)                            /* Frequency enable bit for VPOUT, VPOUT_PCLK, VPOUT_ACLK */

#define SEL_CPLL                        (0x104)                                 /* CPLL control register */
#define     SEL_CPLL_LOCK                   (1 << 31)
#define     SEL_CPLL_SEL                    (0xff << 0)

#define SEL_SPLL                        (0x10c)                                 /* SPLL control register */
#define     SEL_SPLL_LOCK                   (1 << 31)
#define     SEL_SPLL_SEL                    (0xff << 0)


/*************************************************/
/*                GPIO registers                */
/*************************************************/


#define GPIO_SWPORTA_DR                 (0x00)
#define GPIO_SWPORTB_DR                 (0x0C)
#define GPIO_SWPORTC_DR                 (0x18)
#define GPIO_SWPORTD_DR                 (0x24)

#define GPIO_SWPORTA_DDR                (0x04)
#define GPIO_SWPORTB_DDR                (0x10)
#define GPIO_SWPORTC_DDR                (0x1C)
#define GPIO_SWPORTD_DDR                (0x28)

#define GPIO_SWPORTA_CTL                (0x08)
#define GPIO_SWPORTB_CTL                (0x14)
#define GPIO_SWPORTC_CTL                (0x20)
#define GPIO_SWPORTD_CTL                (0x2C)


/*************************************************/
/*   ITE IT66121 I2C HDMI controller registers   */
/*************************************************/


#define REG_RESET                       (0x04)
#define     RESET_RCLK_MASK                 (1 << 5)
#define     RESET_AUDIO_MASK                (1 << 4)
#define     RESET_VIDEO_MASK                (1 << 3)
#define     RESET_AFIFO_MASK                (1 << 2)
#define     RESET_HDCP_MASK                 (1 << 0)
#define REG_IRQ_FIRST                   (0x09)
#define REG_IRQ_LAST                    (0x0B)
#define REG_BANK_SW                     (0x0F)
#define REG_TX_RESET                    (0x61)
#define REG_CLK1                        (0x62)
#define REG_CLK2                        (0x63)
#define REG_CLK3                        (0x64)
#define REG_TXFIFO_SET                  (0x71)
#define REG_AVMUTE                      (0xC1)
