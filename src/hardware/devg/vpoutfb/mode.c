/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


int devg_get_modefuncs( disp_adapter_t *adapter, disp_modefuncs_t *funcs, int tabsize )
{
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, init,                       vpout_init,                 tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, fini,                       vpout_fini,                 tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, module_info,                vpout_module_info,          tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, get_modeinfo,               vpout_get_modeinfo,         tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, get_modelist,               vpout_get_modelist,         tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, set_mode,                   vpout_set_mode,             tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, wait_vsync,                 vpout_wait_vsync,           tabsize );
    //DISP_ADD_FUNC( disp_modefuncs_t, funcs, set_display_offset,         vpout_set_display_offset,   tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, devctl,                     vpout_devctl,               tabsize );

#ifdef ENABLE_HW_CURSOR
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, set_hw_cursor,              vpout_set_hw_cursor,        tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, enable_hw_cursor,           vpout_enable_hw_cursor,     tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, disable_hw_cursor,          vpout_disable_hw_cursor,    tabsize );
    DISP_ADD_FUNC( disp_modefuncs_t, funcs, set_hw_cursor_pos,          vpout_set_hw_cursor_pos,    tabsize );
#endif

    return (0);
}


int vpout_get_modeinfo( disp_adapter_t *adapter, int dispno, disp_mode_t mode, disp_mode_info_t *info )
{
    info->caps = DISP_MCAP_SET_DISPLAY_OFFSET | DISP_MCAP_VIRTUAL_PANNING | DISP_MCAP_DPMS_SUPPORTED;

    info->crtc_start_gran    = DISP_BYTES_PER_PIXEL( info->pixel_format );
    info->crtc_pitch_gran    = 1;
    info->max_virtual_width  = 4096;
    info->max_virtual_height = 4096;

    info->u.generic.min_pixel_clock = 0;
    info->u.generic.max_pixel_clock = 1847000; /* In KHz */
    info->u.generic.h_granularity   = 1;
    info->u.generic.v_granularity   = 1;

    /* All modes we report are generic modes.  The mode numbers we
     * define are simply the number of bits per pixel for the mode */
    switch ( mode )
    {
        case 15: info->pixel_format = DISP_SURFACE_FORMAT_ARGB1555; break;
        case 16: info->pixel_format = DISP_SURFACE_FORMAT_RGB565;   break;
        case 24: info->pixel_format = DISP_SURFACE_FORMAT_RGB888;   break;
        case 32: info->pixel_format = DISP_SURFACE_FORMAT_ARGB8888; break;
        default:
            return (-1);
    }

    info->flags = DISP_MODE_GENERIC;

    return (0);
}


int vpout_get_modelist( disp_adapter_t *adapter, int dispno, disp_mode_t *list, int index, int size )
{
    static unsigned modes[] = { 15, 16, 24, 32 };
    int             i = 0;
    int             j = 0;

    for ( i = index; j < size - 1 && i < sizeof( modes ) / sizeof( modes[0] ); i++ )
        list[j++] = modes[i];

    list[j] = DISP_MODE_LISTEND;

    return (0);
}


int vpout_set_mode( disp_adapter_t *adapter, int dispno, disp_mode_t mode, disp_crtc_settings_t *settings, disp_surface_t *surf, unsigned flags )
{
    vpout_context_t         *vpout      = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw = adapter->gd_ctx;
    int                     pipe        = dispno;

    disp_printf_info( adapter, "[vpoutfb] Info: mode switch sequence started [%d:%d@%d %dbpp]", settings->xres, settings->yres, settings->refresh, mode );
    disp_printf_info( adapter, "[vpoutfb] Info: mode switcher surface [ptr=0x%x(phys=0x%x)]", (unsigned int)surf->vidptr, (unsigned int)surf->paddr );

    /* Checking wrong display.conf settings */
    if ( VPOUT_DISPMODE_BAD_PIPE( pipe ) )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: too much display sections in the display.conf" );
        return (-1);
    }

    /* Disable sequence only at the first display mode set */
    if ( pipe == 0 )
    {
        disp_printf_debug( adapter, "[vpoutfb] Debug: Prepare HW disabling" );
        vpout_hw_disable( vpout, vpout_draw );
        disp_printf_debug( adapter, "[vpoutfb] Debug: HW disabled" );
    }

    /* Current pipe's settings */
    vpout->bpp     = mode;
    vpout->xres    = settings->xres;
    vpout->yres    = settings->yres;
    vpout->refresh = settings->refresh;

    /* Mode set sequences */
    disp_printf_debug( adapter, "[vpoutfb] Debug: %s[%d] configuration = 0x%08x", DISPLAY_PORT_NAME( vpout->display[pipe] ), pipe, vpout->display[pipe] );
    if ( DISPLAY_PORT( vpout->display[pipe] ) == DISPLAY_PORT_TYPE_HDMI )
    {
        if ( vpout_hw_configure_display( vpout, vpout_draw, settings, vpout->display[pipe], pipe, surf, mode ) != 0 )
        {
            disp_printf( adapter, "[vpoutfb] Fatal: %s[%d] mode switch sequence failed", DISPLAY_PORT_NAME( vpout->display[pipe] ), pipe );
            return (-1);
        }
        disp_printf_info( adapter, "[vpoutfb] Info: %s[%d] mode set sequence finished successfully", DISPLAY_PORT_NAME( vpout->display[pipe] ), pipe );
    }

    /* Wait for the current pipe vsync */
    vpout_hw_pipe_wait_for_vblank( vpout, vpout_draw, pipe );

    if ( VPOUT_DISPMODE_LAST_PIPE( pipe ) )
        disp_printf_debug( adapter, "[vpoutfb] Debug: HW enabled" );

    return (0);
}


