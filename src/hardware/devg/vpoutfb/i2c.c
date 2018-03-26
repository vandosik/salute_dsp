/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"
#include <hw/i2c.h>


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


static int          bus[VPOUT_HDMI_PORTS];
static int          bus_en[VPOUT_HDMI_PORTS];


int i2c_read( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t bus_, uint32_t speed_, uint8_t addr, unsigned short offset,
              uint8_t *buf, unsigned count )
{
    disp_adapter_t  *adapter = vpout->adapter;
    int             status   = 0;
    uint8_t         buf_[sizeof(i2c_send_t) + 256];
    i2c_send_t      *hdr     = (i2c_send_t *)buf_;
    int             rbytes;
    uint32_t        speed    = speed_ * 1000;

    if ( bus_en[port] != 1 )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus read failed [bus not found: bus=#%u, port=%d]", bus_, port );
        return (-1);
    }

    if ( devctl( bus[port], DCMD_I2C_SET_BUS_SPEED, &speed, sizeof( speed ), NULL ) )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus read failed [can't set bus speed: bus=#%u, port=%d]", bus_, port );
        return (-1);
    }

#if 0
    memset( buf_, 0, sizeof(i2c_send_t) + 1 );

    hdr->slave.addr       = addr;
    hdr->slave.fmt        = I2C_ADDRFMT_7BIT;
    hdr->len              = 1;
    hdr->stop             = 0;
    buf_[sizeof( i2c_send_t )] = offset;
    status = devctl( bus[port], DCMD_I2C_SEND, buf_, sizeof( i2c_send_t ) + 1, NULL );
    if ( status )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus read failed [transaction failed on write address: bus=#%u, addr=0x%x, port=%d]", bus_, addr, port );
        return (-1);
    }
#endif

    memset( buf_, 0, sizeof(i2c_send_t) + count );

    hdr->slave.addr       = addr;
    hdr->slave.fmt        = I2C_ADDRFMT_7BIT;
    hdr->len              = count;
    hdr->stop             = 0;
    buf_[sizeof( i2c_send_t )] = offset;
    status = devctl( bus[port], DCMD_I2C_RECV, buf_, sizeof( i2c_send_t ) + count, &rbytes );
    if ( status )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus read failed [transaction failed on read: bus=#%u, addr=0x%x, port=%d]", bus_, addr, port );
        return (-1);
    }

    memcpy( buf, &buf_[sizeof( i2c_send_t )], count );

    return (0);
}


int i2c_write( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t bus_, uint32_t speed_, uint8_t addr, unsigned short offset,
               uint8_t *buf, unsigned count )
{
    disp_adapter_t  *adapter = vpout->adapter;
    int             status   = 0;
    uint8_t         buf_[sizeof(i2c_send_t) + 256];
    i2c_send_t      *hdr     = (i2c_send_t *)buf_;
    uint32_t        speed    = speed_ * 1000;

    if ( bus_en[port] != 1 )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus writing failed [bus not found: bus=#%u, port=%d]", bus_, port );
        return (-1);
    }

    if ( devctl( bus[port], DCMD_I2C_SET_BUS_SPEED, &speed, sizeof( speed ), NULL ) )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus read failed [can't set bus speed: bus=#%u, port=%d]", bus_, port );
        return (-1);
    }

#if 0
    memset( buf_, 0, sizeof(i2c_send_t) + 1 );

    hdr->slave.addr       = addr;
    hdr->slave.fmt        = I2C_ADDRFMT_7BIT;
    hdr->len              = 1;
    hdr->stop             = 0;
    buf_[sizeof( i2c_send_t )] = offset;
    status = devctl( bus[port], DCMD_I2C_SEND, buf_, sizeof( i2c_send_t ) + 1, NULL );
    if ( status )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus read failed [transaction failed on write address: bus=#%u, addr=0x%x, port=%d]", bus_, addr, port );
        return (-1);
    }
#endif

    memset( buf_, 0, sizeof(i2c_send_t) + count );
    memcpy( &buf_[sizeof( i2c_send_t ) + 1], buf, count );

    hdr->slave.addr       = addr;
    hdr->slave.fmt        = I2C_ADDRFMT_7BIT;
    hdr->len              = count + 1;
    hdr->stop             = 0;
    buf_[sizeof( i2c_send_t )] = offset;
    status = devctl( bus[port], DCMD_I2C_SEND, buf_, sizeof( i2c_send_t ) + count + 1, NULL );
    if ( status )
    {
        disp_printf( adapter, "[vpout: i2c] Error: I2C bus write failed [transaction failed on write: bus=#%u, addr=0x%x, port=%d]", bus_, addr, port );
        return (-1);
    }

    return (0);
}


int i2c_init( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t bus_, uint8_t port )
{
    disp_adapter_t  *adapter = vpout->adapter;
    char            path[16];

    sprintf( path, "/dev/i2c%u", bus_ );

    bus[port] = open( path, O_RDWR );
    if ( bus[port] < 0 )
    {
        disp_printf( adapter, "[vpout: i2c] Error: can't open bus [bus=#%u, port=%d]", bus_, port );
        return (-1);
    }
    bus_en[port] = 1;

    return (0);
}


void i2c_fini( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port )
{
    if ( bus_en[port] )
    {
        close( bus_en[port] );
        bus_en[port] = 0;
    }
}
