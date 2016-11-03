/* designware.c
 *
 * I2C bus driver for Synopsys DesignWare (TI DaVinci) adapters
 *
 *  Created on: 14.10.2016
 *      Author: SWD Embedded Systems Ltd. */


#include "designware.h"


/****************************************************/
/*                       Misc                       */
/****************************************************/


static void __i2c_dw_enable( designware_i2c_dev_t *dev, bool enable )
{
    uint32_t        reg     = READ( ic_enable );
    int             timeout = 100;

    do {
        WRITE( ic_enable, enable ? reg | IC_ENABLE : reg & (~IC_ENABLE) );
        if ( (READ( ic_enable_status ) & IC_EN) == (enable ? IC_EN : 0) )
            return;

        /* Wait 10 times the signaling period of the highest I2C transfer supported by the driver (for 400KHz this is
         * 25us) as described in the DesignWare I2C databook.*/
        usleep( 250 );
    } while ( timeout-- );

    i2c_printf( _SLOG_ERROR, "Error: timeout in %sabling adapter [%s()]", enable ? "en" : "dis", __FUNCTION__ );
}


/* Waits for bus busy */
static int i2c_wait_for_bb( designware_i2c_dev_t *dev )
{
    struct timespec start, stop;
    uint64_t        ms;

    if ( clock_gettime( CLOCK_REALTIME, &start) == -1 )
    {
        i2c_printf( _SLOG_ERROR, "Error: idle wait failed [%s(): can't get first timestamp]", __FUNCTION__ );
        return (1);
    }

    while ( (READ( ic_status ) & IC_STATUS_MA) || !(READ( ic_status ) & IC_STATUS_TFE) )
    {
        /* Evaluate timeout */
        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 )
        {
            i2c_printf( _SLOG_ERROR, "Error: idle wait failed [%s(): can't get last timestamp]", __FUNCTION__ );
            return (1);
        }

        ms = ((double)( stop.tv_sec - start.tv_sec ) * (double)NANO_TO_SEC + (double)( stop.tv_nsec - start.tv_nsec )) / NANO_TO_MILLI;
        if ( ms > (uint64_t)I2C_BYTE_TO_BB )
        {
            i2c_printf( _SLOG_ERROR, "Error: idle timeout [%s(): status=0x%x]", __FUNCTION__, READ( ic_status ) );
            return (1);
        }
    }

    return (0);
}


/* Flushes the i2c RX FIFO */
static void i2c_flush_rxfifo( designware_i2c_dev_t *dev )
{
    uint32_t        tmp;

    while ( READ( ic_status ) & IC_STATUS_RFNE )
        tmp = READ( ic_cmd_data );
}