int vpout_wait_vsync( disp_adapter_t *adapter, int dispno )
{
    vpout_context_t         *vpout      = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw = adapter->gd_ctx;
    int                     pipe          = dispno;

    if ( VPOUT_DISPMODE_BAD_PIPE( pipe ) )
        return (-1);

    if ( adapter->callback )
        adapter->callback( adapter->callback_handle, DISP_CALLBACK_UNLOCK, NULL );

#ifdef ENABLE_IRQ
    if ( vpout->error_counter || vpout->error_reset_counter )
    {
        disp_printf_debug( adapter, "[vpoutfb] Error: caught OUT_FIFO interrupt (display controller FIFO reinitialized %d times)",
                           vpout->error_counter );
        vpout->error_counter = 0;

        disp_printf_debug( adapter, "[vpoutfb] Error: display controller reset failed at OUT_FIFO interrupt (counter: %d)",
                           vpout->error_reset_counter );
        vpout->error_reset_counter = 0;
    }
#endif

    if ( !vpout->irq_polling )
    {
        iov_t           iov;
        struct _pulse   pulse;
        int             rcvid;
        unsigned        prev_vsync;
        uint64_t        timeout = 500 * 1000 * 1000 /* 500 ms */;
    
        prev_vsync = vpout->vsync_counter[dispno];
        SETIOV( &iov, &pulse, sizeof( pulse ) );

        while ( 1 )
        {
            TimerTimeout( CLOCK_REALTIME, _NTO_TIMEOUT_RECEIVE, NULL, &timeout, NULL );

            rcvid = MsgReceivev( vpout->irq_chid, &iov, 1, NULL );
            if ( rcvid == -1 )
            {
                vpout->irq_polling = 1;
                disp_printf( adapter, "[vpoutfb] Error: switching to vsync polling mode" );
                break;
            } else
                if ( pulse.code == VBLANK_PULSE )
                    if ( prev_vsync != vpout->vsync_counter[dispno] )
                        break;
        }
    }

    if ( vpout->irq_polling )
    {
        vpout_hw_pipe_wait_for_vblank( vpout, vpout_draw, pipe );

        vpout->vsync_counter[pipe]++;
    }

    if ( adapter->callback )
        adapter->callback( adapter->callback_handle, DISP_CALLBACK_LOCK, NULL );

    return (0);
}


int vpout_set_display_offset( disp_adapter_t *adapter, int dispno, unsigned offset, int wait_vsync )
{
    vpout_context_t       *vpout        = adapter->ms_ctx;
    vpout_draw_context_t  *vpout_draw   = adapter->gd_ctx;
    int                     pipe        = dispno;

    vpout_hw_pipe_set_display_offset( vpout, vpout_draw, pipe, offset );

    if ( wait_vsync && adapter->callback )
        adapter->callback( adapter->callback_handle, DISP_CALLBACK_WAIT_VSYNC, &dispno );

    return (0);
}


