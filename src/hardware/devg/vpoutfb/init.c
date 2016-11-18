/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include "vpoutfb.h"


/*************************************************/
/*                   FUNCTIONS                   */
/*************************************************/


const struct sigevent * vpout_isr_handler( void *ptr, int id )
{
    vpout_draw_context_t    *vpout_draw = (vpout_draw_context_t *)ptr;
    vpout_context_t         *vpout      = vpout_draw->vpout;

    return vpout_hw_isr( vpout, vpout_draw );
}


int vpout_isr_setup( disp_adapter_t *adapter, vpout_context_t *vpout, vpout_draw_context_t *vpout_draw )
{
    int             err = EOK;

#ifndef ENABLE_IRQ
    vpout->irq_polling = 0;
#else
    vpout->irq_polling = 1;
    return (0);
#endif

    if ( vpout->irq == 0 )
        /* Switch to polling */
        return (-1);

    if ( (vpout->irq_chid = ChannelCreate( _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK )) == -1 )
    {
        disp_printf( adapter, "[vpoutfb] Error: can't setup interrupt handler [channel creation failed]" );
        return (-1);
    }

    if ( (vpout->irq_coid = ConnectAttach( 0, 0, vpout->irq_chid, _NTO_SIDE_CHANNEL, 0 )) == -1 )
    {
        err = errno;
        goto vpout_isr_fail;
    }

    SIGEV_PULSE_INIT( &vpout->irq_event, vpout->irq_coid, vpout->adapter->pulseprio + 20, VBLANK_PULSE, 0 );
    vpout->irq_iid = InterruptAttach( _NTO_INTR_CLASS_EXTERNAL | vpout->irq, vpout_isr_handler, vpout_draw, sizeof( *vpout_draw ), 
                                      _NTO_INTR_FLAGS_END | _NTO_INTR_FLAGS_TRK_MSK | _NTO_INTR_FLAGS_PROCESS );
    if ( vpout->irq_iid == -1 )
    {
        err = errno;
        goto vpout_isr_fail2;
    }

    /* Enable IRQs: see register LCDINTMASK in the vpout_hw_configure_display() */

    return (0);

vpout_isr_fail2:
    disp_printf( adapter, "[vpoutfb] Error: can't setup interrupt handler [interrupt attaching failed]" );
    ConnectDetach( vpout->irq_coid );
    vpout->irq_coid = -1;

vpout_isr_fail:
    disp_printf( adapter, "[vpoutfb] Error: can't setup interrupt handler [channel connection failed]" );
    ChannelDestroy( vpout->irq_chid );
    vpout->irq_chid = -1;

    errno = err;
    return (-1);
}


void vpout_isr_cleanup( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw )
{
    if ( (vpout->irq_coid != -1) && (vpout->irq_coid != 0) )
        ConnectDetach( vpout->irq_coid );

    if ( (vpout->irq_chid != -1) && (vpout->irq_chid != 0) )
        ChannelDestroy( vpout->irq_chid );

    if ( (vpout->irq_iid != -1) && (vpout->irq_iid != 0) )
        InterruptDetach( vpout->irq_iid );
}


int devg_shmem_size = sizeof( vpout_context_t );


