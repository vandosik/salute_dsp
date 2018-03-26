#ifndef _INTELGPU_DEPRECATED_H_INCLUDED
#define _INTELGPU_DEPRECATED_H_INCLUDED


#if !INTELGPU_VERSION_IS_484


/*************************************************/
/*             Intel GPU Identifiers             */
/*************************************************/


/* Available GPU Functions */
#define GPU_HAS_EDRAM                               (__I915_READ( HSW_EDRAM_CAP ) & EDRAM_ENABLED)  /* eLLC EDRAM memory */


/*************************************************/
/*                     TYPES                     */
/*************************************************/


/* GMBUS */

typedef struct intelgpu_gmbus_i2c_msg
{
    uint8_t                     *buf;
    uint16_t                    len;
    uint16_t                    address;
    uint32_t                    flags;
} intelgpu_gmbus_i2c_msg_t;


/* display.c */

/* When platform provides two set of M_N registers for dp, we can program them and switch between them incase of DRRS. But When only one such
 * register is provided, we have to program the required divider value on that registers itself based on the DRRS state. */
enum link_m_n_set
{
    M1_N1 = 0,                                                  /* Program dp_m_n on M1_N1 registers dp_m2_n2 on M2_N2 registers (If supported) */
    M2_N2                                                       /* Program dp_m2_n2 on M1_N1 registers M2_N2 registers are not supported */
};


/* displayport.c */

typedef void *                  intelgpu_dp_descriptor_t;


/* 3D */

/* Shared context */
struct intelgpu_drm_3d;
//typedef struct intelgpu_drm_3d
//{
#define INTELGPU_VERSION_PRE_484_3D_CONTEXT                                                     \
    /* IRQ processing & synchronizing */                                                        \
    bool                        display_irqs_enabled;           /* ValleyView only */           \
                                                                                                \
    /* Features */                                                                              \
    uint32_t                    ellc_size;                      /* EDRAM memory size (eLLC) */  \
                                                                                                \
    /* Power management */                                                                      \
    struct {                                                                                    \
        /* Raw watermark latency values: in 0.1us units for WM0, in 0.5us units for WM1+. */    \
        uint16_t                pri_latency[5];                 /* primary */                   \
        uint16_t                spr_latency[5];                 /* sprite */                    \
        uint16_t                cur_latency[5];                 /* cursor */                    \
                                                                                                \
        /* current hardware state */                                                            \
        union {                                                                                 \
            struct ilk_wm_values                                                                \
            {                                                                                   \
                uint32_t        wm_pipe[3];                                                     \
                uint32_t        wm_lp[3];                                                       \
                uint32_t        wm_lp_spr[3];                                                   \
                uint32_t        wm_linetime[3];                                                 \
                bool            enable_fbc_wm;                                                  \
                enum intel_ddb_partitioning partitioning;                                       \
            }                   hw;                                                             \
                                                                                                \
            struct vlv_wm_values                                                                \
            {                                                                                   \
                struct {                                                                        \
                    uint16_t    primary;                                                        \
                    uint16_t    sprite[2];                                                      \
                    uint8_t     cursor;                                                         \
                }               pipe[3];                                                        \
                                                                                                \
                struct {                                                                        \
                    uint16_t    plane;                                                          \
                    uint8_t     cursor;                                                         \
                }               sr;                                                             \
                                                                                                \
                struct {                                                                        \
                    uint8_t     cursor;                                                         \
                    uint8_t     sprite[2];                                                      \
                    uint8_t     primary;                                                        \
                }               ddl[3];                                                         \
            }                   vlv;                                                            \
        };                                                                                      \
    }                           wm;
//} intelgpu_drm_3d_t;


/*************************************************/
/*                   PROTOTYPES                  */
/*************************************************/

/* drm.c */
void intelgpu_drm_disable_isr();

/* display.c */
void intel_dp_set_m_n( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, int m_n2 );

