/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <graphics/vbios.h>
#include <graphics/display.h>
#include <graphics/disputil.h>
#include <graphics/ffb.h>
#include <graphics/rop.h>
#include <sys/time.h>
#include <hw/inout.h>


/*************************************************/
/*                   REGISTERS                   */
/*************************************************/

#include "vpout_regs.h"


/*************************************************/
/*               DEBUG DEFINITIONS               */
/*************************************************/

#define VPOUT_SLOG_LEVEL_0                          0
#define VPOUT_SLOG_LEVEL_INFO                       1
#define VPOUT_SLOG_LEVEL_WARNING                    2
#define VPOUT_SLOG_LEVEL_DEBUG                      3

extern int verbose;

#ifdef disp_printf_debug
#undef disp_printf_debug
#endif
#define disp_printf_debug                           if ( verbose >= VPOUT_SLOG_LEVEL_DEBUG )      disp_printf
#define disp_printf_info                            if ( verbose >= VPOUT_SLOG_LEVEL_INFO )       disp_printf
#define disp_printf_warning                         if ( verbose >= VPOUT_SLOG_LEVEL_WARNING )    disp_printf
#define DEBUG                                       ( verbose >= VPOUT_SLOG_LEVEL_DEBUG )
#define INFO                                        ( verbose >= VPOUT_SLOG_LEVEL_INFO )
#define WARNING                                     ( verbose >= VPOUT_SLOG_LEVEL_WARNING )


/*************************************************/
/*                COMPILE OPTIONS                */
/*************************************************/

/* Enable IRQs support */
//#define ENABLE_IRQ

/* Compile with hw-cursor support */
//#define ENABLE_HW_CURSOR


/*************************************************/
/*                HW DEFINITIONS                 */
/*************************************************/

#define VPOUT_GPU_PIPES                             1
#define VPOUT_HDMI_PORTS                            1
#define VPOUT_DSI_PORTS                             1
#define VPOUT_PORTS                                 2
#define VPOUT_HW_PAGESIZE                           4096

#define VPOUT_HW_PRIMARY_SURFACE_SIZE               0x4000000

#define VPOUT_HW_CURSOR_64x64x2_SIZE                (64 * 64 * 2)                   /* 2 pages (64x64 2bpp) */
#define VPOUT_HW_CURSOR_SIZE                        VPOUT_HW_CURSOR_64x64x2_SIZE

#define VPOUT_ALIGN64BYTES_STRIDE( stride )         ((stride & ~0x3f) + 0x3f + 1)

#define VPOUT_XTI_FREQUENCY                         24 /* MHz */


/*************************************************/
/*                 CONFIGURATION                 */
/*************************************************/

/* vpoutfb.conf::dispmode */
#define VPOUT_DISPMODE_BAD_PIPE( pipe )             (pipe > 0)
#define VPOUT_DISPMODE_LAST_PIPE( pipe )            (pipe == 0)

/* vpoutfb.conf::hdmi */
#define CONFIG_DISPLAY_PORT_TYPE_MASK               (0xf)
#define CONFIG_DISPLAY_PORT_TYPE_SHIFT              (0)
#define DISPLAY_PORT_TYPE_DSI                       (0)
#define DISPLAY_PORT_TYPE_HDMI                      (1)
#define CONFIG_DISPLAY_PORT_TYPE_DSI                (DISPLAY_PORT_TYPE_DSI << CONFIG_DISPLAY_PORT_TYPE_SHIFT)
#define CONFIG_DISPLAY_PORT_TYPE_HDMI               (DISPLAY_PORT_TYPE_HDMI << CONFIG_DISPLAY_PORT_TYPE_SHIFT)
#define DISPLAY_PORT( config )                      ((config & CONFIG_DISPLAY_PORT_TYPE_MASK)  >> CONFIG_DISPLAY_PORT_TYPE_SHIFT)
#define DISPLAY_PORT_NAME( config )                 ((config & CONFIG_DISPLAY_PORT_TYPE_MASK) == DISPLAY_PORT_TYPE_HDMI ? "HDMI" : "DSI")
#define CONFIG_DISPLAY_PORT_INDEX_MASK              (0xf0)
#define CONFIG_DISPLAY_PORT_INDEX_SHIFT             (4)
#define DISPLAY_PORT_INDEX_0                        (0)
#define DISPLAY_PORT_INDEX_1                        (1)
#define CONFIG_DISPLAY_PORT_INDEX_0                 (DISPLAY_PORT_INDEX_0 << CONFIG_DISPLAY_PORT_INDEX_SHIFT)
#define CONFIG_DISPLAY_PORT_INDEX_1                 (DISPLAY_PORT_INDEX_1 << CONFIG_DISPLAY_PORT_INDEX_SHIFT)
#define DISPLAY_PORT_INDEX( config )                ((config & CONFIG_DISPLAY_PORT_INDEX_MASK) >> CONFIG_DISPLAY_PORT_INDEX_SHIFT)

