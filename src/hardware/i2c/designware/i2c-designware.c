/* i2c-designware.c
 *
 * I2C bus driver for Synopsys DesignWare (TI DaVinci) adapters
 *
 *  Created on: 14.10.2016
 *      Author: SWD Embedded Systems Ltd. */


#include "designware.h"


static int          verbosity = 0;


i2c_status_t designware_i2c_send( void *handler, void *buf, uint32_t len, uint32_t stop )
{
    designware_i2c_dev_t    *dev = handler;
    int                     rval = I2C_STATUS_DONE;

    i2c_printf( _SLOG_DEBUG2, "Debug: [%s() call]", __FUNCTION__ );

    if ( len <= 0 )
        return I2C_STATUS_DONE;

    pthread_mutex_lock( &dev->lock );

    /* Send data */
    if ( designware_i2c_write( dev, dev->slave_addr.addr, 0 /* offset */, 0 /* offset length */, (uint8_t *)buf, len ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: I2C send operation failed [%s()]", __FUNCTION__ );
        rval = I2C_STATUS_ERROR;
    }

    pthread_mutex_unlock( &dev->lock );

    return rval;
}


i2c_status_t designware_i2c_recv( void *handler, void *buf, uint32_t len, uint32_t stop )
{
    designware_i2c_dev_t    *dev = handler;
    int                     rval = I2C_STATUS_DONE;

    i2c_printf( _SLOG_DEBUG2, "Debug: [%s() call]", __FUNCTION__ );

    if ( len <= 0 )
        return I2C_STATUS_DONE;

    pthread_mutex_lock( &dev->lock );

    /* Receive data */
    if ( designware_i2c_read( dev, dev->slave_addr.addr, 0 /* offset */, 0 /* offset length */, (uint8_t *)buf, len ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: I2C receive operation failed [%s()]", __FUNCTION__ );
        rval = I2C_STATUS_ERROR;
    }

    pthread_mutex_unlock( &dev->lock );

    return rval;
}


int designware_i2c_set_slave_addr( void *handler, uint32_t addr, i2c_addrfmt_t fmt )
{
    designware_i2c_dev_t    *dev = handler;

    i2c_printf( _SLOG_DEBUG2, "Debug: [%s() call: address=0x%x]", __FUNCTION__, addr );

    if ( fmt != I2C_ADDRFMT_7BIT /* && fmt != I2C_ADDRFMT_10BIT need fixes in the designware.c */ )
    {
        i2c_printf( _SLOG_ERROR, "Error: invalid address format [%s()]", __FUNCTION__, addr );

        errno = EINVAL;
        return (-1);
    }

    i2c_printf( _SLOG_INFO, "Info: slave address setup [%s(): address=0x%x]", __FUNCTION__, addr );
    dev->slave_addr.addr = addr;
    dev->slave_addr.fmt  = fmt;

    return (0);
}


int designware_i2c_set_bus_speed_( void *handler, uint32_t speed, uint32_t *ospeed )
{
    designware_i2c_dev_t    *dev = handler;
    i2c_speed_mode_t        speed_mode;

    i2c_printf( _SLOG_DEBUG2, "Debug: [%s() call: speed=%d]", __FUNCTION__, speed );

    if ( speed > 400000 )
        speed_mode = I2C_SPEED_HIGH;
    if ( speed > 100000 )
        speed_mode = I2C_SPEED_FAST;
    else
        speed_mode = I2C_SPEED_STANDARD;

    if ( ospeed )
        *ospeed = dev->speed;

    if ( speed == dev->speed )
    {
        return (0);
    }

    designware_i2c_set_bus_speed( dev, speed_mode );

    return (0);
}


int designware_i2c_version_info( i2c_libversion_t *version )
{
    version->major    = I2CLIB_VERSION_MAJOR;
    version->minor    = I2CLIB_VERSION_MINOR;
    version->revision = I2CLIB_REVISION;

    return (0);
}


int designware_i2c_driver_info( void *handler, i2c_driver_info_t *info )
{
    info->speed_mode = I2C_SPEED_STANDARD | I2C_SPEED_FAST | I2C_SPEED_HIGH;
    info->addr_mode  = I2C_ADDRFMT_7BIT | I2C_ADDRFMT_10BIT;

    return (0);
}


int i2c_printf( int level, const char *fmt, ... )
{
    int             status = 0;
    va_list         arg;
    char            format[255];

    sprintf( format, "[i2c-designware] %s", fmt );

    if ( level <= verbosity || level == _SLOG_ERROR )
    {
        va_start( arg, fmt );

        status = vslogf( _SLOG_SETCODE( _SLOGC_CHAR, 0 ), level, format, arg );
        if ( verbosity > 7 )
        {
            status = vfprintf( stderr, format, arg );
            status += fprintf( stderr, "\n" );
        }

        va_end( arg );
    }

    return status;
}


int designware_i2c_parseopts( designware_i2c_dev_t *dev, int argc, char *argv[] )
{
    int             c;
    int             prev_optind;
    int             done = 0;

    dev->physbase  = 0;
    dev->speed     = 0;
    dev->frequency = IC_CLK;

    while ( !done )
    {
        prev_optind = optind;

        c = getopt( argc, argv, "f:p:v" );

        switch ( c )
        {
            case 'f':
                dev->frequency = strtoul( optarg, NULL, 0 );
                break;

            case 'p':
                dev->physbase = strtoul( optarg, NULL, 0 );
                break;

            case 'v':
                verbosity++;
                break;

            case '?':
                if ( optopt == '-' )
                {
                    ++optind;
                    break;
                }
                return (-1);

            case -1:
                if ( prev_optind < optind ) /* -- */
                    return (-1);

                if ( argv[optind] == NULL )
                {
                    done = 1;
                    break;
                }

                if ( *argv[optind] != '-' )
                {
                    ++optind;
                    break;
                }

                return (-1);

            case ':':
            default:
                return (-1);
        }
    }

    if ( dev->physbase == 0 )
    {
        i2c_printf( _SLOG_ERROR, "Error: invalid I2C base address [%s()]", __FUNCTION__ );
        return (-1);
    }

    return (0);
}


void *designware_i2c_init_( int argc, char *argv[] )
{
    designware_i2c_dev_t    *dev;

    i2c_printf( _SLOG_DEBUG2, "Debug: [%s() call]", __FUNCTION__ );

    if ( ThreadCtl( _NTO_TCTL_IO, 0 ) == -1 )
    {
        i2c_printf( _SLOG_ERROR, "Error: ThreadCtl() call failed [%s()]", __FUNCTION__ );
        return (NULL);
    }

    dev = malloc( sizeof( designware_i2c_dev_t ) );
    if ( !dev )
    {
        i2c_printf( _SLOG_ERROR, "Error: device descriptor allocation failed [%s()]", __FUNCTION__ );
        return (NULL);
    }

    if ( designware_i2c_parseopts( dev, argc, argv ) == -1 )
        goto fail;

    i2c_printf( _SLOG_INFO, "Info: mapping I2C I/O range (base=0x%x)... [%s()]", dev->physbase, __FUNCTION__ );
    dev->registers = (designware_i2c_regs_t *)mmap_device_memory( NULL, DESIGNWARE_I2C_REGLEN, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, dev->physbase );
    if ( dev->registers == (designware_i2c_regs_t *)MAP_FAILED )
    {
        i2c_printf( _SLOG_ERROR, "Error: unable to mmap physical base address [%s()]", __FUNCTION__ );
        goto fail;
    }

    if ( synopsys_i2c_init( dev ) )
    {
        i2c_printf( _SLOG_ERROR, "Error: unable to read Synopsys registers [%s()]", __FUNCTION__ );
        pthread_mutex_destroy( &dev->lock );
        goto fail1;
    }

    if ( pthread_mutex_init( &dev->lock, NULL ) == -1 )
    {
        i2c_printf( _SLOG_ERROR, "Error: unable to initialize lock [%s()]", __FUNCTION__ );
        pthread_mutex_destroy( &dev->lock );
        goto fail1;
    }

    designware_i2c_reset( dev );
    i2c_printf( _SLOG_INFO, "Info: controller reset done [%s()]", __FUNCTION__ );
    designware_i2c_init( dev, I2C_SPEED_STANDARD, 0x0 );

    return dev;

fail1:
    if ( dev->registers )
        munmap_device_memory( dev->registers, DESIGNWARE_I2C_REGLEN );
fail:
    free( dev );

    return (NULL);
}


void designware_i2c_fini( void *handler )
{
    designware_i2c_dev_t    *dev = handler;

    i2c_printf( _SLOG_DEBUG2, "Debug: [%s() call]", __FUNCTION__ );

    designware_i2c_reset( dev );

    if ( dev->registers )
        munmap_device_memory( dev->registers, DESIGNWARE_I2C_REGLEN );

    pthread_mutex_destroy( &dev->lock );

    free( dev );
}


/* Callbacks for libi2c-master library */
int i2c_master_getfuncs( i2c_master_funcs_t *funcs, int tabsize )
{
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, init,           designware_i2c_init_,          tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, fini,           designware_i2c_fini,           tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, send,           designware_i2c_send,           tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, recv,           designware_i2c_recv,           tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, set_slave_addr, designware_i2c_set_slave_addr, tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, set_bus_speed,  designware_i2c_set_bus_speed_, tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, version_info,   designware_i2c_version_info,   tabsize );
    I2C_ADD_FUNC( i2c_master_funcs_t, funcs, driver_info,    designware_i2c_driver_info,    tabsize );

    return (0);
}
