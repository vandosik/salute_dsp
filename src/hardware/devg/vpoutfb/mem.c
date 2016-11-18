/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


int devg_get_memfuncs( disp_adapter_t *adapter, disp_memfuncs_t *funcs, int tabsize )
{
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, init,                 vpout_mem_init,                tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, fini,                 vpout_mem_fini,                tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, module_info,          vpout_module_info,             tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, reset,                vpout_mem_reset,               tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, alloc_surface,        vpout_alloc_surface,           tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, free_surface,         vpout_free_surface,            tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, mem_avail,            vpout_mem_avail,               tabsize );

    DISP_ADD_FUNC( disp_memfuncs_t, funcs, query_apertures,      vpout_query_apertures,         tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, query_surface,        vpout_query_surface,           tabsize );
    DISP_ADD_FUNC( disp_memfuncs_t, funcs, get_alloc_info,       vpout_get_alloc_info,          tabsize );

    DISP_ADD_FUNC( disp_memfuncs_t, funcs, get_alloc_layer_info, vpout_get_alloc_layer_info,    tabsize );

    return (0);
}


int vpout_mem_init( disp_adapter_t *adapter, char *optstring )
{
    vpout_context_t *vpout = adapter->ms_ctx;
    disp_surface_t  surface;

    surface.pixel_format = DISP_SURFACE_FORMAT_BYTES;
    surface.stride       = surface.width = vpout->aperture_size - VPOUT_HW_PRIMARY_SURFACE_SIZE;
    surface.height       = 1;
    surface.offset       = vpout->aperture_base + VPOUT_HW_PRIMARY_SURFACE_SIZE;
    surface.vidptr       = vpout->aperture + VPOUT_HW_PRIMARY_SURFACE_SIZE;
    surface.paddr        = vpout->aperture_base + VPOUT_HW_PRIMARY_SURFACE_SIZE;
    surface.flags        = DISP_SURFACE_DISPLAYABLE         | DISP_SURFACE_SCALER_DISPLAYABLE |
                           DISP_SURFACE_CPU_LINEAR_READABLE | DISP_SURFACE_CPU_LINEAR_WRITEABLE |
                           DISP_SURFACE_3D_TARGETABLE       | DISP_SURFACE_3D_READABLE |
                           DISP_SURFACE_2D_TARGETABLE       | DISP_SURFACE_2D_READABLE |
                           /*DISP_SURFACE_WT_CACHEABLE |*/    DISP_SURFACE_PHYS_CONTIG;

    adapter->mm_ctx      = disp_vm_create_pool( adapter, &surface, 4096 );

    return (0);
}


void vpout_mem_fini( disp_adapter_t *adapter )
{
    if ( adapter->mm_ctx )
        disp_vm_destroy_pool( adapter, adapter->mm_ctx );

    adapter->mm_ctx = NULL;
}


int vpout_mem_reset( disp_adapter_t *adapter, disp_surface_t *surf_ )
{
    vpout_mem_fini( adapter );
    vpout_mem_init( adapter, NULL );

    return (0);
}


disp_surface_t * vpout_alloc_surface( disp_adapter_t *adapter,  int width, int height, unsigned format, unsigned flags, unsigned user_flags )
{
    vpout_context_t     *vpout        = adapter->ms_ctx;
    disp_surface_t      *surf         = NULL;
    unsigned int        stride        = 0,
                        reject_flags  = 0,
                        mapflags      = 0;

    switch ( format )
    {
        case DISP_SURFACE_FORMAT_ARGB1555:
        case DISP_SURFACE_FORMAT_RGB565:
        case DISP_SURFACE_FORMAT_RGB888:
        case DISP_SURFACE_FORMAT_ARGB8888:
        case DISP_SURFACE_FORMAT_BYTES:
             break;
        default:
            /* No acceleration possible */
            reject_flags = DISP_SURFACE_2D_READABLE | DISP_SURFACE_3D_READABLE |
                           DISP_SURFACE_2D_TARGETABLE | DISP_SURFACE_3D_TARGETABLE;

            /* If it isn't meant to be displayable, and we can't render to it,
             * there is no point in the driver creating it */
            if ( !(flags & (DISP_SURFACE_DISPLAYABLE | DISP_SURFACE_SCALER_DISPLAYABLE)) )
                return (NULL);

            break;
    }

    if ( flags & reject_flags )
        return (NULL);

    if ( flags & (DISP_SURFACE_DMA_SAFE | DISP_SURFACE_PHYS_CONTIG | DISP_SURFACE_PAGE_ALIGNED | DISP_SURFACE_DISPLAYABLE) )
        mapflags = DISP_MAP_PHYS;
    else
        mapflags = 0;

    stride = width * DISP_BYTES_PER_PIXEL( format );
    if ( stride & 0x3f )
        stride = VPOUT_ALIGN64BYTES_STRIDE( stride );

    if ( !(flags & (DISP_SURFACE_DISPLAYABLE | DISP_SURFACE_SCALER_DISPLAYABLE)) )
    {
        /* Allocate from aperture */
        surf = disp_vm_alloc_surface( adapter->mm_ctx, width, height, stride, format, flags );
        if ( surf == NULL )
            return (NULL);

        if ( DISP_VMEM_HINT_USAGE( user_flags ) == DISP_VMEM_HINT_USAGE_CPU )
        {
            disp_vm_free_surface( adapter, surf );
            return (NULL);
        }
    } else {
        surf = &vpout->display_surface;

        if ( vpout->display_surface_allocated )
        {
            disp_printf( adapter, "[vpoutfb] Error: primary surface already allocated" );
            return (NULL);
        }

        memset( surf, 0, sizeof( disp_surface_t ) );

        surf->width        = width;
        surf->height       = height;
        surf->stride       = stride;
        surf->offset       = vpout->aperture_base;
        surf->paddr        = vpout->aperture_base;
        surf->vidptr       = vpout->aperture;
        surf->pixel_format = format;
        surf->flags        = flags | DISP_SURFACE_CPU_LINEAR_WRITEABLE | DISP_SURFACE_CPU_LINEAR_READABLE |
                                     DISP_SURFACE_2D_READABLE | DISP_SURFACE_2D_TARGETABLE;

        vpout->display_surface_allocated = 1;
    }

    disp_printf_debug( adapter, "[vpoutfb] Debug: surface allocated [w=%d h=%d s=%d o=0x%x ptr=0x%x(phys=0x%x)]",
                       width, height, stride, surf->offset, (uint32_t)surf->vidptr, (uint32_t)disp_phys_addr( surf->vidptr ) );

    if ( mapflags & DISP_MAP_PHYS )
        surf->flags |= DISP_SURFACE_DMA_SAFE;
    else
        surf->flags &= ~DISP_SURFACE_PHYS_CONTIG;

    surf->flags |= DISP_SURFACE_PAGE_ALIGNED;

    return surf;
}