/* vpoutfb.conf::hdmi display configurations */
#define CONFIG_DISPLAY_PORT_HDMI0                   (CONFIG_DISPLAY_PORT_TYPE_HDMI | CONFIG_DISPLAY_PORT_INDEX_0)


/*************************************************/
/*                     TYPES                     */
/*************************************************/


/* Display configuration mask: 0xff
                                 |`-- display port type [HDMI, DSI]
                                 `--- display port index [0, 1] */
typedef uint32_t            vpout_display_conf_t;

typedef struct vpout_context
{
    /* GPU adapter descriptor */
    disp_adapter_t          *adapter;

    /* Hardware configuration */
    uint16_t                gpu_supported_pipes;
    struct vpout_hdmi_transmitter
    {
        uint8_t             assigned;                                       /* Initialization flag */
        char                transmitter[64];                                /* Transmitter name */

        union vpout_hdmi_device
        {
            #define VPOUT_OPT_HDMI_IT66121  "IT66121"
            struct vpout_hdmi_it66121
            {
                /* I2C */
                uint8_t     bus;
                uint16_t    address;
                uint32_t    speed;

                /* GPIO */
                uint8_t     *registers;
                uint64_t    base;
                uint8_t     reg;
                uint8_t     pin;
            }               it66121;
        }                   device;
    }                       hdmi[VPOUT_HDMI_PORTS];
    uint8_t                 hdmi_count;                                     /* Current assigned HDMI ports count (see vpoutfb.conf::hdmi option) */

    /* Driver configuration */
    disp_surface_t          display_surface;
    uint8_t                 display_surface_allocated;
    uint8_t                 display_count;                                  /* Displays configuration (see vpoutfb.conf) */
    vpout_display_conf_t    display[VPOUT_PORTS];                           /* Display port configuration (see vpoutfb.conf) */

    /* [video ptr] MMIO registers */
    uint8_t                 *registers;
    uint64_t                registers_base;
    uint64_t                registers_size;
    uint8_t                 *cmctr_registers;

    /* [video ptr] Video memory global aperture */
    uint8_t                 *aperture;
    uint64_t                aperture_base;
    uint32_t                aperture_size;

    /* Custom mode switcher */
    int                     xres;
    int                     yres;
    int                     bpp;
    int                     refresh;

    /* IRQ processing */
    uint8_t                 irq_polling;
    uint32_t                irq;
    int                     irq_iid;
    int                     irq_chid;
    int                     irq_coid;
#define VBLANK_PULSE                                0x5b                    /* VBLANK IRQ pulse code */
    struct sigevent         irq_event;
    uint32_t                vsync_counter[VPOUT_GPU_PIPES];                 /* VSYNC counter */
    volatile uint32_t       error_counter;                                  /* Interrupt INTERRUPT_OUT_FIFO counter */
    volatile uint32_t       error_reset_counter;                            /* Sisplay controller failed reset counter */

    /* Misc */
    int                     context_allocated;
} vpout_context_t;

