/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                  DEFINITIONS                  */
/*************************************************/


/* Hardware definitions */
#define UNDIVPIXCLK                     2315                                    /* UNDIVPIXCLK is undivided pixclock in picoseconds for 432MHz */
#define CLEAR_MSEC                      40                                      /* FIFO clear delay (ms) */


/* Misc definitions */
#define DIV_ROUND_CLOSEST( x, divisor ) ({ typeof( divisor ) __divisor = divisor; (((x) + ((__divisor) / 2)) / (__divisor)); })


/*************************************************/
/*                MISC FUNCTIONS                 */
/*************************************************/


/* Hz to picoseconds */
static inline uint32_t hz_to_ps( uint32_t rate )
{
    uint32_t ps_in_us = 1000000;

    /* convert to times / microsecond */
    rate /= 1000000;

    return ps_in_us / rate;
}


/* APLL frequency in Hz */
static inline uint32_t get_apll_frequency( vpout_context_t *vpout )
{
    /* Same as CPLL */
    return ((*CMCTR_MMIO32( SEL_CPLL ) & SEL_CPLL_SEL) + 1) * VPOUT_XTI_FREQUENCY * 1000 * 1000;
}


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


struct sigevent * vpout_hw_isr( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw )
{
    uint32_t        status = *MMIO32( LCDINT );

    if ( status & INTERRUPT_OUT_FIFO )
    {
        uint32_t    i, j;

        /* Reset VPOUT display controller */
            for (j = 0; j < 2; j++)
            {
                *MMIO32( LCDCSR ) = CSR_EN;
                *MMIO32( LCDCSR ) = CSR_INIT | CSR_EN;
                for ( i = 0; i < 250; i++ )
                {
                    if ( !(*MMIO32( LCDCSR ) & CSR_INIT) )
                        break;

                    disp_usecspin( 1000 );
                }
                if ( i == 250 )
                    vpout->error_reset_counter++;
            }
            *MMIO32( LCDCSR ) = CSR_RUN | CSR_EN;

        vpout->error_counter++;
    }

    /* Clear IRQ source */
    *MMIO32( LCDINT ) = status;

    if ( status & INTERRUPT_SYNC_DONE )
    {
        /* Update counter */
        vpout->vsync_counter[0 /*dispno*/]++;

        return &vpout->irq_event;
    }

    return (NULL);
}


void vpout_hw_pipe_wait_for_vblank( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, int pipe )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint32_t        counter  = 50 /* 50 ms */;

    /* Clear previous VSYNC IRQ */
    *MMIO32( LCDINT ) |= INTERRUPT_SYNC_DONE;

    while ( counter )
    {
        if ( *MMIO32( LCDINT ) & INTERRUPT_SYNC_DONE )
        {
            *MMIO32( LCDINT ) |= INTERRUPT_SYNC_DONE;
            return;
        }

        disp_usecspin( 1000 );
        counter--;
    }

    disp_printf_warning( adapter, "[vpout] Warning: pipe %d vsync timeout", pipe );
}


void vpout_hw_disable( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw )
{
    disp_adapter_t  *adapter = vpout->adapter;
    int             i, pipe  = 0;

    /* Mask all IRQs */
        *MMIO32( LCDINTMASK ) = 0;

    /* If the device is currently on, clear FIFO and power it off */
        if ( *MMIO32( LCDCSR ) & CSR_EN )
        {
            *MMIO32( LCDCSR ) = CSR_EN;
            *MMIO32( LCDCSR ) = CSR_EN | CSR_CLR;

            for ( i = 0; i < CLEAR_MSEC; i++ )
            {
                if ( !(*MMIO32( LCDCSR ) & CSR_CLR) )
                    break;

                disp_usecspin( 1000 );
            }
            if ( i == CLEAR_MSEC )
                disp_printf_warning( adapter, "[vpoutfb] Warning: LCD disable sequence failed [FIFO clear timeout]" );
            *MMIO32( LCDCSR ) = 0;
        }

    /* vpoutfb_clocks_destroy() */
        //~ *CMCTR_MMIO32( GATE_CORE_CTR ) &= ~VPOUT_EN;

    /* Reset HDMI device */
        for ( pipe = 0; pipe < VPOUT_GPU_PIPES; pipe++ )
        {
            if ( (DISPLAY_PORT( vpout->display[pipe] ) == DISPLAY_PORT_TYPE_HDMI) &&
                 (strcmp( vpout->hdmi[DISPLAY_PORT_INDEX( vpout->display[pipe] )].transmitter, VPOUT_OPT_HDMI_IT66121 ) == 0) )
            {
                it66121_reset( vpout, vpout_draw, DISPLAY_PORT_INDEX( vpout->display[pipe] ) );
                it66121_remove( vpout, vpout_draw, DISPLAY_PORT_INDEX( vpout->display[pipe] ) );
            }
        }
}


