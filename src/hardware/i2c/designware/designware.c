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
            i2c_printf( _SLOG_ERROR, "Error: idle timeout [%s()]", __FUNCTION__ );
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
    REGISTER( ic_enable ) &= ~IC_ENABLE_0B;

    WRITE( ic_tar, slave_address );

    /* Enable i2c */
    REGISTER( ic_enable ) |= IC_ENABLE_0B;
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


/* Set the i2c speed mode (standard, high, fast) */
static void set_speed( designware_i2c_dev_t *dev, i2c_speed_mode_t speed )
{
    uint32_t        cntl;

    /* to set speed cltr must be disabled */
    REGISTER( ic_enable ) &= ~IC_ENABLE_0B;

    cntl = READ( ic_con ) & (~IC_CON_SPD_MSK);

    switch ( speed )
    {
        case I2C_SPEED_HIGH:
            cntl |= IC_CON_SPD_HS;
            REGISTER( ic_hs_scl_hcnt ) = (dev->frequency * MIN_HS_SCL_HIGHTIME) / NANO_TO_MICRO;
            REGISTER( ic_hs_scl_lcnt ) = (dev->frequency * MIN_HS_SCL_LOWTIME)  / NANO_TO_MICRO;
            break;

        case I2C_SPEED_FAST:
            cntl |= IC_CON_SPD_FS;
            REGISTER( ic_fs_scl_hcnt ) = (dev->frequency * MIN_FS_SCL_HIGHTIME) / NANO_TO_MICRO;
            REGISTER( ic_fs_scl_lcnt ) = (dev->frequency * MIN_FS_SCL_LOWTIME)  / NANO_TO_MICRO;
            break;

        default:
        case I2C_SPEED_STANDARD:
            cntl |= IC_CON_SPD_SS;
            REGISTER( ic_ss_scl_hcnt ) = (dev->frequency * MIN_SS_SCL_HIGHTIME) / NANO_TO_MICRO;
            REGISTER( ic_ss_scl_lcnt) = (dev->frequency * MIN_SS_SCL_LOWTIME)   / NANO_TO_MICRO;
            break;
    }

    WRITE( ic_con, cntl );

    /* Enable back i2c now speed set */
    REGISTER( ic_enable )  |= IC_ENABLE_0B;
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
            if ( --len == 0 )
                WRITE( ic_cmd_data, *buffer | IC_STOP );
            else
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
        if ( len == 1 )
            WRITE( ic_cmd_data, IC_CMD | IC_STOP );
        else
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
    REGISTER( ic_enable ) &= ~IC_ENABLE_0B;
}


/* Initialization function */
void designware_i2c_init( designware_i2c_dev_t *dev, i2c_speed_mode_t speed, uint32_t slaveaddr )
{
    /* Disable i2c */
    REGISTER( ic_enable ) &= ~IC_ENABLE_0B;

    WRITE( ic_con, IC_CON_SD | IC_CON_SPD_FS | IC_CON_MM );
    WRITE( ic_rx_tl, IC_RX_TL );
    WRITE( ic_tx_tl, IC_TX_TL );
    designware_i2c_set_bus_speed( dev, speed );
    WRITE( ic_intr_mask, IC_STOP_DET );
    WRITE( ic_sar, slaveaddr );

    /* Enable i2c */
    REGISTER( ic_enable ) |= IC_ENABLE_0B;
}