typedef struct vpout_draw_context
{
    /* GPU context */
    vpout_context_t         *vpout;

    /* [video ptr] MMIO registers */
    uint8_t                 *registers;                                     /* Device registers base pointer */

    /* External attached process flag */
    int                     external_process;
} vpout_draw_context_t;


/*************************************************/
/*                   PROTOTYPES                  */
/*************************************************/

/* init.c */
int vpout_init( disp_adapter_t *adapter, char *optstring );
void vpout_fini( disp_adapter_t *adapter );

/* mem.c */
int vpout_mem_init( disp_adapter_t *adapter, char *optstring );
void vpout_mem_fini( disp_adapter_t *adapter );
int vpout_mem_reset( disp_adapter_t *adapter, disp_surface_t *surf_ );
disp_surface_t* vpout_alloc_surface( disp_adapter_t *adapter,  int width, int height, unsigned format, unsigned flags, unsigned user_flags );
int vpout_free_surface( disp_adapter_t *adapter, disp_surface_t *surf );
unsigned long vpout_mem_avail( disp_adapter_t *adapter, unsigned flags );
int vpout_query_apertures( disp_adapter_t *adapter, disp_aperture_t *ap );
int vpout_query_surface( disp_adapter_t *adapter, disp_surface_t *surf, disp_surface_info_t *info );
int vpout_get_alloc_info( disp_adapter_t *adapter, int width, int height, unsigned format, unsigned flags, unsigned user_flags, disp_alloc_info_t *info );
int vpout_get_alloc_layer_info( disp_adapter_t *adapter, int dispno[], int layer[], int nlayers, unsigned format, int surface_index,
                                int width, int height, unsigned sflags, unsigned hint_flags, disp_alloc_info_t *info );

/* misc.c */
int vpout_misc_wait_idle( disp_adapter_t *adapter );
int vpout_draw_init( disp_adapter_t *adapter, char *opt );
void vpout_draw_fini( disp_adapter_t *adapter );
void vpout_module_info( disp_adapter_t *adapter, disp_module_info_t *info );
int vpout_attach_external( disp_adapter_t *adapter, disp_aperture_t aper[] );
int vpout_detach_external( disp_adapter_t *adapter );

/* mode.c */
int vpout_get_modeinfo( disp_adapter_t *adapter, int dispno, disp_mode_t mode, disp_mode_info_t *info );
int vpout_get_modelist( disp_adapter_t *adapter, int dispno, disp_mode_t *list, int index, int size );
int vpout_set_mode( disp_adapter_t *adapter, int dispno, disp_mode_t mode, disp_crtc_settings_t *settings, disp_surface_t *surf, unsigned flags );
int vpout_wait_vsync( disp_adapter_t *adapter, int dispno );
int vpout_set_display_offset( disp_adapter_t *adapter, int dispno, unsigned offset, int wait_vsync );

/* vpout.c */
struct sigevent * vpout_hw_isr( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw );
void vpout_hw_pipe_wait_for_vblank( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, int pipe );
void vpout_hw_disable( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw );
void vpout_hw_pipe_set_display_offset( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, int pipe, uint32_t offset );
int vpout_hw_configure_display( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, disp_crtc_settings_t *settings,
                                vpout_display_conf_t display, int pipe, disp_surface_t *surface, disp_mode_t mode );

/* i2c.c */
int i2c_read( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t addr, unsigned short offset, uint8_t *buf, unsigned count );
int i2c_write( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port, uint8_t addr, unsigned short offset, uint8_t *buf, unsigned count );
int i2c_init( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t bus_, uint8_t port );
void i2c_fini( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port );

/* it66121.c */
void it66121_probe( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index );
void it66121_reset( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index );
void it66121_remove( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index );
int it66121_init( vpout_context_t *vpout, vpout_draw_context_t *vpout_draw, uint8_t port_index, uint32_t pixel_clock );

/* options.c */
int parse_options( disp_adapter_t *adapter, vpout_context_t *vpout, char *filename );