static inline uint32_t get_layer_format( uint32_t surface_format )
{
    uint32_t        layer_format = 0;

    switch ( surface_format )
    {
        case DISP_SURFACE_FORMAT_BYTES:                     layer_format = DISP_LAYER_FORMAT_BYTES;           break;
        case DISP_SURFACE_FORMAT_PAL8:                      layer_format = DISP_LAYER_FORMAT_PAL8;            break;
        case DISP_SURFACE_FORMAT_ARGB1555:                  layer_format = DISP_LAYER_FORMAT_ARGB1555;        break;
        case DISP_SURFACE_FORMAT_RGB565:                    layer_format = DISP_LAYER_FORMAT_RGB565;          break;
        case DISP_SURFACE_FORMAT_RGB888:                    layer_format = DISP_LAYER_FORMAT_RGB888;          break;
        case DISP_SURFACE_FORMAT_ARGB8888:                  layer_format = DISP_LAYER_FORMAT_ARGB8888;        break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_UYVY:            layer_format = DISP_LAYER_FORMAT_UYVY;            break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_YUY2:            layer_format = DISP_LAYER_FORMAT_YUY2;            break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_YVYU:            layer_format = DISP_LAYER_FORMAT_YVYU;            break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_V422:            layer_format = DISP_LAYER_FORMAT_V422;            break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_UYVY_INTERLACED: layer_format = DISP_LAYER_FORMAT_UYVY_INTERLACED; break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_YUY2_INTERLACED: layer_format = DISP_LAYER_FORMAT_YUY2_INTERLACED; break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_YVYU_INTERLACED: layer_format = DISP_LAYER_FORMAT_YVYU_INTERLACED; break;
        case DISP_SURFACE_FORMAT_PACKEDYUV_V422_INTERLACED: layer_format = DISP_LAYER_FORMAT_V422_INTERLACED; break;
    }

    return layer_format;
}