/* dpll_mgr.c */
void intel_enable_shared_dpll( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_disable_shared_dpll( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );

/* ddi.c */
void * intel_ddi_fini( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder );
void * intel_ddi_init( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, int pipe, uint32_t port_config, int port );
void intel_ddi_get_config( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, uint32_t *pipe_bpp );
bool intel_ddi_compute_config( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, uint32_t pixel_clock, uint32_t *pipe_bpp,
                               char *common_debug_label, int debug_common );
void intel_ddi_pre_pll_enable( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
bool intel_ddi_pll_select( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, char *common_debug_label, int debug_common );
void intel_ddi_pre_enable( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, disp_crtc_settings_t *settings );
void intel_ddi_enable_pipe_clock( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_ddi_set_pipe_settings( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_ddi_enable_transcoder_func( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, int pfit_enabled, int vpol, int hpol );
void intel_ddi_set_vc_payload_alloc( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, bool state );
void intel_enable_ddi( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_ddi_post_disable( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_ddi_disable_pipe_clock( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_ddi_disable_transcoder_func( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_disable_ddi( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intel_ddi_destroy( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void intelgpu_ddi_pll_init( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );

/* hdmi.c */
void intel_hdmi_fini( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_ );
void * intel_hdmi_init( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, uint32_t port_config, int pipe, uint32_t output_reg /* common HDMI register */,
                        uint32_t port_ );
bool intel_hdmi_compute_config( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, void *encoder_, uint32_t pixel_clock,
                                uint32_t *pipe_bpp, char *common_debug_label, int debug_common );

/* displayport.c */
int intel_dp_is_edp( struct intelgpu_drm_3d *ctx, uint32_t port_config );
intelgpu_dp_descriptor_t * intelgpu_dp_init( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, uint32_t port_config, int pipe, uint32_t output_reg,
                                             uint32_t pp_status_reg, uint32_t port_ /* see PORT_* */,
                                             void (*vlv_hw_write_dp_link_train)( disp_adapter_t *adapter, int pipe, uint32_t port_config, uint32_t demph_reg_value,
                                                                                 uint32_t uniqtranscale_reg_value, uint32_t preemph_reg_value ) );
void intelgpu_dp_fini( intelgpu_dp_descriptor_t *dp );
void intel_dp_encoder_destroy( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp );
int intel_dp_detect( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp );
bool intel_dp_compute_config( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp, uint32_t pixel_clock, uint32_t *pipe_bpp,
                              char *common_debug_label, int debug_common );
void intelgpu_dp_force_edp( struct intelgpu_drm_3d *ctx, uint32_t port_config );
int intelgpu_dp_get_edid( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp, void *buf, int size );
void intelgpu_edp_backlight_power( struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp, bool enable );
void intelgpu_dp_enable( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp,
                         int (*vlv_wait_digital_port)( disp_adapter_t *adapter, uint32_t port_config ) );
void intelgpu_dp_disable( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp );
void intelgpu_dp_enable_hw( struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp );
void intelgpu_dp_disable_hw( struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp );
void intelgpu_dp_prepare( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp, int vpol, int hpol );
void intelgpu_dp_set_configuration( disp_adapter_t *adapter, intelgpu_dp_descriptor_t *dp, uint32_t *n, uint32_t *m1, uint32_t *m2, uint32_t *p1, uint32_t *p2,
                                    uint32_t *tu, uint32_t *gmch_m, uint32_t *gmch_n, uint32_t *link_m, uint32_t *link_n, 
                                    char *common_debug_label, int debug_common );
void intel_edp_backlight_power( disp_adapter_t *adapter, struct intelgpu_drm_3d *ctx, intelgpu_dp_descriptor_t *dp, bool enable );


#else   /* INTELGPU_VERSION_IS_484 */
#define INTELGPU_VERSION_PRE_484_3D_CONTEXT
#endif  /* INTELGPU_VERSION_IS_484 */


#endif  /* _INTELGPU_DEPRECATED_H_INCLUDED */