int vpout_free_surface( disp_adapter_t *adapter, disp_surface_t *surf )
{
    vpout_context_t *vpout = adapter->ms_ctx;

    disp_printf_debug( adapter, "[vpoutfb] Debug: surface freed [w=%d h=%d s=%d o=0x%x ptr=0x%x(phys=0x%x)]",
                       surf->width, surf->height, surf->stride, surf->offset, (uint32_t)surf->vidptr, (uint32_t)disp_phys_addr( surf->vidptr ) );

    if ( surf->offset == vpout->aperture_base )
        vpout->display_surface_allocated = 0;
    else
        disp_vm_free_surface( adapter->mm_ctx, surf );

    return (0);
}


unsigned long vpout_mem_avail( disp_adapter_t *adapter, unsigned flags )
{
    if ( adapter->mm_ctx )
        return disp_vm_mem_avail( adapter->mm_ctx );

  return (0);
}


/* return the aperture within which the memory surface resides, and the physical offset of the memory within that aperture */
int vpout_query_apertures( disp_adapter_t *adapter, disp_aperture_t *ap )
{
    vpout_context_t *vpout = adapter->ms_ctx;

    ap->base  = vpout->aperture_base;
    ap->size  = vpout->aperture_size;
    ap->flags = DISP_APER_NOCACHE;
    ap++;

    ap->base  = vpout->registers_base;
    ap->size  = vpout->registers_size;
    ap->flags = DISP_APER_NOCACHE;
    ap++;

    return (2);
}


/* return the aperture within which the memory surface resides, and the physical offset of the memory within that aperture */
int vpout_query_surface( disp_adapter_t *adapter, disp_surface_t *surf, disp_surface_info_t *info )
{
    vpout_context_t *vpout = adapter->ms_ctx;

    info->aperture_index = 0;
    info->offset         = surf->offset - vpout->aperture_base;

    return (0);
}


/* If a client of the driver wants to allocate memory itself, it must allocate it in accordance with the parameters returned by
 * this function.  Since this memory will not be mapped into the video aperture, we must check the flags accordingly. */
int vpout_get_alloc_info( disp_adapter_t *adapter, int width, int height, unsigned format, unsigned flags, unsigned user_flags, disp_alloc_info_t *info )
{
    flags &= DISP_SURFACE_CAPS_MASK;

    if ( flags & ~( DISP_SURFACE_CPU_LINEAR_READABLE | DISP_SURFACE_CPU_LINEAR_WRITEABLE | DISP_SURFACE_PAGE_ALIGNED | DISP_SURFACE_PHYS_CONTIG) )
        return (-1);

    info->start_align = 4096;
    info->end_align   = 1;
    info->min_stride  = width * DISP_BYTES_PER_PIXEL( format );
    info->max_stride  = ~0;
    info->stride_gran = 64;
    info->map_flags   = 0;
    info->prot_flags  = DISP_PROT_READ | DISP_PROT_WRITE;
    info->surface_flags = DISP_SURFACE_CPU_LINEAR_READABLE | DISP_SURFACE_CPU_LINEAR_WRITEABLE | DISP_SURFACE_DRIVER_NOT_OWNER | DISP_SURFACE_PHYS_CONTIG;

    return (0);
}


int vpout_get_alloc_layer_info( disp_adapter_t *adapter, int dispno[], int layer[], int nlayers, unsigned format, int surface_index,
                                int width, int height, unsigned sflags, unsigned hint_flags, disp_alloc_info_t *info )
{
    return (-1);
}
