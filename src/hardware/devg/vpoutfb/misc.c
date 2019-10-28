/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


int vpout_misc_wait_idle( disp_adapter_t *adapter )
{
    return 0;
}


int vpout_draw_init( disp_adapter_t *adapter, char *opt )
{
    return 0;
}


void vpout_draw_fini( disp_adapter_t *adapter )
{

    return;
}


void vpout_module_info( disp_adapter_t *adapter, disp_module_info_t *info )
{
    info->description       = "vpoutfb - Elvees 1892VM14YA ARMv7 SoC";
    info->ddk_version_major = DDK_VERSION_MAJOR;
    info->ddk_version_minor = DDK_VERSION_MINOR;
    info->ddk_rev           = DDK_REVISION;
    info->driver_rev        = 0;
}


/* Set up things so that miscfuncs, corefuncs and contextfuncs can be called by an external process. */
int vpout_attach_external( disp_adapter_t *adapter, disp_aperture_t aper[] )
{
    vpout_context_t         *vpout      = adapter->shmem;
    vpout_draw_context_t    *vpout_draw = NULL;

    /* Allocate external process local draw-context */
    vpout_draw = (vpout_draw_context_t *)calloc( 1, sizeof( vpout_draw_context_t ) );
    if ( vpout_draw == NULL )
        return (-1);

    /* Assign GPU context and draw-context */
    vpout_draw->vpout = vpout;
    adapter->ms_ctx   = vpout;
    adapter->gd_ctx   = vpout_draw;
    adapter->caps     = DISP_CAP_NO_IO_PRIVITY;

    /* Assign remmaped pointers */
    vpout_draw->registers = aper[0].vaddr;

    vpout_draw->external_process = 1;

    return 0;
}


int vpout_detach_external( disp_adapter_t *adapter )
{
    vpout_draw_context_t  *vpout_draw = adapter->gd_ctx;

    free( vpout_draw );
    adapter->gd_ctx = NULL;

    return 0;
}

/* Populate the miscellaneuous graphics driver function table. tabsize is the size of the function table in bytes */
int devg_get_miscfuncs( disp_adapter_t *adapter, disp_draw_miscfuncs_t *funcs, int tabsize )
{
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, init,                  vpout_draw_init,        tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, fini,                  vpout_draw_fini,        tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, module_info,           vpout_module_info,      tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, get_2d_caps,           ffb_get_2d_caps,        tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, get_corefuncs_sw,      ffb_get_corefuncs,      tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, get_contextfuncs_sw,   ffb_get_contextfuncs,   tabsize );

#ifdef __QNXNTO__
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, wait_idle,             vpout_misc_wait_idle,   tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, attach_external,       vpout_attach_external,  tabsize );
    DISP_ADD_FUNC( disp_draw_miscfuncs_t, funcs, detach_external,       vpout_detach_external,  tabsize );
#endif

    return 0;
}
