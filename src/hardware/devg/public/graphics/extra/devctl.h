#ifndef _GRAPHICS_EXTRA_DEVCTL_H_INCLUDED
#define _GRAPHICS_EXTRA_DEVCTL_H_INCLUDED


/* disp_modefuncs_t::devctl() commands */

#define DEVCTL_DDC                              0x1000              /* driver side 'ddc' utility support */


/* disp_modefuncs_t::devctl() data types */

typedef struct devctl_ddc
{
    #define DEVCTL_DDC_MODE_DISPLAY             0x0
    #define DEVCTL_DDC_MODE_BUS                 0x1
    uint8_t     mode;

    uint8_t     bus;
} devctl_ddc_t;
typedef devctl_ddc_t    devctl_ddc_request_t;


#endif /* _GRAPHICS_EXTRA_DEVCTL_H_INCLUDED */