void vpout_hw_pipe_set_display_offset( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, int pipe, uint32_t offset )
{
    *MMIO32( LCDAB0 ) = offset;
    *MMIO32( LCDAB1 ) = offset;
}


int vpout_hw_configure_display( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, disp_crtc_settings_t *settings,
                                vpout_display_conf_t display, int pipe, disp_surface_t *surface, disp_mode_t mode_ )
{
    disp_adapter_t  *adapter = vpout->adapter;
    uint32_t        mode     = 0;
    int             hsw, hgdel, hgate, hlen, vsw, vgdel, vgate, vlen, div, i, undiv;

    disp_printf_info( adapter, "[vpout] Info: %s[%d] mode set sequence started [%d:%d@%d %dbpp]", 
                      DISPLAY_PORT_NAME( display ), DISPLAY_PORT_INDEX( display ), vpout->xres, vpout->yres, vpout->refresh, vpout->bpp );

    /* vpoutfb_clocks_init() */
        //~ *CMCTR_MMIO32( GATE_CORE_CTR ) |= VPOUT_EN;

    /* Prepare HDMI port */
    if ( (DISPLAY_PORT( display ) == DISPLAY_PORT_TYPE_HDMI) &&
         (strcmp( vpout->hdmi[DISPLAY_PORT_INDEX( display )].transmitter, VPOUT_OPT_HDMI_IT66121 ) == 0) )
        it66121_probe( vpout, vpout_draw, DISPLAY_PORT_INDEX( display ) );

    /* vpoutfb_set_par() */
        /* If the device is currently on, clear FIFO and power it off */
            if ( *MMIO32( LCDCSR ) & CSR_EN )
            {
                *MMIO32( LCDCSR ) = CSR_EN;
                *MMIO32( LCDCSR ) = CSR_EN | CSR_CLR;
                for ( i = 0; i < CLEAR_MSEC; i++ )
                {
                    if ( !(*MMIO32( LCDCSR ) & CSR_CLR) )
                        break;

                    disp_usecspin( 1000 );
                }
                if ( i == CLEAR_MSEC )
                {
                    disp_printf( adapter, "[vpoutfb] Fatal: HDMI mode set sequence failed [FIFO clear timeout]" );
                    return (-EBUSY);
                }
                *MMIO32( LCDCSR ) = 0;
            }

        /* Turn on and reset the device */
            *MMIO32( LCDCSR ) = CSR_EN;
            *MMIO32( LCDCSR ) = CSR_EN | CSR_CLR;
            for ( i = 0; i < CLEAR_MSEC; i++ )
            {
                if ( !(*MMIO32( LCDCSR ) & CSR_CLR) )
                    break;

                disp_usecspin( 1000 );
            }
            if ( i == CLEAR_MSEC )
            {
                disp_printf( adapter, "[vpoutfb] Fatal: HDMI mode set sequence failed [LCD reset timeout]" );
                return (-EBUSY);
            }

        /* Configure video mode */
            hsw   = settings->h_sync_len - 1;
            vsw   = settings->v_sync_len - 1;
            hgate = settings->xres - 1;
            vgate = settings->yres - 1;
            hgdel = settings->h_total - settings->h_sync_start - settings->h_sync_len - 1;
            vgdel = settings->v_total - settings->v_sync_start - settings->v_sync_len - 1;
            hlen  = settings->h_total - 1;
            vlen  = settings->v_total - 1;

#ifdef __QNX__
            undiv = hz_to_ps( get_apll_frequency( vpout ) );
            div = DIV_ROUND_CLOSEST( hz_to_ps( settings->pixel_clock * 1000 ), undiv ) - 1;
#else
            if ( par->clk_count == 2 )
                undiv = hz_to_ps( get_apll_frequency( vpout ) );
            else
                undiv = UNDIVPIXCLK;
            div   = DIV_ROUND_CLOSEST( hz_to_ps( settings->pixel_clock ), undiv ) - 1;
#endif

            *MMIO32( LCDHT0 ) = ((hgdel << LCDHT0_HGDEL_SHIFT) & LCDHT0_HGDEL_MASK) |
                                ((hsw   << LCDHT0_HSW_SHIFT)   & LCDHT0_HSW_MASK);
            *MMIO32( LCDHT1 ) = ((hlen  << LCDHT1_HLEN_SHIFT)  & LCDHT1_HLEN_MASK) |
                                ((hgate << LCDHT1_HGATE_SHIFT) & LCDHT1_HGATE_MASK);
            *MMIO32( LCDVT0 ) = ((vgdel << LCDHT1_VGDEL_SHIFT) & LCDHT1_VGDEL_MASK) |
                                ((vsw   << LCDHT1_VSW_SHIFT)   & LCDHT1_VSW_MASK);
            *MMIO32( LCDVT1 ) = ((vlen  << LCDHT1_VLEN_SHIFT)  & LCDHT1_VLEN_MASK) |
                                ((vgate << LCDHT1_VGATE_SHIFT) & LCDHT1_VGATE_MASK);
            *MMIO32( LCDDIV ) = div;
            *MMIO32( LCDAB0 ) = vpout->aperture_base;
            *MMIO32( LCDAB1 ) = vpout->aperture_base;
            *MMIO32( LCDOF0 ) = 0;
            *MMIO32( LCDOF1 ) = 0;

        /* Setup LCD display controller mode */
#if 0
            if ( DISPLAY_PORT( display ) == DISPLAY_PORT_TYPE_HDMI )
                mode = LCDMODE_VINV | LCDMODE_HINV;
#else
            mode = (settings->flags & DISP_SYNC_POLARITY_H_POS ? 0 : LCDMODE_HINV) |
                   (settings->flags & DISP_SYNC_POLARITY_V_POS ? 0 : LCDMODE_VINV);
#endif
#ifdef ENABLE_HW_CURSOR
            mode |= LCDMODE_HWC_MODE | LCDMODE_HWC_MODE_64x64;
#endif
            switch( surface->pixel_format )
            {
                case DISP_SURFACE_FORMAT_ARGB1555:
                    mode |= LCDMODE_INSIZE_ARGB1555;
                    break;
                case DISP_SURFACE_FORMAT_RGB565:
                    mode |= LCDMODE_INSIZE_RGB565;
                    break;
                case DISP_SURFACE_FORMAT_RGB888:
                    mode |= LCDMODE_INSIZE_RGB888;
                    break;
                default:
                case DISP_SURFACE_FORMAT_ARGB8888:
                    mode |= LCDMODE_INSIZE_ARGB8888;
                    break;
            }
            *MMIO32( LCDMODE ) = mode;

        /* Unmask and clear IRQ source */
#ifdef ENABLE_IRQ
            *MMIO32( LCDINT )     = INTERRUPT_SYNC_DONE | INTERRUPT_OUT_FIFO;
            *MMIO32( LCDINTMASK ) = INTERRUPT_SYNC_DONE | INTERRUPT_OUT_FIFO;
#else
            *MMIO32( LCDINTMASK ) = 0;
#endif

        /* Finally, initialize and run the device */
            *MMIO32( LCDCSR ) = CSR_INIT | CSR_EN;
            for ( i = 0; i < CLEAR_MSEC; i++ )
            {
                if ( !(*MMIO32( LCDCSR ) & CSR_INIT) )
                    break;

                disp_usecspin( 1000 );
            }
            if ( i == CLEAR_MSEC )
            {
                disp_printf( adapter, "[vpoutfb] Fatal: HDMI mode set sequence failed [timeout]" );
                return (-EBUSY);
            }
            *MMIO32( LCDCSR ) = CSR_RUN | CSR_EN;

            if ( (DISPLAY_PORT( display ) == DISPLAY_PORT_TYPE_HDMI) &&
                 (strcmp( vpout->hdmi[DISPLAY_PORT_INDEX( display )].transmitter, VPOUT_OPT_HDMI_IT66121 ) == 0) )
                return it66121_init( vpout, vpout_draw, DISPLAY_PORT_INDEX( display ), hz_to_ps( settings->pixel_clock * 1000 ) );

    return (0);
}
