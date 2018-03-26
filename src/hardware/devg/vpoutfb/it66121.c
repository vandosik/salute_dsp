/*************************************************/
/*  ITE IT66121 I2C HDMI transmitter interface   */
/*************************************************/


/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                 I2C FUNCTIONS                 */
/*************************************************/


static int hdmi_read( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint16_t offset )
{
    int             ret  = 0;
    uint8_t         data = 0;

    ret = i2c_read( vpout, vpout_draw, port, vpout->hdmi[port].device.it66121.bus, vpout->hdmi[port].device.it66121.speed, vpout->hdmi[port].device.it66121.address,
                    offset, &data, 1 );

    if ( ret < 0 )
        return ret;

    return data;
}


static int hdmi_write_masked( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t offset, uint8_t value, uint8_t mask )
{
    int             ret = 0;

    ret = hdmi_read( vpout, vpout_draw, port, offset );

    if ( ret < 0 )
        return ret;

    value = (value & mask) | ((uint8_t)ret & ~mask);

    ret = i2c_write( vpout, vpout_draw, port, vpout->hdmi[port].device.it66121.bus, vpout->hdmi[port].device.it66121.speed, vpout->hdmi[port].device.it66121.address,
                     offset, &value, 1 );

    return (ret < 0) ? ret : 0;
}


/*************************************************/
/*              INTERFACE FUNCTIONS              */
/*************************************************/


void it66121_probe( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index )
{
    disp_adapter_t  *adapter       = vpout->adapter;
    uint32_t        gpio_data      = 0;
    uint32_t        gpio_direction = 0;
    uint32_t        gpio_control   = 0;

    if ( i2c_init( vpout, vpout_draw, vpout->hdmi[port_index].device.it66121.bus, port_index ) )
    {
        disp_printf( adapter, "[vpoutfb: it66121] Fatal: I2C HDMI controller initialization failed [I2C issue: port=%d]", port_index );
        return;
    }

    /* Initial HDMI chip GPIO port value */
    switch ( vpout->hdmi[port_index].device.it66121.reg )
    {
        case 0xa:
            gpio_data      = GPIO_SWPORTA_DR;
            gpio_direction = GPIO_SWPORTA_DDR;
            gpio_control   = GPIO_SWPORTA_CTL;
            break;

        case 0xb:
            gpio_data      = GPIO_SWPORTB_DR;
            gpio_direction = GPIO_SWPORTB_DDR;
            gpio_control   = GPIO_SWPORTB_CTL;
            break;

        case 0xc:
            gpio_data      = GPIO_SWPORTC_DR;
            gpio_direction = GPIO_SWPORTC_DDR;
            gpio_control   = GPIO_SWPORTC_CTL;
            break;

        case 0xd:
            gpio_data      = GPIO_SWPORTD_DR;
            gpio_direction = GPIO_SWPORTD_DDR;
            gpio_control   = GPIO_SWPORTD_CTL;
            break;

    }

    /* Switch to the software control */
    *GPIO_MMIO32( vpout->hdmi[port_index].device.it66121.registers, gpio_control )   &= ~(1 << vpout->hdmi[port_index].device.it66121.pin);
    /* Switch to the output */
    *GPIO_MMIO32( vpout->hdmi[port_index].device.it66121.registers, gpio_direction ) |= 1 << vpout->hdmi[port_index].device.it66121.pin;

    *GPIO_MMIO32( vpout->hdmi[port_index].device.it66121.registers, gpio_data )      |= 1 << vpout->hdmi[port_index].device.it66121.pin;
}