int vpout_devctl( disp_adapter_t *adapter, int dispno, disp_mode_devctl_t cmd, void *data_in, int nbytes, void *data_out, int *out_buffer_size )
{
    vpout_context_t       *vpout      = adapter->ms_ctx;
    vpout_draw_context_t  *vpout_draw = adapter->gd_ctx;

    switch ( cmd )
    {
#if defined( ENABLE_DDC )
        #define DEVCTL_DDC_EXIT_STATUS( error ) {         \
            if ( *out_buffer_size >= sizeof( uint32_t ) ) \
                *((uint32_t *)data_out) = error;          \
            *out_buffer_size = sizeof( uint32_t );        \
            return (0);                                   \
        }

        case DEVCTL_DDC:
        {
            int                     size     = *out_buffer_size;
            int                     port     = 0;
            devctl_ddc_request_t    *request = (devctl_ddc_request_t *)data_in;

            disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DDC received (isize=%d, osize=%d)", nbytes, size );

            if ( nbytes != sizeof( devctl_ddc_request_t ) )
            {
                disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DDC failed (invalid request isize)" );
                DEVCTL_DDC_EXIT_STATUS( EINVAL );
            }

            if ( request->mode == DEVCTL_DDC_MODE_DISPLAY )
                port = dispno;
            else {
                switch ( request->bus )
                {
                    case 0: port = dispno; break;
                    default:
                        disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DDC failed (invalid bus)" );
                        DEVCTL_DDC_EXIT_STATUS( ENODEV );
                }
            }

            memset( data_out, 0, size );

            size = vpout_hw_read_edid( vpout, vpout_draw, port, data_out, size );

            if ( size <= 0 )
                DEVCTL_DDC_EXIT_STATUS( EIO );

            if ( DEBUG )
            {
                int         j = 0,
                            i = 0;
                uint8_t     *edid = data_out;
                char        str[256];

                sprintf( str, "EDID dump:         " );
                for ( i = 0; i < 16; i++ )
                    sprintf( str, "%s%X:    ", str, i );
                disp_printf_debug( adapter, "[vpoutfb] Debug: %s", str );
                
                disp_printf_debug( adapter, "[vpoutfb]                         ----------------------------------------------------------------------------------------------" );
                for ( i = 0; i < size / 16; i++ )
                {
                    sprintf( str, "           %02X: | ", i );
                    for ( j = 0; j < 16; j++ )
                        sprintf( str, "%s0x%02X  ", str, edid[i * 16 + j] );
                    disp_printf_debug( adapter, "[vpoutfb]        %s", str );
                }
            }

            *out_buffer_size = size;

            break;
        }
#endif

#if defined( ENABLE_DISPLAY_INFO )
        case DEVCTL_DISPLAY_INFO:
        {
            devctl_display_mode_t           *buffer  = (devctl_display_mode_t *)data_in;
            devctl_display_mode_request_t   *request = &((devctl_display_mode_t *)data_in)->request;
            devctl_display_mode_reply_t     *reply   = &((devctl_display_mode_t *)data_out)->reply;
            uint8_t                         mode     = request->mode;
            uint8_t                         display  = request->display;
            uint8_t                         layer    = request->layer;

            disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DISPLAY_INFO received (isize=%d, osize=%d)", nbytes, *out_buffer_size );

            *out_buffer_size = sizeof( devctl_display_mode_t );
            memset( buffer, 0, sizeof( devctl_display_mode_t ) );

            switch ( mode )
            {
                case DEVCTL_DISPLAY_INFO_LAYER:
                    reply->status = DEVCTL_DISPLAY_INFO_STATUS_OK;
                    if ( display < 0 || display >= VPOUT_GPU_PIPES )
                    {
                        reply->status = DEVCTL_DISPLAY_INFO_STATUS_FAIL;
                        disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DISPLAY_INFO failed (invalid display index)" );
                        break;
                    }
                    if ( layer < 0 || layer >= VPOUT_GPU_LAYERS )
                    {
                        reply->status = DEVCTL_DISPLAY_INFO_STATUS_FAIL;
                        disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DISPLAY_INFO failed (invalid layer index)" );
                        break;
                    }

                    reply->layer.id     = layer;
                    reply->layer.format = -1;
                    reply->layer.state  = vpout->xres > 0 ? DEVCTL_DISPLAY_INFO_STATE_ON : DEVCTL_DISPLAY_INFO_STATE_OFF;
                    if ( reply->layer.state == DEVCTL_DISPLAY_INFO_STATE_ON )
                    {
                        reply->layer.sid                     = (uint32_t)adapter->callback( adapter->callback_handle, DISP_CALLBACK_SURFACE_SID, &vpout->display_surface );
                        reply->layer.format                  = get_layer_format( vpout->display_surface.pixel_format );
                        reply->layer.width                   = vpout->xres;
                        reply->layer.height                  = vpout->yres;
                        reply->layer.stride                  = vpout->display_surface.stride;
                        reply->layer.x                       = 0;
                        reply->layer.y                       = 0;
                        reply->layer.addr                    = vpout->display_surface.paddr;
                        reply->layer.viewport.source.x1      = 0;
                        reply->layer.viewport.source.y1      = 0;
                        reply->layer.viewport.source.x2      = vpout->xres - 1;
                        reply->layer.viewport.source.y2      = vpout->yres - 1;
                        reply->layer.viewport.destination.x1 = 0;
                        reply->layer.viewport.destination.y1 = 0;
                        reply->layer.viewport.destination.x2 = vpout->xres - 1;
                        reply->layer.viewport.destination.y2 = vpout->yres - 1;
                        reply->layer.chroma_key.color        = 0;
                        reply->layer.chroma_key.mode         = DEVCTL_DISPLAY_INFO_CHROMA_NONE;

                    }

                case DEVCTL_DISPLAY_INFO_DISPLAY:
                    reply->status = DEVCTL_DISPLAY_INFO_STATUS_OK;
                    if ( display < 0 || display >= VPOUT_GPU_PIPES )
                    {
                        reply->status = DEVCTL_DISPLAY_INFO_STATUS_FAIL;
                        disp_printf_debug( adapter, "[vpoutfb] Debug: DEVCTL_DISPLAY_INFO failed (invalid display index)" );
                        break;
                    }

                    reply->display.id        = display;
                    reply->display.state     = display < VPOUT_GPU_PIPES ? DEVCTL_DISPLAY_INFO_STATE_ON : DEVCTL_DISPLAY_INFO_STATE_OFF;
                    reply->display.layers    = VPOUT_GPU_LAYERS;
                    reply->display.interface = DEVCTL_DISPLAY_INFO_IFACE_NONE;

                    if ( reply->display.state == DEVCTL_DISPLAY_INFO_STATE_ON )
                    {
                        reply->display.width     = vpout->xres;
                        reply->display.height    = vpout->yres;
                        reply->display.refresh   = vpout->refresh;
                        reply->display.interface = DEVCTL_DISPLAY_INFO_IFACE_HDMI;
                    }

                case DEVCTL_DISPLAY_INFO_DISPLAYS:
                    reply->status   = DEVCTL_DISPLAY_INFO_STATUS_OK;
                    reply->displays = VPOUT_GPU_PIPES;
                    break;
            }

            break;
        }
#endif

        default:
            return (-1);
    }

    return (EOK);
}
