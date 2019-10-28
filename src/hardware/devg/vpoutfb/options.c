/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                  VARS / DEFS                  */
/*************************************************/


static char *vpout_opts[] = {
#define VPOUT_OPT_DISPMODE              0
    "dispmode",
#define VPOUT_OPT_BASE                  1
    "base",
#define VPOUT_OPT_SIZE                  2
    "size",
#define VPOUT_OPT_IRQ                   3
    "irq",
#define VPOUT_OPT_HDMI                  4
    "hdmi",
#define VPOUT_OPT_ENABLE                5
    "enable",
#define VPOUT_OPT_VERBOSE               6
    "verbose",
    NULL
};

int verbose = VPOUT_SLOG_LEVEL_0;


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


int parse_options( disp_adapter_t *adapter, vpout_context_t *vpout, char *filename )
{
    FILE                    *fin     = NULL;
    char                    buf[256],
                            *c       = NULL,
                            *opt     = NULL,
                            *value   = NULL;
    char                    path[255];
    uint32_t                tmp      = 0;

    if ( (filename == NULL) || ( strlen( filename ) == 0 ) )
    {
        int                 fd       = 0;

        strcpy( path, "/etc/system/config/vpoutfb.conf" );
        fd = open( path, O_RDONLY );
        if ( fd == -1 )
        {
#if 0
            strcpy( path, "/usr/photon/config/vpoutfb.conf" );
            fd = open( path, O_RDONLY );
            if ( fd != -1 )
            {
                close( fd );

                filename = path;
            }
#endif
        } else {
            close( fd );

            filename = path;
        }
    }

    if ( (filename == NULL) || ( strlen( filename ) == 0 ) ) {
        /* Fatal: configuration file not found */
    } else if ( (fin = fopen( filename, "r" )) == NULL )
        disp_printf( adapter, "[vpoutfb] Critical: could not open config file \"%s\": %s", filename, strerror( errno ) );
    else
        disp_printf( adapter, "[vpoutfb] Configuration: \"%s\"", filename );

    vpout->display[0]     = CONFIG_DISPLAY_PORT_HDMI0;
    vpout->enabled        = 0;
    vpout->registers_size = 0x1000;

    if ( fin == NULL )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: configuration file not found" );

        return (-1);
    }

    while ( fgets( buf, sizeof (buf), fin ) != NULL )
    {
        c = buf;
        while ( *c == ' ' || *c == '\t' )
            c++;
        if ( *c == '\015' || *c== '\032' || *c == '\0' || *c == '\n' || *c == '#' )
            continue;
        opt = c;
        while ( *c == '\015' || *c== '\032' || (*c != '\0' && *c != '\n' && *c != '#') )
            c++;
        *c = '\0';
        break;
    }

    while ( *opt != '\0' )
    {
        c = opt;
        switch ( getsubopt( &opt, vpout_opts, &value ) )
        {
            case VPOUT_OPT_DISPMODE:
                if ( strcmp( value, "hdmi-0" ) == 0 ) {
                    vpout->display[0]     = CONFIG_DISPLAY_PORT_HDMI0;
                    disp_printf( adapter, "[vpoutfb] Configuration: display[0] -> HDMI-0" );
                } else
                    disp_printf( adapter, "[vpoutfb] Warning: unknown display configuration (see vpoutfb.conf)" );
                break;

            case VPOUT_OPT_BASE:
                vpout->registers_base = strtoul( value, NULL, 0 );
                disp_printf( adapter, "[vpoutfb] Configuration: display controller registers base - 0x%08x", vpout->registers_base );
                break;

            case VPOUT_OPT_SIZE:
                vpout->registers_size = strtoul( value, NULL, 0 );
                disp_printf( adapter, "[vpoutfb] Configuration: display controller registers size - 0x%x", vpout->registers_size );
                break;

            case VPOUT_OPT_IRQ:
                vpout->irq = strtoul( value, NULL, 0 );
                disp_printf( adapter, "[vpoutfb] Configuration: display controller interrupt - %d", vpout->irq );
                break;

            case VPOUT_OPT_HDMI:
                if ( strncmp( value, VPOUT_OPT_HDMI_IT66121":", strlen( VPOUT_OPT_HDMI_IT66121 ) + 1 ) == 0 )
                {
                    char    *name = value;
                    int     bus;
                    int     address;
                    int     speed;
                    int     base;
                    int     reg;
                    int     pin;

                    value += strlen( VPOUT_OPT_HDMI_IT66121 );
                    *value= 0;
                    value++;

                    if ( sscanf( value, "%d:0x%x:%d:0x%x:0x%x:%d", &bus, &address, &speed, &base, &reg, &pin ) == 6 )
                    {
                        if ( bus < 0 || bus > 3 )
                        {
                            disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter I2C bus invalid (see vpoutfb.conf)" );
                            break;
                        }

                        if ( address < 0 || address > 0xffff )
                        {
                            disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter I2C address invalid (see vpoutfb.conf)" );
                            break;
                        }

                        if ( speed <= 0 )
                        {
                            disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter I2C speed invalid (see vpoutfb.conf)" );
                            break;
                        }

                        if ( base <= 0 )
                        {
                            disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter GPIO base invalid (see vpoutfb.conf)" );
                            break;
                        }

                        if ( reg < 0xa || reg > 0xd )
                        {
                            disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter GPIO register invalid (see vpoutfb.conf: [0xa; 0xd] expected)" );
                            break;
                        }

                        if ( vpout->hdmi_count >= VPOUT_HDMI_PORTS )
                        {
                            disp_printf( adapter, "[vpoutfb] Error: only %d HDMI ports supported", VPOUT_HDMI_PORTS );
                            break;
                        }

                        vpout->hdmi[vpout->hdmi_count].assigned               = true;
                        strcpy( vpout->hdmi[vpout->hdmi_count].transmitter, name );
                        vpout->hdmi[vpout->hdmi_count].device.it66121.bus     = bus;
                        vpout->hdmi[vpout->hdmi_count].device.it66121.address = address;
                        vpout->hdmi[vpout->hdmi_count].device.it66121.speed   = speed;
                        vpout->hdmi[vpout->hdmi_count].device.it66121.base    = base;
                        vpout->hdmi[vpout->hdmi_count].device.it66121.reg     = reg;
                        vpout->hdmi[vpout->hdmi_count].device.it66121.pin     = pin;
                        vpout->hdmi_count++;

                        disp_printf( adapter, "[vpoutfb] Configuration: HDMI[%d] - controller:%s I2C:bus=%d,address=0x%x,speed=%d GPIO:base=0x%x,reg=GPIO%X,pin=%d",
                                     vpout->hdmi_count - 1, name, bus, address, speed * 1000, base, reg, pin );
                        break;
                    }
                } else
                    if ( strncmp( value, VPOUT_OPT_HDMI_TDA998x":", strlen( VPOUT_OPT_HDMI_TDA998x ) + 1 ) == 0 )
                    {
                        char    *name = value;
                        int     bus;
                        int     address;
                        int     speed;
                        int     video_ports = 0;

                        value += strlen( VPOUT_OPT_HDMI_TDA998x );
                        *value= 0;
                        value++;

                        if ( sscanf( value, "%d:0x%x:%d:0x%x", &bus, &address, &speed, &video_ports ) == 4 )
                        {
                            if ( bus < 0 || bus > 3 )
                            {
                                disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter I2C bus invalid (see vpoutfb.conf)" );
                                break;
                            }

                            if ( address < 0 || address > 0xffff )
                            {
                                disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter I2C address invalid (see vpoutfb.conf)" );
                                break;
                            }

                            if ( speed <= 0 )
                            {
                                disp_printf( adapter, "[vpoutfb] Error: HDMI transmitter I2C speed invalid (see vpoutfb.conf)" );
                                break;
                            }

                            if ( vpout->hdmi_count >= VPOUT_HDMI_PORTS )
                            {
                                disp_printf( adapter, "[vpoutfb] Error: only %d HDMI ports supported", VPOUT_HDMI_PORTS );
                                break;
                            }

                            vpout->hdmi[vpout->hdmi_count].assigned                   = true;
                            strcpy( vpout->hdmi[vpout->hdmi_count].transmitter, name );
                            vpout->hdmi[vpout->hdmi_count].device.tda998x.bus         = bus;
                            vpout->hdmi[vpout->hdmi_count].device.tda998x.address     = address;
                            vpout->hdmi[vpout->hdmi_count].device.tda998x.speed       = speed;
                            vpout->hdmi[vpout->hdmi_count].device.tda998x.video_ports = video_ports;
                            vpout->hdmi_count++;

                            disp_printf( adapter, "[vpoutfb] Configuration: HDMI[%d] - controller:%s I2C:bus=%d,address=0x%x,speed=%d",
                                         vpout->hdmi_count - 1, name, bus, address, speed * 1000 );
                            break;
                        }
                    }

                disp_printf( adapter, "[vpoutfb] Warning: unknown HDMI transmitter configuration (see vpoutfb.conf)" );
                break;

            case VPOUT_OPT_ENABLE:
                if ( strstr( value, "lcd_sync_fix" ) != NULL ) {
                    disp_printf( adapter, "[vpoutfb] Configuration: LCD sync generation fix enabled" );
                    vpout->enabled |= CONFIG_ENABLE_LCD_SYNC_FIX;
                }
                break;

            case VPOUT_OPT_VERBOSE:
                if ( strstr( value, "silent" ) != NULL ) {
                    disp_printf( adapter, "[vpoutfb] Configuration: \"silent\" output level" );
                    verbose = VPOUT_SLOG_LEVEL_0;
                } else if ( strstr( value, "warn" ) != NULL ) {
                    disp_printf( adapter, "[vpoutfb] Configuration: \"warn\" output level" );
                    verbose = VPOUT_SLOG_LEVEL_WARNING;
                } else if ( strstr( value, "info" ) != NULL ) {
                    disp_printf( adapter, "[vpoutfb] Configuration: \"info\" output level" );
                    verbose = VPOUT_SLOG_LEVEL_INFO;
                } else if ( strstr( value, "debug" ) != NULL ) {
                    disp_printf( adapter, "[vpoutfb] Configuration: \"debug\" output level" );
                    verbose = VPOUT_SLOG_LEVEL_DEBUG;
                } else
                    disp_printf( adapter, "[vpoutfb] Warning: unknown verbosity level (see vpoutfb.conf)" );
                break;

            default:
                disp_printf( adapter, "[vpoutfb] Unknown option %s", c );
                break;
        }
    }

    fclose( fin );

    /* Error detection */
    #define CONFIG_ERROR( text )    { disp_printf( adapter, text ); return (-1); }
        /* base option */
        if ( vpout->registers_base == 0 )
            CONFIG_ERROR( "[vpoutfb] Fatal: unknown display controller registers base phys-address (see vpoutfb.conf)" );
        /* size option */
        if ( vpout->registers_size == 0 )
            disp_printf( adapter, "[vpoutfb] Warning: using display controller default registers size (see vpoutfb.conf)" );
        /* irq option */
        if ( vpout->irq == 0 )
            disp_printf( adapter, "[vpoutfb] Warning: unknown display controller interrupt (see vpoutfb.conf)" );
        /* dispmode option */
        switch ( vpout->display[0] )
        {
            case CONFIG_DISPLAY_PORT_HDMI0:
                if ( !vpout->hdmi[DISPLAY_PORT_INDEX( CONFIG_DISPLAY_PORT_HDMI0 )].assigned )
                    CONFIG_ERROR( "[vpoutfb] Fatal: damaged display controller configuration (see vpoutfb.conf::hdmi option)" );
                break;
        }

    return (0);
}