void it66121_reset( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index )
{
    uint32_t        gpio_data = 0;

    switch ( vpout->hdmi[port_index].device.it66121.reg )
    {
        case 0xa:
            gpio_data = GPIO_SWPORTA_DR;
            break;

        case 0xb:
            gpio_data = GPIO_SWPORTB_DR;
            break;

        case 0xc:
            gpio_data = GPIO_SWPORTC_DR;
            break;

        case 0xd:
            gpio_data = GPIO_SWPORTD_DR;
            break;

    }

    *GPIO_MMIO32( vpout->hdmi[port_index].device.it66121.registers, gpio_data ) &= ~(1 << vpout->hdmi[port_index].device.it66121.pin);
    /* Takes a pause or reset won't take effect */
    disp_usecspin( 1000 );
    *GPIO_MMIO32( vpout->hdmi[port_index].device.it66121.registers, gpio_data ) |= 1 << vpout->hdmi[port_index].device.it66121.pin;
}


void it66121_remove( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index )
{
    i2c_fini( vpout, vpout_draw, port_index );
}


int it66121_init( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index /* ignored */, uint32_t pixel_clock )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint8_t         ident[]  = { 0x54, 0x49, 0x12, 0x16 };
    int             ret      = 0, i;

    it66121_reset( vpout, vpout_draw, port_index );
    disp_usecspin( 5000 );

    /* Verify it's the correct device */
    for ( i = 0; i < 4; i++ )
    {
        if ( ident[i] != hdmi_read( vpout, vpout_draw, port_index, i ) )
        {
            disp_printf( adapter, "[vpoutfb: it66121] Fatal: ITE IT66121 I2C HDMI controller not found [port=%d]", port_index );
            return (-ENODEV);
        }
    }

    /* reset whole circuit */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_RESET, 0x20, 0x20 );
    disp_usecspin( 5000 );

    /* each circuit is off so we can switch them on one by one */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_RESET, RESET_RCLK_MASK | RESET_AUDIO_MASK | RESET_VIDEO_MASK | RESET_AFIFO_MASK |
                                                             RESET_HDCP_MASK, 0xFF );
    disp_usecspin( 5000 );

    /* hdmi tx flipflops reset */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_TX_RESET, 0x10, 0xFF );

    /* DVI mode, packet interface off */
    for ( i = 0xc0; i <= 0xd0; i++ )
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, i, 0, 0xFF );

    /* Ignore all the interrupts */
    for ( i = REG_IRQ_FIRST; i <= REG_IRQ_LAST; i++ )
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, i, 0xFF, 0xFF );

    /* Enable video circuit */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_RESET, 0x00, RESET_VIDEO_MASK );
    /* Switch avmute on */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_AVMUTE, 0x01, 0xFF );

    /* Disable audio */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, 0xE0, 0x00, 0x0F );

    /* Set up clock-related settings */
    if ( (float)pixel_clock >= (float)12500.0 )
    {
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_CLK1, 0x18, 0xFF );
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_CLK2, 0x10, 0xFF );
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_CLK3, 0x0C, 0xFF );
    } else {
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_CLK1, 0x88, 0xFF );
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_CLK2, 0x10, 0xFF );
        ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_CLK3, 0x84, 0xFF );
    }
    /* Clear TX FIFO */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_TXFIFO_SET, 0x02, 0x02 );
    disp_usecspin( 2000 );

    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_TXFIFO_SET, 0x00, 0x02 );
    /* It takes a while to measure clocks, so we give it time */
    disp_usecspin( 150000 );

    for ( i = 0; i < 10; i++ )
    {
        if ( !(hdmi_read( vpout, vpout_draw, port_index, 0x0e ) & 0x10) )
        {
            disp_printf( adapter, "[vpoutfb: it66121] Warning: transmitter video input not stable [port=%d]", port_index );
            disp_usecspin( 20000 );
        } else
            break;
    }
    if ( i == 10 )
    {
        it66121_reset( vpout, vpout_draw, port_index );
        return (-EBUSY);
    }
    /* Switch avmute off */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, REG_AVMUTE, 0x00, 0xFF );
    /* Turn on display */
    ret |= hdmi_write_masked( vpout, vpout_draw, port_index, 0x61, 0x03, 0xFF );
    if ( ret )
        it66121_reset( vpout, vpout_draw, port_index );

    return ret;
}
