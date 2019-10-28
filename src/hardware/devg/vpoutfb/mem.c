/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/

static int vpout_mem_init( disp_adapter_t *adapter, char *optstring )
{
	return 0;
}


static void vpout_mem_fini( disp_adapter_t *adapter )
{

}

static int vpout_mem_reset( disp_adapter_t *adapter, disp_surface_t *surf_ )
{
	vpout_mem_fini( adapter );
	vpout_mem_init( adapter, NULL );

	return 0;
}


/* return the aperture within which the memory surface resides, and the physical offset of the memory within that aperture */
static int vpout_query_apertures( disp_adapter_t *adapter, disp_aperture_t *ap )
{
	vpout_context_t *vpout = adapter->ms_ctx;

	ap->base  = vpout->registers_base;
	ap->size  = vpout->registers_size;
	ap->flags = DISP_APER_NOCACHE;
	ap ++;

	return 1;
}


/* If a client of the driver wants to allocate memory itself, it must allocate it in accordance with the parameters returned by
 * this function.  Since this memory will not be mapped into the video aperture, we must check the flags accordingly. */
static int vpout_get_alloc_info( disp_adapter_t *adapter, int width, int height, unsigned format, unsigned flags, unsigned user_flags, disp_alloc_info_t *info )
{
	flags &= DISP_SURFACE_CAPS_MASK;

	/*if ( flags & ~( DISP_SURFACE_CPU_LINEAR_READABLE | DISP_SURFACE_CPU_LINEAR_WRITEABLE | DISP_SURFACE_PAGE_ALIGNED | DISP_SURFACE_PHYS_CONTIG) )
		return -1;*/

	info->start_align = 64/8; /*4096;*/
	info->end_align   = 1;
	info->min_stride  = width * DISP_BYTES_PER_PIXEL(format);	/* actual stride */
	info->max_stride  = ~0;
	info->stride_gran = 64/8; /* this is actually ignored */
	info->map_flags   = 0;
	info->prot_flags  = DISP_PROT_READ | DISP_PROT_WRITE;
	info->surface_flags = flags | DISP_SURFACE_PAGE_ALIGNED;
	
	if ( flags & DISP_SURFACE_DISPLAYABLE )
	{
		info->min_stride = ALIGN_64BIT( info->min_stride );
		info->map_flags |= DISP_MAP_PHYS;
		info->prot_flags |= DISP_PROT_NOCACHE;
		info->surface_flags |= DISP_SURFACE_NON_CACHEABLE | DISP_SURFACE_DMA_SAFE |
			DISP_SURFACE_PHYS_CONTIG | DISP_SURFACE_PAGE_ALIGNED |
			DISP_SURFACE_CPU_LINEAR_READABLE | DISP_SURFACE_CPU_LINEAR_WRITEABLE;
	}

	return 0;
}


static int vpout_get_alloc_layer_info( disp_adapter_t *adapter, int dispno[], int layer[], int nlayers, unsigned format, int surface_index,
								int width, int height, unsigned sflags, unsigned hint_flags, disp_alloc_info_t *info )
{
	unsigned alloc_format;
	
	switch (format) {
		case DISP_LAYER_FORMAT_PAL8:
			alloc_format = DISP_SURFACE_FORMAT_PAL8;
			break;
		case DISP_LAYER_FORMAT_ARGB1555:
			alloc_format = DISP_SURFACE_FORMAT_ARGB1555;
			break;
		case DISP_LAYER_FORMAT_RGB565:
			alloc_format = DISP_SURFACE_FORMAT_RGB565;
			break;
		case DISP_LAYER_FORMAT_RGB888:
			alloc_format = DISP_SURFACE_FORMAT_RGB888;
			break;
		case DISP_LAYER_FORMAT_ARGB8888:
			alloc_format = DISP_SURFACE_FORMAT_ARGB8888;
			break;
		default:
			return -1;
	}
	
	return vpout_get_alloc_info( adapter, width, height, alloc_format,
		sflags | DISP_SURFACE_DISPLAYABLE, hint_flags, info );
}

static int vpout_submit_alloced_info(disp_adapter_t *adp, disp_surface_t *surf, unsigned flags)
{
	/*surf->offset = surf->paddr;*/
	disp_printf_debug( adp, "[vpoutfb] Debug: surface allocated [w=%d h=%d s=%d o=0x%x ptr=0x%p(phys=0x%llx), flags=0x%x]",
					   surf->width, surf->height, surf->stride, surf->offset,
					   surf->vidptr, disp_phys_addr( surf->vidptr ), flags );
	
	return 0;
}


int devg_get_memfuncs( disp_adapter_t *adapter, disp_memfuncs_t *funcs, int tabsize )
{
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, init,                 vpout_mem_init,                tabsize );
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, fini,                 vpout_mem_fini,                tabsize );
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, module_info,          vpout_module_info,             tabsize );
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, reset,                vpout_mem_reset,               tabsize );

	DISP_ADD_FUNC( disp_memfuncs_t, funcs, query_apertures,      vpout_query_apertures,         tabsize );
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, get_alloc_info,       vpout_get_alloc_info,          tabsize );
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, get_alloc_layer_info, vpout_get_alloc_layer_info,    tabsize );
	DISP_ADD_FUNC( disp_memfuncs_t, funcs, submit_alloced_info,  vpout_submit_alloced_info,     tabsize );

	return 0;
}