int vpout_init( disp_adapter_t *adapter, char *optstring )
{
    vpout_context_t         *vpout          = NULL;
    vpout_draw_context_t    *vpout_draw     = NULL;
    int                     i               = 0;
    uint8_t                 *gpio_registers = NULL;

    if ( disp_register_adapter( adapter ) == -1 )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: can't register driver" );
        return (-1);
    }

    /* Allocate GPU context */
    vpout = adapter->shmem;
    if ( vpout )
        memset( vpout, 0, sizeof( vpout_context_t ) );
    else {
        vpout = calloc( 1, sizeof( vpout_context_t ) );
        if ( vpout == NULL )
        {
            disp_printf( adapter, "[vpoutfb] Fatal: can't allocate GPU context" );
            goto fail;
        }
        vpout->context_allocated = 1;
    }

    /* Parse configuration options */
    if ( parse_options( adapter, vpout, optstring ) )
        goto fail;

    /* Allocate draw-context */
    vpout_draw = calloc( 1, sizeof( vpout_draw_context_t ) );
    if ( vpout_draw == NULL )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: can't allocate GPU draw-context" );
        goto fail;
    }

    vpout->adapter         = adapter;
    vpout_draw->vpout      = vpout;
    adapter->ms_ctx        = vpout;
    adapter->vsync_counter = &vpout->vsync_counter[0];
    adapter->gd_ctx        = vpout_draw;

    /* Mmap registers */
    vpout->registers = (uint8_t *)mmap64( NULL, vpout->registers_size, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_PHYS, NOFD, vpout->registers_base );
    if ( vpout->registers == MAP_FAILED || vpout->registers == NULL )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: can't mmap registers" );
        goto fail1;
    }
    vpout_draw->registers = vpout->registers;

    /* Get aperture metrics */
    vpout->aperture       = (uint8_t *)mmap64( NULL, vpout->aperture_size, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_PHYS | MAP_ANON, NOFD, 0 );
    if ( vpout->aperture == MAP_FAILED || vpout->aperture == NULL )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: can't mmap aperture" );
        goto fail2;
    }
    if ( mem_offset64( vpout->aperture, NOFD, vpout->aperture_size, (off64_t *)&vpout->aperture_base, NULL ) == -1 )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: can't allcoate GPU memory" );
        goto fail3;
    }
    disp_printf( adapter, "[vpoutfb] GPU memory size: %d Mb", vpout->aperture_size / 1024 / 1024 );

    vpout->cmctr_registers = (uint8_t *)mmap64( NULL, CMCTR_REGISTERS_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_PHYS, NOFD, CMCTR_REGISTERS_BASE );
    if ( vpout->cmctr_registers == MAP_FAILED || vpout->cmctr_registers == NULL )
    {
        disp_printf( adapter, "[vpoutfb] Fatal: can't mmap CMCTR registers" );
        goto fail3;
    }
    /* Check PLLs */
    if ( (*CMCTR_MMIO32( SEL_CPLL ) & SEL_CPLL_LOCK) || ((*CMCTR_MMIO32( SEL_CPLL ) & SEL_CPLL_SEL) == 0) )
        disp_printf( adapter, "[vpoutfb] CPLL frequency: %d MHz", ((*CMCTR_MMIO32( SEL_CPLL ) & SEL_CPLL_SEL) + 1) * VPOUT_XTI_FREQUENCY );
    else {
        disp_printf( adapter, "[vpoutfb] Fatal: CPLL disabled" );
        goto fail4;
    }
    if ( (*CMCTR_MMIO32( SEL_SPLL ) & SEL_SPLL_LOCK) || ((*CMCTR_MMIO32( SEL_SPLL ) & SEL_SPLL_SEL) == 0) )
        disp_printf( adapter, "[vpoutfb] SPLL frequency: %d MHz (288 MHz expected)", ((*CMCTR_MMIO32( SEL_SPLL ) & SEL_SPLL_SEL) + 1) * VPOUT_XTI_FREQUENCY );
    else {
        disp_printf( adapter, "[vpoutfb] Fatal: SPLL disabled" );
        goto fail4;
    }

    for ( i = 0; i < vpout->hdmi_count; i++ )
    {
        if ( gpio_registers == NULL )
        {
            gpio_registers = (uint8_t *)mmap64( NULL, GPIO_REGISTERS_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_PHYS, NOFD,
                                                vpout->hdmi[i].device.it66121.base );
            if ( gpio_registers == MAP_FAILED || gpio_registers == NULL )
            {
                disp_printf( adapter, "[vpoutfb] Fatal: can't mmap GPIO registers" );
                goto fail4;
            }
        }
        
        vpout->hdmi[i].device.it66121.registers = gpio_registers;
    }

    adapter->adapter_ram  = vpout->aperture_size;
    adapter->caps        |= DISP_CAP_NO_IO_PRIVITY;

    /* Setup ISR */
    if ( vpout_isr_setup( adapter, vpout, vpout_draw ) == -1 )
    {
        disp_printf( adapter, "[vpoutfb] Error: can't attach interrupt handler [switching to vsync polling]" );
        vpout->irq_polling = 1;
    }

    return VPOUT_GPU_PIPES;

fail4:
    munmap( vpout->cmctr_registers, CMCTR_REGISTERS_SIZE );
fail3:
    munmap( vpout->aperture, vpout->aperture_size );
fail2:
    munmap( vpout_draw->registers, vpout->registers_size );
fail1:
    free( vpout_draw );
fail:
    if ( vpout->context_allocated )
        free( vpout );

    disp_unregister_adapter( adapter );

    return (-1);
}


void vpout_fini( disp_adapter_t *adapter )
{
    vpout_context_t         *vpout      = adapter->ms_ctx;
    vpout_draw_context_t    *vpout_draw = adapter->gd_ctx;
    int                     i           = 0;

    /* Done if vpout_init failed */
    if ( adapter->gd_ctx == NULL )
        return;

    /* Disable hardware */
    vpout_hw_disable( vpout, vpout_draw );

    /* Cleanup ISR */
    vpout_isr_cleanup( vpout, vpout_draw );

    /* Free mmapings */
    for ( i = 0; i < vpout->hdmi_count; i++ )
        if ( vpout->hdmi[i].device.it66121.registers )
        {
            munmap( vpout->hdmi[i].device.it66121.registers, GPIO_REGISTERS_SIZE );
            break;
        }
    munmap( vpout->cmctr_registers, CMCTR_REGISTERS_SIZE  );
    munmap( vpout->registers,       vpout->registers_size );
    munmap( vpout->aperture,        vpout->aperture_size  );

    /* Free draw-context */
    free( vpout_draw );
    adapter->gd_ctx = NULL;

    /* Free GPU context */
    if ( vpout->context_allocated )
        free( vpout );
    adapter->ms_ctx = NULL;

    disp_unregister_adapter( adapter );
}
