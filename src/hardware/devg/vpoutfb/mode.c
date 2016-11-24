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
        disp_printf_info( adapter, "[haswell] Info: %s[%d] mode set sequence finished successfully", DISPLAY_PORT_NAME( vpout->display[pipe] ), pipe );
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
