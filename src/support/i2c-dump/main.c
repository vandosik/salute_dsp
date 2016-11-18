/*
#ifdef __USAGE
Syntax:
%C [-d name] [-s addr] [-r reg]

Options:
-d name     I2C device name (default: /dev/i2c0)
-s addr     I2C address
-r reg      Device register offset
#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <devctl.h>
#include <fcntl.h>
#include <hw/i2c.h>


char        *devname = "/dev/i2c0";
unsigned    bus_speed = 100000;
uint8_t     addr = 0;
uint8_t     reg = 0;
int         fd = 0;

void init( int argc, char *argv[] )
{
    int                 c;

    while (-1 != (c = getopt(argc, argv, "d:s:r:"))) {
        switch (c) {
            case 'd':
                devname = optarg;
                break;
            case 's':
                addr = strtol( optarg, NULL, 0 );
                break;
            case 'r':
                reg = strtol( optarg, NULL, 0 );
                break;
        }
    }
}

int i2c_read( uint8_t addr, uint8_t reg, uint8_t *data )
{
    int             status;
    unsigned char   buf[sizeof(i2c_send_t) + 1];
    i2c_send_t      *pshdr = (i2c_send_t *)buf;
    int             rbytes;

    memset( buf, 0, sizeof(i2c_send_t) + 1 );

    pshdr->slave.addr       = addr;
    pshdr->slave.fmt        = I2C_ADDRFMT_7BIT;
    pshdr->len              = 1;
    pshdr->stop             = 0;
    buf[sizeof( i2c_send_t )] = reg;
    status = devctl( fd, DCMD_I2C_RECV, buf, sizeof( i2c_send_t ) + 1, &rbytes );
    if ( status ) {
        printf( "I2C read failed - fd error on read\n" );
        return (-1);
    }

    printf( "Reading %s: addr=0x%x reg=0x%x val=0x%x\n", devname, addr, reg, buf[sizeof( i2c_send_t )] );
    *data = buf[sizeof( i2c_send_t )];

    return (0);
}


int main( int argc, char *argv[] )
{
    init( argc, argv );
    uint8_t data;

    fd = open(devname, O_RDWR);
    if ( fd < 0 )
    {
        fprintf(stderr, "open(%s): %s\n", devname, strerror(errno));
        exit(-1);
    }

#if 0
    if ( devctl( fd, DCMD_I2C_SET_BUS_SPEED, &bus_speed, sizeof(bus_speed), NULL ) ) {
        printf( "DCMD_I2C_SET_BUS_SPEED error\n" );
        exit(-1);
    }
#endif

    i2c_read( addr, reg, &data );

    return (0);
}