static int i2c_xfer_finish( designware_i2c_dev_t *dev )
{
    struct timespec start, stop;
    uint64_t        ms;
    uint32_t        tmp;

    if ( clock_gettime( CLOCK_REALTIME, &start) == -1 )
    {
        i2c_printf( _SLOG_ERROR, "Error: transfer termination failed [%s(): can't get first timestamp]", __FUNCTION__ );
        return (1);
    }

    while ( 1 )
    {
        if ( READ( ic_raw_intr_stat ) & IC_STOP_DET )
        {
            tmp = READ( ic_clr_stop_det );
            break;
        } else {
            /* Evaluate timeout */
            if( clock_gettime( CLOCK_REALTIME, &stop) == -1 )
            {
                i2c_printf( _SLOG_ERROR, "Error: transfer termination failed [%s(): can't get last timestamp]", __FUNCTION__ );
                return (1);
            }
            ms = ((double)( stop.tv_sec - start.tv_sec ) * (double)NANO_TO_SEC + (double)( stop.tv_nsec - start.tv_nsec )) / NANO_TO_MILLI;
            if ( ms > (uint64_t)I2C_STOPDET_TO )
            {
                i2c_printf( _SLOG_ERROR, "Warning: transfer termination timeout [%s()]", __FUNCTION__ );
                break;
            }
        }
    }

    if ( i2c_wait_for_bb( dev ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: timed out waiting for bus [%s()]", __FUNCTION__ );
        return (1);
    }

    i2c_flush_rxfifo( dev );

    return (0);
}


/* Sets the target slave address */
static void i2c_setaddress( designware_i2c_dev_t *dev, unsigned int slave_address )
{
    /* Disable i2c */
    __i2c_dw_enable( dev, false );

    WRITE( ic_tar, slave_address );

    /* Enable i2c */
    __i2c_dw_enable( dev, true );
}


static int i2c_xfer_init( designware_i2c_dev_t *dev, uint8_t slave_address, uint32_t offset, int alen )
{
    if ( i2c_wait_for_bb( dev ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: device not ready [%s(): slave=0x%02x, offset=0x%02x]", __FUNCTION__, slave_address, offset );

        return (1);
    }

    i2c_setaddress( dev, slave_address );

    while ( alen )
    {
        alen--;

        /* high byte address going out first */
        WRITE( ic_cmd_data, (offset >> (alen * 8)) & 0xff );
    }

    return (0);
}


static uint32_t i2c_dw_scl_hcnt( uint32_t ic_clk, uint32_t tSYMBOL, uint32_t tf, int cond, int offset )
{
    /* DesignWare I2C core doesn't seem to have solid strategy to meet the tHD;STA timing spec.  Configuring _HCNT based on tHIGH spec
     * will result in violation of the tHD;STA spec. */
    if ( cond )
        /* Conditional expression: IC_[FS]S_SCL_HCNT + (1 + 4 + 3) >= IC_CLK * tHIGH
         *
         * This is based on the DW manuals, and represents an ideal configuration. The resulting I2C bus speed will be faster than any
         * of the others. If your hardware is free from tHD;STA issue, try this one. */
        return (ic_clk * tSYMBOL + 500000) / 1000000 - 8 + offset;

    /* Conditional expression: IC_[FS]S_SCL_HCNT + 3 >= IC_CLK * (tHD; STA + tf)
     *
     * This is just experimental rule; the tHD;STA period turned out to be proportinal to (_HCNT + 3). With this setting, we could
     * meet both tHIGH and tHD;STA timing specs. If unsure, you'd better to take this alternative. The reason why we need to take
     * into account "tf" here, is the same as described in i2c_dw_scl_lcnt(). */
    return (ic_clk * (tSYMBOL + tf) + 500000) / 1000000 - 3 + offset;
}


static uint32_t i2c_dw_scl_lcnt( uint32_t ic_clk, uint32_t tLOW, uint32_t tf, int offset )
{
    /* Conditional expression: IC_[FS]S_SCL_LCNT + 1 >= IC_CLK * (tLOW + tf)
     *
     * DW I2C core starts counting the SCL CNTs for the LOW period of the SCL clock (tLOW) as soon as it pulls the SCL line.
     * In order to meet the tLOW timing spec, we need to take into account the fall time of SCL signal (tf).  Default tf value
     * should be 0.3 us, for safety.*/
    return ((ic_clk * (tLOW + tf) + 500000) / 1000000) - 1 + offset;
}


/* Set the i2c speed mode (standard, high, fast) */
static void set_speed( designware_i2c_dev_t *dev, i2c_speed_mode_t speed )
{
    uint32_t        cntl;

    /* to set speed cltr must be disabled */
    __i2c_dw_enable( dev, false );

    cntl = READ( ic_con ) & (~IC_CON_SPEED_MSK);

    switch ( speed )
    {
        case I2C_SPEED_HIGH:
            cntl |= IC_CON_SPEED_HIGH;
            WRITE( ic_ss_scl_hcnt, i2c_dw_scl_hcnt( dev->frequency, MIN_HS_SCL_HIGHTIME, 300 /* ns */, 0, 0 ) );
            WRITE( ic_ss_scl_lcnt, i2c_dw_scl_lcnt( dev->frequency, MIN_HS_SCL_LOWTIME,  300 /* ns */, 0 ) );
            break;

        case I2C_SPEED_FAST:
            cntl |= IC_CON_SPEED_FAST;
            WRITE( ic_ss_scl_hcnt, i2c_dw_scl_hcnt( dev->frequency, MIN_FS_SCL_HIGHTIME, 300 /* ns */, 0, 0 ) );
            WRITE( ic_ss_scl_lcnt, i2c_dw_scl_lcnt( dev->frequency, MIN_FS_SCL_LOWTIME,  300 /* ns */, 0 ) );
            break;

        default:
        case I2C_SPEED_STANDARD:
            cntl |= IC_CON_SPEED_STD;
            WRITE( ic_ss_scl_hcnt, i2c_dw_scl_hcnt( dev->frequency, MIN_SS_SCL_HIGHTIME, 300 /* ns */, 0, 0 ) );
            WRITE( ic_ss_scl_lcnt, i2c_dw_scl_lcnt( dev->frequency, MIN_SS_SCL_LOWTIME,  300 /* ns */, 0 ) );
            break;
    }

    WRITE( ic_con, cntl );

    /* Enable back i2c now speed set */
    __i2c_dw_enable( dev, true );
}


/****************************************************/
/*                     Interface                    */
/****************************************************/


/* Write to i2c memory */
uint32_t designware_i2c_write( designware_i2c_dev_t *dev, uint8_t slave_address, uint32_t offset, int alen, uint8_t *buffer, int len )
{
    struct timespec start, stop;
    uint64_t        ms;
    int             nb = len;

#ifdef I2C_EEPROM_ADDR_OVERFLOW
    /* EEPROM chips that implement "address overflow" are ones like Catalyst 24WC04/08/16 which has 9/10/11 bits of address and the extra
     * bits end up in the "chip address" bit slots. This makes a 24WC08 (1Kbyte) chip look like four 256 byte chips.
     *
     * Note that we consider the length of the address field to still be one byte because the extra address bits are hidden in the chip address. */
    slave_address |= ((offset >> (alen * 8)) & I2C_EEPROM_ADDR_OVERFLOW);
    offset        &= ~(I2C_EEPROM_ADDR_OVERFLOW << (alen * 8));

    i2c_printf( _SLOG_INFO, "Info: fixing addr_overflow [%s(): slave=0x%02x, offset=0x%02x]", __FUNCTION__, slave_address, offset );
#endif

    if ( i2c_xfer_init( dev, slave_address, offset, alen ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: writing failed [%s(): transfer initialization failed, slave=0x%02x, offset=0x%02x]", __FUNCTION__,
                    slave_address, offset );

        return (1);
    }

    if ( clock_gettime( CLOCK_REALTIME, &start) == -1 )
    {
        i2c_printf( _SLOG_ERROR, "Error: writing failed [%s(): can't get first timestamp]", __FUNCTION__ );
        return (1);
    }

    while ( len )
    {
        if ( READ( ic_status ) & IC_STATUS_TFNF )
        {
            if ( --len == 0 ) {
                WRITE( ic_cmd_data, *buffer | IC_STOP );
            } else
                WRITE( ic_cmd_data, *buffer );
            buffer++;
            
            if ( clock_gettime( CLOCK_REALTIME, &start) == -1 )
            {
                i2c_printf( _SLOG_ERROR, "Error: writing failed [%s(): can't get next timestamp]", __FUNCTION__ );
                return (1);
            }
        } else {
            /* Evaluate timeout */
            if( clock_gettime( CLOCK_REALTIME, &stop) == -1 )
            {
                i2c_printf( _SLOG_ERROR, "Error: writing failed [%s(): can't get last timestamp]", __FUNCTION__ );
                return (1);
            }
            ms = ((double)( stop.tv_sec - start.tv_sec ) * (double)NANO_TO_SEC + (double)( stop.tv_nsec - start.tv_nsec )) / NANO_TO_MILLI;
            if ( ms > (uint64_t)(nb * I2C_BYTE_TO) )
            {
                i2c_printf( _SLOG_ERROR, "Error: writing failed [%s(): timeout]", __FUNCTION__ );
                return (1);
            }
        }
    }

    return i2c_xfer_finish( dev );
}


/* Read from i2c memory */
uint32_t designware_i2c_read( designware_i2c_dev_t *dev, uint8_t slave_address, uint32_t offset, int alen, uint8_t *buffer, int len )
{
    struct timespec start, stop;
    uint64_t        ms;

#ifdef I2C_EEPROM_ADDR_OVERFLOW
    /* EEPROM chips that implement "address overflow" are ones like Catalyst 24WC04/08/16 which has 9/10/11 bits of address and the extra
     * bits end up in the "chip address" bit slots. This makes a 24WC08 (1Kbyte) chip look like four 256 byte chips.
     *
     * Note that we consider the length of the address field to still be one byte because the extra address bits are hidden in the chip address. */
    slave_address |= ((offset >> (alen * 8)) & I2C_EEPROM_ADDR_OVERFLOW);
    offset        &= ~(I2C_EEPROM_ADDR_OVERFLOW << (alen * 8));

    i2c_printf( _SLOG_INFO, "Info: fixing addr_overflow [%s(): slave=0x%02x, offset=0x%02x]", __FUNCTION__, slave_address, offset );
#endif

    if ( i2c_xfer_init( dev, slave_address, offset, alen ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: reading failed [%s(): transfer initialization failed, slave=0x%02x, offset=0x%02x]", __FUNCTION__,
                    slave_address, offset );

        return (1);
    }

    if ( clock_gettime( CLOCK_REALTIME, &start) == -1 )
    {
        i2c_printf( _SLOG_ERROR, "Error: reading failed [%s(): can't get first timestamp]", __FUNCTION__ );
        return (1);
    }

    while ( len )
    {
        if ( len == 1 ) {
            WRITE( ic_cmd_data, IC_CMD | IC_STOP );
        } else
            WRITE( ic_cmd_data, IC_CMD );

        if ( READ( ic_status ) & IC_STATUS_RFNE )
        {
            *buffer++ = (uint8_t)READ( ic_cmd_data );
            len--;

            if ( clock_gettime( CLOCK_REALTIME, &start) == -1 )
            {
                i2c_printf( _SLOG_ERROR, "Error: reading failed [%s(): can't get next timestamp]", __FUNCTION__ );
                return (1);
            }
        } else {
            /* Evaluate timeout */
            if( clock_gettime( CLOCK_REALTIME, &stop) == -1 )
            {
                i2c_printf( _SLOG_ERROR, "Error: reading failed [%s(): can't get last timestamp]", __FUNCTION__ );
                return (1);
            }
            ms = ((double)( stop.tv_sec - start.tv_sec ) * (double)NANO_TO_SEC + (double)( stop.tv_nsec - start.tv_nsec )) / NANO_TO_MILLI;
            if ( ms > (uint64_t)I2C_BYTE_TO )
            {
                i2c_printf( _SLOG_ERROR, "Error: reading failed [%s(): timeout]", __FUNCTION__ );
                return (1);
            }
        }
    }

    return i2c_xfer_finish( dev );
}


/* Set the i2c speed */
uint32_t designware_i2c_set_bus_speed( designware_i2c_dev_t *dev, i2c_speed_mode_t speed )
{

    set_speed( dev, speed );
    dev->speed = speed;

    return 0;
}


void designware_i2c_reset( designware_i2c_dev_t *dev )
{
    /* Disable i2c */
    __i2c_dw_enable( dev, false );
}


/* Initialization function */
void designware_i2c_init( designware_i2c_dev_t *dev, i2c_speed_mode_t speed, uint32_t slaveaddr )
{
    /* Disable i2c */
    __i2c_dw_enable( dev, false );

    WRITE( ic_con, IC_CON_SLAVE_DISABLE | IC_CON_RESTART_EN | IC_CON_SPEED_FAST | IC_CON_MASTER_MODE );
    WRITE( ic_rx_tl, IC_RX_TL );
    WRITE( ic_tx_tl, IC_TX_TL );
    designware_i2c_set_bus_speed( dev, speed );
    WRITE( ic_intr_mask, IC_STOP_DET );
    WRITE( ic_sar, slaveaddr );

    /* Enable i2c */
    __i2c_dw_enable( dev, true );
}


/* Initialization function */
int synopsys_i2c_init( designware_i2c_dev_t *dev )
{
    uint32_t        param1 = READ( ic_comp_param_1 );
    uint32_t        reg    = READ( ic_comp_type );

    dev->tx_fifo_depth = ((param1 >> 16) & 0xff) + 1;
    dev->rx_fifo_depth = ((param1 >> 8)  & 0xff) + 1;
    i2c_printf( _SLOG_INFO, "Info: Synopsys: TX FIFO depth = %d", dev->tx_fifo_depth );
    i2c_printf( _SLOG_INFO, "Info: Synopsys: RX FIFO depth = %d", dev->rx_fifo_depth );

    #define ___constant_swab32( x )             ((uint32_t)((((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |  \
                                                            (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |  \
                                                            (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |  \
                                                            (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))
    if ( reg == ___constant_swab32( IC_COMP_TYPE_VALUE ) )
    {
        /* Configure register endianess access */
        dev->accessor_flags |= ACCESS_SWAP;
        i2c_printf( _SLOG_INFO, "Info: Synopsys: registers access swap enabled" );
    } else if ( reg == (IC_COMP_TYPE_VALUE & 0x0000ffff) )
    {
        /* Configure register access mode 16bit */
        dev->accessor_flags |= ACCESS_16BIT;
        i2c_printf( _SLOG_INFO, "Warning: Synopsys: 16-bits registers access not supported" );
    } else if ( reg != IC_COMP_TYPE_VALUE )
    {
        i2c_printf( _SLOG_ERROR, "Error: unknown Synopsys component type [0x%08x]", reg );
        return (-ENODEV);
    }
    #undef ___constant_swab32

    return (0);
}
