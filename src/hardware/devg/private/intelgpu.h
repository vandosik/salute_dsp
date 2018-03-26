#ifndef _INTELGPU_H_INCLUDED
#define _INTELGPU_H_INCLUDED


/*************************************************/
/*                COMPILE OPTIONS                */
/*************************************************/


#define INTELGPU_VERSION_416                        (416)       /* 4.1.6 kernel [2015-08-17 - Intel GPU gen 7 support] */
#define INTELGPU_VERSION_484                        (484)       /* 4.8.4 kernel [2016-10-22 - Intel GPU gen 9 support] */
#ifndef INTERNAL_INTELGPU_VERSION
#error INTERNAL_INTELGPU_VERSION not set (see common.h)
#else   /* INTERNAL_INTELGPU_VERSION */
#define INTELGPU_VERSION                            INTERNAL_INTELGPU_VERSION
#define INTELGPU_VERSION_IS_484                     (INTELGPU_VERSION - (0) >= INTELGPU_VERSION_484)
#define IS_INTELGPU_VERSION( version )              (INTELGPU_VERSION - (0) == version)
#endif  /* INTERNAL_INTELGPU_VERSION */


/*************************************************/
/*                    HEADERS                    */
/*************************************************/


#include <errno.h>
#include <stdint.h>
#define __KERNEL__
#ifdef __QNXNTO__
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <inttypes.h>
#include <sys/types.h>
//#define _UNISTD_H_INCLUDED                                      /* to disable pipe() pointer */
#include <unistd.h>
#else
#include <qnx4_helpers.h>
#endif
#include <graphics/display.h>
#include <graphics/disputil.h>
#include <drm/drmP.h>
#include <linux/hdmi.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_dp_dual_mode_helper.h>
#include <drm/drm_dp_helper.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_edid.h>
#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/i915_drm.h>
#include <intelgpu_deprecated.h>


/*************************************************/
/*             Intel GPU Identifiers             */
/*************************************************/


/* GPU Device ID */

/* GPU Core Family */
#define GPU_CORE_UNSUPPORTED                        (0x00000000)
#define GPU_CORE_i965                               (0x00000001)
#define GPU_CORE_IRONLAKE                           (0x00000002)
#define GPU_CORE_SANDYBRIDGE                        (0x00000004)
#define GPU_CORE_IVYBRIDGE                          (0x00000008)
#define GPU_CORE_HASWELL                            (0x00000010)
#define GPU_CORE_VALLEYVIEW                         (0x00000020)
#define GPU_CORE_BROADWELL                          (0x00000040)
#define GPU_CORE_CHERRYVIEW                         (0x00000080 | GPU_CORE_VALLEYVIEW)
#define GPU_CORE_SKYLAKE                            (0x00000100)
#define GPU_CORE_BROXTON                            (0x00000200)
#define GPU_CORE_KABYLAKE                           (0x00000400)
#define GPU_CORE_MAX                                (0x000007ff)
/* GPU Core Subfamily */
#define GPU_CORE_IS_ULTRA                           (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_ATTR_ULTRA)
#define GPU_CORE_IS_HASWELL_ULTRA                   (GPU_IS_HASWELL && GPU_CORE_IS_ULTRA)
#define GPU_CORE_IS_BROADWELL_ULTRA                 (GPU_IS_BROADWELL && GPU_CORE_IS_ULTRA)
#define GPU_CORE_IS_SKYLAKE_ULTRA                   (GPU_IS_SKYLAKE && GPU_CORE_IS_ULTRA)
#define GPU_CORE_IS_KABYLAKE_ULTRA                  (GPU_IS_KABYLAKE && GPU_CORE_IS_ULTRA)
#define GPU_CORE_IS_ULX                             (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_ATTR_ULX)
#define GPU_CORE_IS_HASWELL_ULX                     (GPU_IS_HASWELL && GPU_CORE_IS_ULX)
#define GPU_CORE_IS_BROADWELL_ULX                   (GPU_IS_BROADWELL && GPU_CORE_IS_ULX)
#define GPU_CORE_IS_SKYLAKE_ULX                     (GPU_IS_SKYLAKE && GPU_CORE_IS_ULX)
#define GPU_CORE_IS_KABYLAKE_ULX                    (GPU_IS_KABYLAKE && GPU_CORE_IS_ULX)
#define GPU_CORE_IS_GT3                             (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_ATTR_GT3)
#define GPU_CORE_IS_HASWELL_GT3                     (GPU_IS_HASWELL && GPU_CORE_IS_GT3)
#define GPU_CORE_IS_BROADWELL_GT3                   (GPU_IS_BROADWELL && GPU_CORE_IS_GT3)
#define GPU_CORE_IS_SKYLAKE_GT3                     (GPU_IS_SKYLAKE && GPU_CORE_IS_GT3)
#define GPU_CORE_IS_GT4                             (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_ATTR_GT4)
#define GPU_CORE_IS_SKYLAKE_GT4                     (GPU_IS_SKYLAKE && GPU_CORE_IS_GT4)
/* GPU Core Revisions */
#define GPU_CORE_SKYLAKE_REVID_A0                   (0x0)
#define GPU_CORE_SKYLAKE_REVID_B0                   (0x1)
#define GPU_CORE_SKYLAKE_REVID_C0                   (0x2)
#define GPU_CORE_SKYLAKE_REVID_D0                   (0x3)
#define GPU_CORE_SKYLAKE_REVID_E0                   (0x4)
#define GPU_CORE_SKYLAKE_REVID_F0                   (0x5)
#define GPU_CORE_SKYLAKE_REVID_G0                   (0x6)
#define GPU_CORE_SKYLAKE_REVID_H0                   (0x7)
#define GPU_CORE_BROXTON_REVID_A0                   (0x0)
#define GPU_CORE_BROXTON_REVID_A1                   (0x1)
#define GPU_CORE_BROXTON_REVID_B0                   (0x3)
#define GPU_CORE_BROXTON_REVID_C0                   (0x9)
#define GPU_CORE_KABYLAKE_REVID_A0                  (0x0)
#define GPU_CORE_KABYLAKE_REVID_B0                  (0x1)
#define GPU_CORE_KABYLAKE_REVID_C0                  (0x2)
#define GPU_CORE_KABYLAKE_REVID_D0                  (0x3)
#define GPU_CORE_KABYLAKE_REVID_E0                  (0x4)
/* GPU Core Attributes */
#define GPU_CORE_ATTR_ULTRA                         (0x80000000)
#define GPU_CORE_ATTR_ULX                           (0x40000000)
#define GPU_CORE_ATTR_GT3                           (0x20000000)
#define GPU_CORE_ATTR_GT4                           (0x10000000)
#define GPU_CORE_GEN                                (INTELGPU_3D_GET_CONTEXT_PTR->gpu_gen)
#define GPU_CORE_DEVID                              (INTELGPU_3D_GET_CONTEXT_PTR->gpu_did)
#define GPU_CORE_REVID                              (INTELGPU_3D_GET_CONTEXT_PTR->gpu_rev)
#define GPU_CORE_NUM_PIPES                          ((GPU_IS_IRONLAKE || GPU_IS_SANDYBRIDGE || GPU_IS_VALLEYVIEW) ? 2 : \
                                                    ((GPU_IS_IVYBRIDGE || GPU_IS_HASWELL || GPU_IS_BROADWELL || GPU_IS_CHERRYVIEW || GPU_IS_SKYLAKE || \
                                                      GPU_IS_BROXTON || GPU_IS_KABYLAKE) ? 3 : 0))
#define GPU_CORE_NUM_PLANES( pipe )                 (INTELGPU_3D_GET_CONTEXT_PTR->info.num_sprites[pipe])
#define GPU_CORE_NUM_THREADS_PER_EU                 (7)     /* Skylake/Volume 4 - Configurations.pdf */
#define GPU_CORE_NUM_FPUS_PER_EU                    (8)     /* see wiki "en.wikipedia.org/wiki/List_of_Intel_graphics_processing_units": "Each EU contains 2 x SIMD-4 FPUs" */
/* GPU Core Tests */
#define GPU_IS_i965                                 (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_i965)
#define GPU_IS_IRONLAKE                             (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_IRONLAKE)
#define GPU_IS_SANDYBRIDGE                          (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_SANDYBRIDGE)
#define GPU_IS_IVYBRIDGE                            (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_IVYBRIDGE)
#define GPU_IS_HASWELL                              (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_HASWELL)
#define GPU_IS_VALLEYVIEW                           (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_VALLEYVIEW)
#define GPU_IS_BROADWELL                            (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_BROADWELL)
#define GPU_IS_CHERRYVIEW                           ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_MAX) == GPU_CORE_CHERRYVIEW)
#define GPU_IS_SKYLAKE                              (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_SKYLAKE)
#define GPU_IS_BROXTON                              (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_BROXTON)
#define GPU_IS_KABYLAKE                             (INTELGPU_3D_GET_CONTEXT_PTR->gpu_id & GPU_CORE_KABYLAKE)
#define GPU_IS_GEN5                                 (GPU_IS_IRONLAKE)
#define GPU_IS_GEN6                                 (GPU_IS_SANDYBRIDGE)
#define GPU_IS_GEN7                                 (GPU_IS_IVYBRIDGE || GPU_IS_HASWELL || (GPU_IS_VALLEYVIEW && !GPU_IS_CHERRYVIEW))
#define GPU_IS_GEN8                                 (GPU_IS_BROADWELL || GPU_IS_CHERRYVIEW)
#define GPU_IS_GEN9                                 (GPU_IS_SKYLAKE || GPU_IS_BROXTON || GPU_IS_KABYLAKE)
#define GPU_CORE_SHORT_NAME                         (GPU_IS_i965        ? "i965 [unsupported]" : \
                                                     GPU_IS_IRONLAKE    ? "Ironlake" : \
                                                     GPU_IS_SANDYBRIDGE ? "SandyBridge" : \
                                                     GPU_IS_IVYBRIDGE   ? "IvyBridge" : \
                                                     GPU_IS_HASWELL     ? "Haswell" : \
                                                     GPU_IS_VALLEYVIEW  ? "ValleyView" : \
                                                     GPU_IS_BROADWELL   ? "Broadwell" : \
                                                     GPU_IS_CHERRYVIEW  ? "CherryView" : \
                                                     GPU_IS_SKYLAKE     ? "Skylake" : \
                                                     GPU_IS_BROXTON     ? "Broxton (Apollo Lake)" : \
                                                     GPU_IS_KABYLAKE    ? "Kabylake" : "unknown")
#define GPU_IS_SKYLAKE_REVID_A0                     (GPU_IS_SKYLAKE && (GPU_CORE_REVID == GPU_CORE_SKYLAKE_REVID_A0))
#define GPU_IS_SKYLAKE_REVID_B0                     (GPU_IS_SKYLAKE && (GPU_CORE_REVID == GPU_CORE_SKYLAKE_REVID_B0))
#define GPU_IS_SKYLAKE_REVID_C0                     (GPU_IS_SKYLAKE && (GPU_CORE_REVID == GPU_CORE_SKYLAKE_REVID_C0))
#define GPU_IS_SKYLAKE_REVID_D0                     (GPU_IS_SKYLAKE && (GPU_CORE_REVID == GPU_CORE_SKYLAKE_REVID_D0))
#define GPU_IS_SKYLAKE_REVID_E0                     (GPU_IS_SKYLAKE && (GPU_CORE_REVID == GPU_CORE_SKYLAKE_REVID_E0))
#define GPU_IS_BROXTON_REVID_A0                     (GPU_IS_BROXTON && (GPU_CORE_REVID == GPU_CORE_BROXTON_REVID_A0))
#define GPU_IS_BROXTON_REVID_A1                     (GPU_IS_BROXTON && (GPU_CORE_REVID == GPU_CORE_BROXTON_REVID_A1))
#define GPU_IS_BROXTON_REVID_B0                     (GPU_IS_BROXTON && (GPU_CORE_REVID == GPU_CORE_BROXTON_REVID_B0))
#define GPU_IS_BROXTON_REVID_C0                     (GPU_IS_BROXTON && (GPU_CORE_REVID == GPU_CORE_BROXTON_REVID_C0))
#define GPU_IS_KABYLAKE_REVID_A0                    (GPU_IS_KABYLAKE && (GPU_CORE_REVID == GPU_CORE_KABYLAKE_REVID_A0))
#define GPU_IS_KABYLAKE_REVID_B0                    (GPU_IS_KABYLAKE && (GPU_CORE_REVID == GPU_CORE_KABYLAKE_REVID_B0))
#define GPU_IS_KABYLAKE_REVID_C0                    (GPU_IS_KABYLAKE && (GPU_CORE_REVID == GPU_CORE_KABYLAKE_REVID_C0))
#define GPU_IS_KABYLAKE_REVID_D0                    (GPU_IS_KABYLAKE && (GPU_CORE_REVID == GPU_CORE_KABYLAKE_REVID_D0))
#define GPU_IS_KABYLAKE_REVID_E0                    (GPU_IS_KABYLAKE && (GPU_CORE_REVID == GPU_CORE_KABYLAKE_REVID_E0))
#define GPU_IS_REVID_FOREVER                        (0xff)
#define GPU_IS_REVID( since, until )                (INTELGPU_3D_GET_CONTEXT_PTR->gpu_rev >= (since) && INTELGPU_3D_GET_CONTEXT_PTR->gpu_rev <= (until))
#define GPU_IS_SKYLAKE_REVID( since, until )        (GPU_IS_SKYLAKE  && GPU_IS_REVID( since, until ))
#define GPU_IS_BROXTON_REVID( since, until )        (GPU_IS_BROXTON  && GPU_IS_REVID( since, until ))
#define GPU_IS_KABYLAKE_REVID( since, until )       (GPU_IS_KABYLAKE && GPU_IS_REVID( since, until ))
#define GPU_IS_IVB_GT1                              (GPU_CORE_DEVID == 0x0156 || GPU_CORE_DEVID == 0x0152 || GPU_CORE_DEVID == 0x015a)
/* Available GPU Functions */
#define GPU_HAS_DDI                                 (GPU_IS_HASWELL || GPU_IS_BROADWELL || GPU_IS_SKYLAKE || GPU_IS_BROXTON || GPU_IS_KABYLAKE)
#define GPU_HAS_DP_MST                              (GPU_IS_HASWELL || GPU_IS_BROADWELL || GPU_CORE_GEN >= 9)
#define GPU_HAS_PSR                                 (GPU_IS_HASWELL    || GPU_IS_BROADWELL  || GPU_IS_VALLEYVIEW || GPU_IS_CHERRYVIEW || \
                                                     GPU_IS_SKYLAKE || GPU_IS_KABYLAKE) /* Panel Self Refresh (PSR) - power saving feature */
#define GPU_HAS_DRRS                                (GPU_CORE_GEN >= 7) /* Display Refresh Rate Switching (Intel DRRS) - dynamic slowdown - see http://www.intel.com/content/dam/doc/white-paper/power-management-technologies-for-processor-graphics-display-and-memory-paper.pdf */
#define GPU_HAS_AUX_IRQ                             (GPU_CORE_GEN >= 5) /* DisplayPort AUX Channel IRQ */
#define GPU_HAS_GMBUS_IRQ                           (GPU_CORE_GEN >= 5) /* GMBUS IRQ */
#define GPU_HAS_HW_CONTEXTS                         (GPU_CORE_GEN >= 6) /* Hardware contexts */
#define GPU_HAS_LOGICAL_RING_CONTEXTS               (GPU_CORE_GEN >= 8) /* Logical engine contexts */
#define GPU_HAS_LLC                                 ((GPU_CORE_GEN >= 6) && !GPU_IS_VALLEYVIEW && !GPU_IS_BROXTON)
#define GPU_HAS_SNOOP                               (INTELGPU_3D_GET_CONTEXT_PTR->info.has_snoop)
#ifndef GPU_HAS_EDRAM
#define GPU_HAS_EDRAM                               (!!(INTELGPU_3D_GET_CONTEXT_PTR->edram_cap & EDRAM_ENABLED))  /* eLLC EDRAM memory */
#endif
#define GPU_HAS_L3_DPF                              (GPU_IS_IVYBRIDGE || GPU_IS_HASWELL)        /* DPF = dynamic parity feature */
#define GPU_HAS_FPGA_DBG_UNCLAIMED                  (GPU_IS_HASWELL   || GPU_IS_BROADWELL || GPU_IS_SKYLAKE || GPU_IS_BROXTON || GPU_IS_KABYLAKE)
#define GPU_HAS_RUNTIME_PM                          (GPU_IS_GEN6 || GPU_IS_HASWELL || GPU_IS_BROADWELL || GPU_IS_VALLEYVIEW || GPU_IS_SKYLAKE || \
                                                     GPU_IS_BROXTON || GPU_IS_KABYLAKE) /* Runtime power management */
#define GPU_HAS_CSR                                 (GPU_IS_GEN9)
#define GPU_HAS_GUC                                 (GPU_IS_GEN9)   /* For now, anything with a GuC requires uCode loading, and then supports command submission once
                                                                     * loaded. But these are logically independent properties, so we have separate macros to test them.*/
#define GPU_HAS_GUC_UCODE                           (GPU_HAS_GUC)
#define GPU_HAS_GUC_SCHED                           (GPU_HAS_GUC)
#define GPU_HAS_CORE_RING_FREQ                      (GPU_CORE_GEN >= 6 && !GPU_IS_VALLEYVIEW && !GPU_IS_CHERRYVIEW && !GPU_IS_BROXTON)
#define GPU_HAS_POOLED_EU                           (INTELGPU_3D_GET_CONTEXT_PTR->info.has_pooled_eu)
#define GPU_HAS_GMCH_DISPLAY                        ((GPU_CORE_GEN < 5) || GPU_IS_VALLEYVIEW || GPU_IS_CHERRYVIEW)   /* pre-HD display support style */
#define GPU_USES_PPGTT                              (i915.enable_ppgtt)
#define GPU_USES_FULL_PPGTT                         (i915.enable_ppgtt >= 2)
#define GPU_USES_FULL_48BIT_PPGTT                   (i915.enable_ppgtt == 3)


/* PCH Device ID */

/* PCH ID */
#define INTEL_PCH_DEVICE_ID_MASK                    (0xff00)
#define INTEL_PCH_TYPE_NONE                         (0x0000)
#define INTEL_PCH_TYPE_NOP                          (0x0100)
#define INTEL_PCH_TYPE_IBX                          (0x3b00)    /* Ibex Peak PCH (PCI DID mask)       - GPU_IS_GEN5 */
#define INTEL_PCH_TYPE_CPT                          (0x1c00)    /* CougarPoint PCH (PCI DID mask)     - GPU_CORE_IVYBRIDGE || GPU_IS_GEN6 */
#define INTEL_PCH_TYPE_PPT                          (0x1e00)    /* PantherPoint PCH (PCI DID mask)    - GPU_CORE_IVYBRIDGE || GPU_IS_GEN6 */
#define INTEL_PCH_TYPE_LPT                          (0x8c00)    /* LynxPoint PCH (PCI DID mask)       - GPU_CORE_HASWELL( !ultra ) || GPU_CORE_BROADWELL( !ultra ) */
#define INTEL_PCH_TYPE_LPT_LP                       (0x9c00)    /* LynxPoint LP PCH (PCI DID mask)    - GPU_CORE_HASWELL( ultra )  || GPU_CORE_BROADWELL( ultra ) */
#define INTEL_PCH_TYPE_SPT                          (0xA100)    /* SunrisePoint PCH (PCI DID mask)    - GPU_CORE_SKYLAKE || GPU_CORE_KABYLAKE */
#define INTEL_PCH_TYPE_SPT_LP                       (0x9D00)    /* SunrisePoint LP PCH (PCI DID mask) - GPU_CORE_SKYLAKE || GPU_CORE_KABYLAKE */
#define INTEL_PCH_TYPE_KBP                          (0xA200)    /* KabyPoint PCH (PCI DID mask)       - GPU_CORE_KABYLAKE */
/* PCH Tests */
#define GPU_PCH_IS_IBX                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_IBX)
#define GPU_PCH_IS_CPT                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_CPT)
#define GPU_PCH_IS_PPT                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_PPT)
#define GPU_PCH_IS_LPT                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_LPT)
#define GPU_PCH_IS_LPT_LP                           ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_LPT_LP)
#define GPU_PCH_IS_SPT                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_SPT)
#define GPU_PCH_IS_SPT_LP                           ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_SPT_LP)
#define GPU_PCH_IS_KBP                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_KBP)
#define GPU_PCH_IS_NOP                              ((INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch & INTEL_PCH_DEVICE_ID_MASK) == INTEL_PCH_TYPE_NOP)
#define GPU_PCH_IS_SUPPORTED                        (GPU_PCH_IS_IBX || GPU_PCH_IS_CPT || GPU_PCH_IS_PPT || GPU_PCH_IS_LPT || GPU_PCH_IS_LPT_LP || \
                                                     GPU_PCH_IS_SPT || GPU_PCH_IS_SPT_LP || GPU_PCH_IS_KBP)
#define GPU_PCH_IS_NONE                             ((!(GPU_PCH_IS_IBX || GPU_PCH_IS_CPT || GPU_PCH_IS_PPT || GPU_PCH_IS_LPT || GPU_PCH_IS_SPT || GPU_PCH_IS_SPT_LP || GPU_PCH_IS_KBP || GPU_PCH_IS_NOP)) || \
                                                       (INTELGPU_3D_GET_CONTEXT_PTR->gpu_pch == INTEL_PCH_TYPE_NONE))
/* Available GPU PCH Device */
#define GPU_HAS_PCH_NOP                             (GPU_PCH_IS_NOP)        /* Quanta IvyBridge transcode only */
#define GPU_HAS_PCH_IBX                             (GPU_PCH_IS_IBX)
#define GPU_HAS_PCH_CPT                             (GPU_PCH_IS_CPT || GPU_PCH_IS_PPT)
#define GPU_HAS_PCH_LPT                             (GPU_PCH_IS_LPT)
#define GPU_HAS_PCH_LPT_LP                          (GPU_PCH_IS_LPT_LP)
#define GPU_HAS_PCH_LPT_H                           (GPU_PCH_IS_LPT)
#define GPU_HAS_PCH_SPT                             (GPU_PCH_IS_SPT || GPU_PCH_IS_SPT_LP)
#define GPU_HAS_PCH_KBP                             (GPU_PCH_IS_KBP)
#define GPU_HAS_PCH_SPLIT                           (GPU_PCH_IS_SUPPORTED)  /* GPU function splitted between PCH- (PCI class = ISA Bridge) and GPU-
                                                                             * (PCI class = Display) PCI devices */


/* GPU Port Identification */

/* GPU Port Types */
#define CONFIG_DISPLAY_PORT_MASK                    (0x0f)
#define CONFIG_DISPLAY_PORT_NONE                    (0x00)      /* Special case for atomic modeswitcher disable sequence */
#define CONFIG_DISPLAY_PORT_CRT                     (0x01)
#define CONFIG_DISPLAY_PORT_DVI                     (0x02)
#define CONFIG_DISPLAY_PORT_HDMI                    (0x03)
#define CONFIG_DISPLAY_PORT_LVDS                    (0x04)
#define CONFIG_DISPLAY_PORT_DSI                     (0x05)      /* Display Serial Interface (DSI) by Mobile Industry Processor Interface (MIPI) Alliance */
#define CONFIG_DISPLAY_PORT_DP                      (0x06)
#define CONFIG_DISPLAY_PORT_DP_MST                  (0x07)      /* DisplayPort with multi-streaming */
#define CONFIG_DISPLAY_PORT_eDP                     (0x08)
#define CONFIG_DISPLAY_PORT_VARIANT_MASK            (0xf0)
#define CONFIG_DISPLAY_PORT_VARIANT_A               (0x00)
#define CONFIG_DISPLAY_PORT_VARIANT_B               (0x10)
#define CONFIG_DISPLAY_PORT_VARIANT_C               (0x20)
#define CONFIG_DISPLAY_PORT_VARIANT_D               (0x30)
#define CONFIG_DISPLAY_PORT_VARIANT_E               (0x40)
#define CONFIG_DISPLAY_MASK                         (0xff)
/* GPU Ports */
#define CONFIG_DISPLAY_CRT                          (CONFIG_DISPLAY_PORT_CRT)
#define CONFIG_DISPLAY_DVI_B                        (CONFIG_DISPLAY_PORT_DVI  | CONFIG_DISPLAY_PORT_VARIANT_B)
#define CONFIG_DISPLAY_DVI_C                        (CONFIG_DISPLAY_PORT_DVI  | CONFIG_DISPLAY_PORT_VARIANT_C)
#define CONFIG_DISPLAY_DVI_D                        (CONFIG_DISPLAY_PORT_DVI  | CONFIG_DISPLAY_PORT_VARIANT_D)
#define CONFIG_DISPLAY_DVI_E                        (CONFIG_DISPLAY_PORT_DVI  | CONFIG_DISPLAY_PORT_VARIANT_E)  /* Skylake+ */
#define CONFIG_DISPLAY_HDMI_B                       (CONFIG_DISPLAY_PORT_HDMI | CONFIG_DISPLAY_PORT_VARIANT_B)
#define CONFIG_DISPLAY_HDMI_C                       (CONFIG_DISPLAY_PORT_HDMI | CONFIG_DISPLAY_PORT_VARIANT_C)
#define CONFIG_DISPLAY_HDMI_D                       (CONFIG_DISPLAY_PORT_HDMI | CONFIG_DISPLAY_PORT_VARIANT_D)
#define CONFIG_DISPLAY_HDMI_E                       (CONFIG_DISPLAY_PORT_HDMI | CONFIG_DISPLAY_PORT_VARIANT_E)  /* Skylake+ */
#define CONFIG_DISPLAY_DP_A                         (CONFIG_DISPLAY_PORT_DP   | CONFIG_DISPLAY_PORT_VARIANT_A)
#define CONFIG_DISPLAY_DP_B                         (CONFIG_DISPLAY_PORT_DP   | CONFIG_DISPLAY_PORT_VARIANT_B)
#define CONFIG_DISPLAY_DP_C                         (CONFIG_DISPLAY_PORT_DP   | CONFIG_DISPLAY_PORT_VARIANT_C)
#define CONFIG_DISPLAY_DP_D                         (CONFIG_DISPLAY_PORT_DP   | CONFIG_DISPLAY_PORT_VARIANT_D)
#define CONFIG_DISPLAY_DP_E                         (CONFIG_DISPLAY_PORT_DP   | CONFIG_DISPLAY_PORT_VARIANT_E)  /* Skylake+ */
#define CONFIG_DISPLAY_eDP_A                        (CONFIG_DISPLAY_PORT_eDP  | CONFIG_DISPLAY_PORT_VARIANT_A)
#define CONFIG_DISPLAY_eDP_B                        (CONFIG_DISPLAY_PORT_eDP  | CONFIG_DISPLAY_PORT_VARIANT_B)
#define CONFIG_DISPLAY_eDP_C                        (CONFIG_DISPLAY_PORT_eDP  | CONFIG_DISPLAY_PORT_VARIANT_C)
#define CONFIG_DISPLAY_eDP_D                        (CONFIG_DISPLAY_PORT_eDP  | CONFIG_DISPLAY_PORT_VARIANT_D)
#define CONFIG_DISPLAY_eDP_E                        (CONFIG_DISPLAY_PORT_eDP  | CONFIG_DISPLAY_PORT_VARIANT_E)
/* GPU Port Tests */
#define DISPLAY_PORT( config )                      (config & CONFIG_DISPLAY_MASK)
#define DISPLAY_PORT_TYPE( config )                 (config & CONFIG_DISPLAY_PORT_MASK)
#define DISPLAY_PORT_TYPE_NAME( config )            (DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_NONE ? "NONE" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_CRT ? " CRT" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_DVI ? " DVI" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_HDMI ? "HDMI" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_LVDS ? "LVDS" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_DSI ? " DSI" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_DP ? "  DP" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_DP_MST ? "DPmst" : \
                                                     DISPLAY_PORT_TYPE( config ) == CONFIG_DISPLAY_PORT_eDP ? " eDP" : "?")
#define DISPLAY_PORT_VARIANT( config )              (config & CONFIG_DISPLAY_PORT_VARIANT_MASK)
#define DISPLAY_PORT_VARIANT_NAME( config )         ((config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == 0                             ? "A" : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_B ? "B" : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_C ? "C" : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_D ? "D" : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_E ? "E" : "?")
#define DISPLAY_PORT_INDEX( config )                ((config & CONFIG_DISPLAY_PORT_VARIANT_MASK) >> 4)


/* GPU pipes */
#define INVALID_PIPE                                (-1)
#define PIPE_A                                      (0)
#define PIPE_B                                      (1)
#define PIPE_C                                      (2)
#define _PIPE_EDP                                   (3)         /* Exclusive Haswell/Broadwell/Skylake/Kabylake pipe for PORT_A (eDP) */
#define PIPE_MAX                                    (GPU_IS_VALLEYVIEW ? PIPE_B : PIPE_C)
#define I915_MAX_PIPES                              (_PIPE_EDP)
#define pipe_name( pipe )                           ((pipe) + 'A')


/* Transcoders (pseudonames of GPU pipes [except Haswell: it have special EDP-transcoder for PORT_A]) */
#define TRANSCODER_A                                (0)
#define TRANSCODER_B                                (1)
#define TRANSCODER_C                                (2)
#define TRANSCODER_EDP                              (3)
#define TRANSCODER_DSI_A                            (4)
#define TRANSCODER_DSI_C                            (5)
#define I915_MAX_TRANSCODERS                        (6)
#define transcoder_name( transcoder )               (transcoder == TRANSCODER_A     ? "A"     :             \
                                                     transcoder == TRANSCODER_B     ? "B"     :             \
                                                     transcoder == TRANSCODER_C     ? "C"     :             \
                                                     transcoder == TRANSCODER_EDP   ? "EDP"   :             \
                                                     transcoder == TRANSCODER_DSI_A ? "DSI A" :             \
                                                     transcoder == TRANSCODER_DSI_C ? "DSI C" : "<invalid>")


/* This is the maximum (across all platforms) number of planes (primary + sprites) that can be active at the same time on one pipe. */
#define PLANE_A                                     (0)
#define PLANE_B                                     (1)
#define PLANE_C                                     (2)
#define PLANE_CURSOR                                (3)
#define I915_MAX_PLANES                             (4)
#define plane_name( p )                             ((p) + 'A')


/*************************************************/
/*                    DDC/EDID                   */
/*************************************************/


#define DDC_I2C_SLAVE_EDID                          0x50
#define DDC_I2C_SLAVE_SEGMENT                       0x30

#define DDC_I2C_EDID_LENGTH                         128


/* intelgpu_get_edid() modes */

#define GET_EDID_MODE_GPIO                          0x0
#define GET_EDID_MODE_GMBUS                         0x1


/*************************************************/
/*                     GMBUS                     */
/*************************************************/


#define GMBUS_BAD_PORT                              0x80000000

/* GMBUS I2C flags */
#define GMBUS_I2C_XFER_FLAG_ADDR10                  0x00000001  /* I2C slave - 10-bits address */
#define GMBUS_I2C_XFER_FLAG_IGNORE_NAK              0x00000002  /* Ignore NAK: Normally message is interrupted immediately if there is [NA] from the client.
                                                                 * Setting this flag treats any [NA] as [A], and all of message is sent. These messages may
                                                                 * still fail to SCL lo->hi timeout. */
#define GMBUS_I2C_XFER_FLAG_RECV_LEN                0x00000004  /* Length will be first received byte */
#define GMBUS_I2C_XFER_FLAG_NO_RD_ACK               0x00000008  /* In a read message, master A/NA bit is skipped */
#define GMBUS_I2C_XFER_FLAG_REV_DIR_ADDR            0x00000010  /* This toggles the Rd/Wr flag. That is, if you want to do a write, but
                                                                 * need to emit an Rd instead of a Wr, or vice versa, you set this flag.
                                                                 * Example: [S Addr Rd [A] Data [A] Data [A] ... [A] Data [A] P] */
#define GMBUS_I2C_XFER_FLAG_NOSTART                 0x00000020  /* In a combined transaction, no 'S Addr Wr/Rd [A]' is generated at some point. For example,
                                                                 * setting GMBUS_I2C_XFER_FLAG_NOSTART on the second partial message generates: [S Addr Rd [A]
                                                                 * [Data] NA Data [A] P]. If you set the GMBUS_I2C_XFER_FLAG_NOSTART variable for the first
                                                                 * partial message, we do not generate Addr, but we do generate the startbit S. This will
                                                                 * probably confuse all other clients on your bus, so don't try this. This is often used to
                                                                 * gather transmits from multiple data buffers in system memory into something that appears as
                                                                 * a single transfer to the I2C device but may also be used between direction changes by some
                                                                 * rare devices. */
#define GMBUS_I2C_XFER_FLAG_READ                    0x00008000  /* Read data from slave to master */
#define GMBUS_I2C_XFER_FLAG_WRITE                   0x00000000  /* Write data to slave from master */


/*************************************************/
/*                 Digital ports                 */
/*************************************************/


#define PORT_A                                      0           /* Digital port A (DP/eDP on Haswell+) */
#define PORT_B                                      1           /* Digital port B (DVI/HDMI/DP/eDP) */
#define PORT_C                                      2           /* Digital port C (DVI/HDMI/DP/eDP) */
#define PORT_D                                      3           /* Digital port D (DVI/HDMI/DP) */
#define PORT_E                                      4           /* Digital port E (DVI/HDMI/DP on Skylake/Kabylake) */
#define I915_MAX_PORTS                              (PORT_E + 1)
#define PORT_NAME( port )                           (port == PORT_A ? "A" : port == PORT_B ? "B" : port == PORT_C ? "C" : \
                                                     port == PORT_D ? "D" : port == PORT_E ? "E" : "?")

/* GPU Port Type (variant field) to digital port */
#define PORT( config )                              ((config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_A ? PORT_A : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_B ? PORT_B : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_C ? PORT_C : \
                                                     (config & CONFIG_DISPLAY_PORT_VARIANT_MASK) == CONFIG_DISPLAY_PORT_VARIANT_D ? PORT_D : PORT_E)

/* Digital port to GPU Port Type (variant field) */
#define VARIANT( port )                             (port == PORT_A ? CONFIG_DISPLAY_PORT_VARIANT_A : port == PORT_B ? CONFIG_DISPLAY_PORT_VARIANT_B : \
                                                     port == PORT_C ? CONFIG_DISPLAY_PORT_VARIANT_C : port == PORT_D ? CONFIG_DISPLAY_PORT_VARIANT_D : \
                                                     port == PORT_E ? CONFIG_DISPLAY_PORT_VARIANT_E : 0)


/*************************************************/
/*              GPU memory domains               */
/*************************************************/


#define INTELGPU_MEMORY_DOMAIN_ANY                  (0)         /* Memory in any domain - [aperture][extended memory] */
#define INTELGPU_MEMORY_DOMAIN_APERTURE             (1)         /* Aperture memory */
#define INTELGPU_MEMORY_DOMAIN_EXTENDED             (2)         /* Extended memory */
#define INTELGPU_MEMORY_DOMAINS_COUNT               (2)

#ifndef DISP_SURFACE_ZONE_SHIFT
#define DISP_SURFACE_ZONE_SHIFT                     (24)
#endif
#define INTELGPU_MEMORY_DOMAIN_ANY_FLAG             (INTELGPU_MEMORY_DOMAIN_ANY << DISP_SURFACE_ZONE_SHIFT)
#define INTELGPU_MEMORY_DOMAIN_APERTURE_FLAG        (INTELGPU_MEMORY_DOMAIN_APERTURE << DISP_SURFACE_ZONE_SHIFT)
#define INTELGPU_MEMORY_DOMAIN_EXTENDED_FLAG        (INTELGPU_MEMORY_DOMAIN_EXTENDED << DISP_SURFACE_ZONE_SHIFT)

#define INTELGPU_MEMORY_DOMAIN_NAME( domain )       (domain == INTELGPU_MEMORY_DOMAIN_APERTURE ?      "aperture" : \
                                                     domain == INTELGPU_MEMORY_DOMAIN_APERTURE_FLAG ? "aperture" : \
                                                     domain == INTELGPU_MEMORY_DOMAIN_EXTENDED ?      "extended" : \
                                                     domain == INTELGPU_MEMORY_DOMAIN_EXTENDED_FLAG ? "extended" : "undefined")


/*************************************************/
/*                       3D                      */
/*************************************************/


/* 3D context descriptor initialization */
#define INTELGPU_3D_CONTEXT                         intelgpu_drm_3d_t intelgpu_3d_ctx;

/* 3D context pointers */
#ifdef INTELGPU_DRIVER_MODE
#ifdef INTELGPU_DRIVER_NAME
#define INTELGPU_3D_GET_CONTEXT( driver )           &(driver->intelgpu_3d_ctx)
#define INTELGPU_3D_GET_CONTEXT_PTR                 ((intelgpu_drm_3d_t *)INTELGPU_3D_GET_CONTEXT( INTELGPU_DRIVER_NAME ))
#else
#error [intelgpu] INTELGPU_DRIVER_MODE & INTELGPU_DRIVER_NAME must be defined
#endif
#else
#define INTELGPU_3D_GET_CONTEXT_PTR                 (ctx)
#endif

/* 3D context engine lock/unlock functions */
#ifdef __QNXNTO__
#define INTELGPU_3D_CONTEXT_RING_LOCK(   driver, rid )  if ( driver->intelgpu_3d_ctx.engine[rid].initialized ) \
                                                            pthread_mutex_lock( &(driver->intelgpu_3d_ctx.engine[rid].ring_mutex) );
#define INTELGPU_3D_CONTEXT_RING_TMDLOCK( driver, rid ) if ( driver->intelgpu_3d_ctx.engine[rid].initialized ) { \
                                                            int             status = 0; \
                                                            struct timespec timeout; \
                                                            clock_gettime( CLOCK_REALTIME, &timeout ); \
                                                            timeout.tv_sec += 2; \
                                                            status = pthread_mutex_timedlock( &(driver->intelgpu_3d_ctx.engine[rid].ring_mutex), &timeout ); \
                                                            if ( status != 0 ) \
                                                                disp_printf( adapter, "[driver] Error: engine[rid=%d] mutex lock failed [%s(): code=0x%x]", \
                                                                             rid, __FUNCTION__, status ); }
#define INTELGPU_3D_CONTEXT_RING_UNLOCK( driver, rid )  if ( driver->intelgpu_3d_ctx.engine[rid].initialized ) \
                                                            pthread_mutex_unlock( &(driver->intelgpu_3d_ctx.engine[rid].ring_mutex) );
#define INTELGPU_3D_CONTEXT_REGS_LOCK(   driver )       if ( driver->intelgpu_3d_ctx.valid ) \
                                                            pthread_mutex_lock( &(driver->intelgpu_3d_ctx.reg_mutex) );
#define INTELGPU_3D_CONTEXT_REGS_TMDLOCK( driver, rid ) if ( driver->intelgpu_3d_ctx.valid ) { \
                                                            int             status = 0; \
                                                            struct timespec timeout; \
                                                            clock_gettime( CLOCK_REALTIME, &timeout ); \
                                                            timeout.tv_sec += 2; \
                                                            status = pthread_mutex_timedlock( &(driver->intelgpu_3d_ctx.reg_mutex), &timeout ); \
                                                            if ( status != 0 ) \
                                                                disp_printf( adapter, "[driver] Error: registers mutex lock failed [%s(): code=0x%x]", \
                                                                             __FUNCTION__, status ); }
#define INTELGPU_3D_CONTEXT_REGS_UNLOCK( driver )       if ( driver->intelgpu_3d_ctx.valid ) \
                                                            pthread_mutex_unlock( &(driver->intelgpu_3d_ctx.reg_mutex) );
#else
#define INTELGPU_3D_CONTEXT_RING_LOCK(   driver, rid )  ;
#define INTELGPU_3D_CONTEXT_RING_UNLOCK( driver, rid )  ;
#define INTELGPU_3D_CONTEXT_REGS_LOCK(   driver )   ;
#define INTELGPU_3D_CONTEXT_REGS_UNLOCK( driver )   ;
#endif

/* 3D context engine lock/unlock lib functions */
#ifdef __QNXNTO__
#define INTELGPU_3D_CONTEXT_LIB_RING_LOCK(   ctx, rid ) if ( ctx->engine[rid].initialized ) \
                                                            pthread_mutex_lock( &(ctx->engine[rid].ring_mutex) );
#define INTELGPU_3D_CONTEXT_LIB_RING_UNLOCK( ctx, rid ) if ( ctx->engine[rid].initialized ) \
                                                            pthread_mutex_unlock( &(ctx->engine[rid].ring_mutex) );
#define INTELGPU_3D_CONTEXT_LIB_REGS_LOCK(   ctx )      if ( ctx->valid ) \
                                                            pthread_mutex_lock( &(ctx->reg_mutex) );
#define INTELGPU_3D_CONTEXT_LIB_REGS_UNLOCK( ctx )      if ( ctx->valid ) \
                                                            pthread_mutex_unlock( &(ctx->reg_mutex) );
#else
#define INTELGPU_3D_CONTEXT_LIB_RING_LOCK(   ctx, rid ) ;
#define INTELGPU_3D_CONTEXT_LIB_RING_UNLOCK( ctx, rid ) ;
#define INTELGPU_3D_CONTEXT_LIB_REGS_LOCK(   ctx )  ;
#define INTELGPU_3D_CONTEXT_LIB_REGS_UNLOCK( ctx )  ;
#endif

/* GPU power/frequency profiles [see intelgpu_gpu_frequency_update()] */
#define INTELGPU_3D_GPU_FREQUENCY_PROFILE_IDLE          (0x1)
#define INTELGPU_3D_GPU_FREQUENCY_PROFILE_EFFECTIVE     (0x2)
#define INTELGPU_3D_GPU_FREQUENCY_PROFILE_PERFORMANCE   (0x3)
#define INTELGPU_3D_GPU_FREQUENCY_PROFILE_OVERCLOCK     (0x4)


/*************************************************/
/*                     TYPES                     */
/*************************************************/


/* Intel GPU Identifiers */

typedef uint32_t                intelgpu_port_id_t;


/* Intel VBIOS descriptors */
typedef struct intel_vbt_data
{
    struct {
        uint16_t                port_mask;                      /* (1 << PORT_*) mask */
        void                    *pps;                           /* Panel's vbios.h::struct edp_power_seq pointer */

        uint8_t                 bpp;                            /* 18/24/30 */
        uint16_t                rate;                           /* 162 = 1.62 Gbps; 270 = 2.70 Gbps */
        uint8_t                 lanes;                          /* 1/2/4 */
        uint8_t                 preemphasis;
        uint8_t                 vswing;
        uint8_t                 low_vswing;
    }                           edp;

    struct {
        uint8_t                 min_brightness;
        uint16_t                pwm_freq_hz;

        /* Fixed mode */
        uint8_t                 fixed_mode;                     /* flag */
        uint32_t                clk;                            /* Clock (kHz) */
        uint32_t                hsp;                            /* hsync pol */
        uint32_t                ht;                             /* htotal */
        uint32_t                hd;                             /* hdisp */
        uint32_t                hss;                            /* hsync start */
        uint32_t                hse;                            /* hsync end */
        uint32_t                vsp;                            /* vsync pol */
        uint32_t                vt;                             /* vtotal */
        uint32_t                vd;                             /* vdisp */
        uint32_t                vss;                            /* vsync start */
        uint32_t                vse;                            /* vsync end */
    }                           lfp;

    struct ddi_vbt_port_info
    {
    #define HDMI_LEVEL_SHIFT_UNKNOWN    0xff
        uint8_t                 hdmi_level_shift;               /* This is an index in the HDMI/DVI DDI buffer translation table.The special
                                                                 * value HDMI_LEVEL_SHIFT_UNKNOWN means the VBT didn't populate this field. */

        bool                    supports_dvi;
        bool                    supports_hdmi;
        bool                    supports_dp;

    #define DP_AUX_A                    0x40
    #define DP_AUX_B                    0x10
    #define DP_AUX_C                    0x20
    #define DP_AUX_D                    0x30
        uint8_t                 alternate_aux_channel;
    #define DDC_PIN_B                   0x05
    #define DDC_PIN_C                   0x04
    #define DDC_PIN_D                   0x06
        uint8_t                 alternate_ddc_pin;

        uint8_t                 dp_boost_level;
        uint8_t                 hdmi_boost_level;
    }                           ddi_port_info[I915_MAX_PORTS];
} intel_vbt_data_t;


/* Cache control */

enum i915_cache_level
{
    I915_CACHE_NONE = 0,
    I915_CACHE_LLC,                                             /* also used for snoopable memory on non-LLC */
    I915_CACHE_L3_LLC,                                          /* gen7+, L3 sits between the domain specifc caches, eg sampler/render caches, and the
                                                                 * large Last-Level-Cache. LLC is coherent with the CPU, but L3 is only visible to the GPU. */
    I915_CACHE_WT,                                              /* hsw:gt3e WriteThrough for scanouts */
};


/* Forcewake control */

enum forcewake_domain_id
{
    FW_DOMAIN_ID_RENDER = 0,
    FW_DOMAIN_ID_BLITTER,
    FW_DOMAIN_ID_MEDIA,

    FW_DOMAIN_ID_COUNT
};

enum forcewake_domains
{
    FORCEWAKE_RENDER    = (1 << FW_DOMAIN_ID_RENDER),
    FORCEWAKE_BLITTER   = (1 << FW_DOMAIN_ID_BLITTER),
    FORCEWAKE_MEDIA     = (1 << FW_DOMAIN_ID_MEDIA),
    FORCEWAKE_ALL       = (FORCEWAKE_RENDER | FORCEWAKE_BLITTER | FORCEWAKE_MEDIA)
};

#if INTELGPU_VERSION_IS_484
struct intel_uncore
{
#ifndef __QNX__
    struct intel_uncore_funcs   funcs;

    unsigned                fifo_count;
#endif
    enum forcewake_domains  fw_domains;

    struct intel_uncore_forcewake_domain
    {
        enum forcewake_domain_id    id;
        enum forcewake_domains      mask;
        unsigned            wake_count;
        uint32_t            reg_set;
        uint32_t            val_set;
        uint32_t            val_clear;
        uint32_t            reg_ack;
        uint32_t            reg_post;
        uint32_t            val_reset;
    }                       fw_domain[FW_DOMAIN_ID_COUNT];
};
#endif


/* GMBUS */

typedef uint32_t (* intelgpu_gmbus_port_mapping_t)( disp_adapter_t *adapter, intelgpu_port_id_t port, uint8_t silent );
typedef char *   (* intelgpu_gmbus_port_name_t)( disp_adapter_t *adapter, intelgpu_port_id_t port );


/* GPIO */

typedef uint32_t * (* intelgpu_gpio_port_mapping_t)( disp_adapter_t *adapter, intelgpu_port_id_t port, uint8_t silent );
typedef char *     (* intelgpu_gpio_port_name_t)( disp_adapter_t *adapter, intelgpu_port_id_t port );


/* 3D */

typedef disp_surface_t * (* alloc_surface_callback_t)( disp_adapter_t *adapter, int width, int height, unsigned format, unsigned sflags, unsigned hint_flags );
struct intelgpu_drm_3d;

typedef struct drm_i915_fence_reg
{
    void                        *obj;                           /* assigned object (drm_i915_gem_object) in the local memory */
    int                         pin_count;
#ifdef __QNX__
    pid_t                       owner;                          /* assigned object owner's pid */
    uint8_t                     id;
#endif
} fence_reg_t;

typedef struct ring_shared
{
    int                         initialized;                    /* Ring mutex initialized flag */

#ifdef __QNXNTO__
    pthread_mutex_t             ring_mutex;                     /* Shared engine mutex */
#endif
    uint32_t                    atomic;                         /* Is it safe to write to the engine */

    /* Pointers */
    uint32_t                    head;
    uint32_t                    tail;
    int                         space;
    uint32_t                    last_retired_head;              /* We track the position of the requests in the engine buffer, and when each is retired we increment
                                                                 * last_retired_head as the GPU must have finished processing the request and so we know we can
                                                                 * advance the ringbuffer up to that position. last_retired_head is set to -1 after the value is
                                                                 * consumed so we can detect new retirements. */
} ring_shared_t;

typedef struct blitter_forwarding
{
    void                        (* allocate)( disp_adapter_t *adapter, uint32_t len );
    void                        (* write)( disp_adapter_t *adapter, uint32_t data );
    void                        (* finish)( disp_adapter_t *adapter );
    void                        (* push)( disp_adapter_t *adapter );
    void                        (* pop)( disp_adapter_t *adapter );
    int                         (* wait_idle)( disp_adapter_t *adapter );
} blitter_forwarding_t;

typedef struct sync_descriptor
{
    /* IRQ */
    volatile int                irq;
    volatile int                irq_id;

    /* Driver's ISR thread */
#ifdef __QNXNTO__
    pthread_t                   thread;
    pthread_attr_t              thread_attr;
#endif

    /* ISR channel */
    int                         channel;
    int                         connection;

    /* Common events */
#ifdef __QNXNTO__
    #define PULSE_CODE_SYNC                     (_PULSE_CODE_MINAVAIL + 0x10)
    struct sigevent             sync_event;
    #define PULSE_CODE_EXIT                     (_PULSE_CODE_MINAVAIL + 0x11)
    struct sigevent             exit_event;
    #define PULSE_CODE_REG_MUTEX_DEAD           (_PULSE_CODE_MINAVAIL + 0x12)
    struct sigevent             reg_mutex_event;
    #define PULSE_CODE_RING_MUTEX_DEAD( ring_id )   (_PULSE_CODE_MINAVAIL + 0x13 + ring_id)
    struct sigevent             ring_mutex_event[0x10 /* > I915_NUM_RINGS */];
    /* User's events */
        /* vsync */
        struct sigevent         *vblank_event;
        uint32_t                *vsync_counters;
        int                     vsync_counters_number;

    /* Shared condvar */
    pthread_mutex_t             mutex;
    pthread_cond_t              condvar;
    pthread_mutexattr_t         mutex_attr;
    pthread_condattr_t          condvar_attr;
#endif

    /* vsync support */
    uint16_t                    vblank_pipes;

    /* Sync primitive */
    uint16_t                    sync_counter;                   /* Client's sync counter */

    uint8_t                     initialized;
    uint8_t                     fallbak_to_polling;
} intelgpu_drm_3d_sync_descriptor_t;

struct i915_gem_mm
{
    uint32_t                    bit_6_swizzle_x;                /* Bit 6 swizzling required for X tiling */
    uint32_t                    bit_6_swizzle_y;                /* Bit 6 swizzling required for Y tiling */
};

typedef struct gpu_pm_descriptor
{
    uint32_t                    mem_freq;
    bool                        enabled;

    /* Frequencies are stored in potentially platform dependent multiples. In other words, *_freq needs to be multiplied by X to be interesting.
     * Soft limits are those which are used for the dynamic reclocking done by the driver (raise frequencies under heavy loads, and lower for
     * lighter loads). Hard limits are those imposed by the hardware. A distinction is made for overclocking, which is never enabled by default,
     * and is considered to be above the hard limit if it's possible at all. */
    uint8_t                     cur_freq;                       /* Current frequency (cached, may not == HW) */
    uint8_t                     min_freq_softlimit;             /* Minimum frequency permitted by the driver */
    uint8_t                     max_freq_softlimit;             /* Max frequency permitted by the driver */
    uint8_t                     max_freq;                       /* Maximum frequency, RP0 if not overclocking */
    uint8_t                     min_freq;                       /* AKA RPn. Minimum frequency */
    uint8_t                     idle_freq;                      /* Frequency to request when we are idle */
    uint8_t                     efficient_freq;                 /* AKA RPe. Pre-determined balanced frequency */
    uint8_t                     rp1_freq;                       /* "less than" RP0 power/freqency */
    uint8_t                     rp0_freq;                       /* Non-overclocked max frequency. */
    uint32_t                    cz_freq;

    enum {
        LOW_POWER, BETWEEN, HIGH_POWER
    }                           power;
} gpu_pm_descriptor_t;

enum intel_ddb_partitioning     { INTEL_DDB_PART_1_2, INTEL_DDB_PART_5_6, /* IVB+ */ };
struct skl_ddb_entry            { uint16_t start, end;  /* in number of blocks, 'end' is exclusive */ };

/* Shared context */
typedef struct intelgpu_drm_3d
{
    /* Debug */
    int                         debug;                          /* 1 - debug enabled */

    /* GPU info */
    uint32_t                    gpu_did;                        /* GPU Device ID */
    uint32_t                    gpu_id;                         /* GPU Core ID (see GPU_CORE_*) */
    uint32_t                    gpu_gen;                        /* GPU Core Generation */
    uint32_t                    gpu_pch;                        /* GPU PCH Device ID */
    uint32_t                    gpu_rev;                        /* GPU Revision */
    struct intel_device_info {
        uint8_t                 num_sprites[I915_MAX_PIPES];    /* GPU planes count (without PLANE_CURSOR, see intel_device_info_runtime_init()) */
        uint8_t                 has_snoop:1;
        uint8_t                 has_pooled_eu:1;

        /* Slice/subslice/EU info */
        struct sseu_dev_info {
            uint8_t             slice_mask;
            uint8_t             subslice_mask;
            uint8_t             eu_total;
            uint8_t             eu_per_subslice;
            uint8_t             min_eu_in_pool;
            /* For each slice, which subslice(s) has(have) 7 EUs (bitfield)? */
            uint8_t             subslice_7eu[3];
            uint8_t             has_slice_pg:1;
            uint8_t             has_subslice_pg:1;
            uint8_t             has_eu_pg:1;
        }                       sseu;

        struct color_luts {
            uint16_t            degamma_lut_size;
            uint16_t            gamma_lut_size;
        }                       color;
    }                           info;

    /* Registers */
    uint32_t                    registers_paddr;
    uint32_t                    registers_paddr_size;
#ifdef __QNXNTO__
    pthread_mutex_t             reg_mutex;                      /* Shared mutex for registers */
#endif

    /* VBIOS data */
    struct intel_vbt_data       vbt;

    /* IRQ processing & synchronizing */
    struct sync_descriptor      sync_descriptor;
    volatile bool               irqs_enabled;
    /** Cached value of IMR to avoid reads in updating the bitfield */
        union {
            uint32_t            irq_mask;
            uint32_t            de_irq_mask[I915_MAX_PIPES];
        };
        uint32_t                gt_irq_mask;
        uint32_t                pm_irq_mask;
        uint32_t                pm_rps_events;
        uint32_t                pipestat_irq_mask[I915_MAX_PIPES];

    /* GPU frequency processing */
    gpu_pm_descriptor_t         rps;

    /* Fence registers (tiled surfaces support) */
    fence_reg_t                 fence_regs[0x40];               /* > I915_MAX_NUM_FENCES */
    int                         num_fence_regs; /* 16 on pre-gen7 and vlv/chv, 32 otherwise */

    /* Rings */
    ring_shared_t               engine[0x10 /* > I915_NUM_RINGS */];
    uint8_t                     rings_available;                /* Available rings count */
    uint32_t                    ring_mask;
    disp_surface_t              rings_surface;                  /* Ring buffers surface (see rings.c) */
    struct i915_workarounds {                                   /* RCS engine workarounds */
        #define I915_MAX_WA_REGS        16
        struct i915_wa_reg {
            uint32_t            addr;
            uint32_t            value;
            uint32_t            mask;                           /* bitmask representing WA bits */
        }                       reg[I915_MAX_WA_REGS];
        uint32_t                count;
    }                           workarounds;
    disp_surface_t              render_state_surface;           /* RCS engine default state surface (see ring_render_state.c) */
    /* Rings requests */
    uint32_t                    last_seqno;
    uint32_t                    next_seqno;
    /* Current hw-context info */
    struct current_context_info {
        pid_t                   pid;
        uint32_t                ctx_id;
    }                           context;

    /* Features */
    uint32_t                    edram_cap;                      /* EDRAM memory size (eLLC): Cannot be determined by PCIID. You must always read a register. */
    int                         relative_constants_mode;
    int                         ring_hw_valid;                  /* Ring hardware initialization done */
    int                         valid;                          /* 3D context valid flag */
    bool                        has_pch_encoder;                /* Whether to set up the PCH/FDI. Note that we never allow sharing between pch encoders and cpu
                                                                 * encoders. */
    #define QUIRK_PIPEA_FORCE           (1 << 0)
    #define QUIRK_LVDS_SSC_DISABLE      (1 << 1)
    #define QUIRK_INVERT_BRIGHTNESS     (1 << 2)
    #define QUIRK_BACKLIGHT_PRESENT     (1 << 3)
    #define QUIRK_PIPEB_FORCE           (1 << 4)
    #define QUIRK_PIN_SWIZZLED_PAGES    (1 << 5)
    unsigned long               quirks;

    /* Memory management */
    struct i915_gem_mm          mm;

    /* Forcewake control */
#if INTELGPU_VERSION_IS_484
    struct intel_uncore         uncore;
#endif

    INTELGPU_VERSION_PRE_484_3D_CONTEXT;
} intelgpu_drm_3d_t;


/*************************************************/
/*                   PROTOTYPES                  */
/*************************************************/

/* qnx.c */
#if INTELGPU_VERSION_IS_484
void * intelgpu_get_drm_device();
void * intelgpu_get_drm_i915_private();

/* GGTT size */
uint64_t intelgpu_ggtt_address_space_size();
/* GGTT PTE size */
uint32_t intelgpu_ggtt_pte_size( void *drm_device );
/* Assign pages to GGTT */
void intelgpu_ggtt_bind_buffer( uint32_t domain, uint64_t addr, uint64_t offset, uint32_t size, uint32_t cache_level );
/* Drop pages from GGTT */
void intelgpu_ggtt_unbind_buffer( uint32_t domain, uint64_t addr, uint64_t offset, uint32_t size );

/* Vsync polling support */
uint32_t intelgpu_vblank_poll( void *drm_device, int pipe );

/* Cursor plane */
int intelgpu_display_update_cursor_plane( void *drm_device, int pipe, bool enable, bool move_event, uint32_t format /* 2 = 64x64x2bit / 64/128/256 = XxXx32bit */,
                                          uint32_t x, uint32_t y, uint32_t offset, uint32_t rotation /* DRM_ROTATE_0 / DRM_ROTATE_180 */ );

/* Sprite plane format converter */
uint32_t intelgpu_convert_layer_pixel_format( uint32_t format );
/* Sprite plane number */
int intelgpu_display_sprite_number( void *drm_device, int pipe );
/* Sprite plane configuration call */
int intelgpu_display_update_sprite_plane( void *drm_device, int pipe, int _sprite, bool enable, uint32_t format /* DRM_FORMAT_RGB565 / DRM_FORMAT_XBGR8888 / ... */,
                                          uint32_t sx1, uint32_t sy1, uint32_t sx2, uint32_t sy2, uint32_t dx1, uint32_t dy1, uint32_t dx2, uint32_t dy2,
                                          uint32_t offset, uint32_t stride, uint32_t rotation /* DRM_ROTATE_0 / DRM_ROTATE_180 */,
                                          uint32_t chroma_key /* I915_SET_COLORKEY_DESTINATION / I915_SET_COLORKEY_SOURCE / I915_SET_COLORKEY_NONE mask */,
                                          uint32_t chroma_key_min_value, uint32_t chroma_key_max_value, uint32_t chroma_key_channel_mask );

/* Primary/sprite plane align (in bytes) */
uint32_t intelgpu_display_plane_align( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
/* Primary/sprite plane extra memory size allocation (before surface memory, in bytes) */
uint32_t intelgpu_display_plane_allocation_pre_mem_size( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
/* Primary/sprite plane extra memory size allocation (after surface memory, in bytes) */
uint32_t intelgpu_display_plane_allocation_post_mem_size( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
/* Primary plane format converter */
uint32_t intelgpu_convert_surface_pixel_format( uint32_t format );
/* Atomic modeswitcher */
int intelgpu_display_atomic_modeswitcher( void *drm_device, int pipe, int port, intelgpu_port_id_t port_config, disp_crtc_settings_t *settings,
                                          int bpp /* bits per pixel */, int scaler_mode /* see DRM_MODE_SCALE_* */, bool disable_sequence, bool enable_vga );
/* Primary plane configuration call */
int intelgpu_display_update_primary_plane( void *drm_device, int pipe, bool enable, uint32_t format /* DRM_FORMAT_RGB565 / DRM_FORMAT_XBGR8888 / ... */,
                                           uint32_t width, uint32_t height, uint32_t offset, uint32_t stride, uint32_t rotation /* DRM_ROTATE_0 / DRM_ROTATE_180 */ );
#endif

/* gpu_id.c */
void intelgpu_set_supported_gpu_ids( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, uint32_t gpu_id_, uint16_t pch_id, uint16_t pci_did );
char * intelgpu_get_gpu_name( intelgpu_drm_3d_t *ctx );
char * intelgpu_get_pch_name( intelgpu_drm_3d_t *ctx );

/* vbios.c */
int intelgpu_vbios_init( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
void intelgpu_vbios_read( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
void intelgpu_vbios_fini();

/* gmbus.c */
void intelgpu_gmbus_init( void *gmbus_base /* unused */, intelgpu_gmbus_port_mapping_t pm, intelgpu_gmbus_port_name_t pn );
void intelgpu_gmbus_reset( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
int intelgpu_gmbus_read( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, intelgpu_port_id_t port, uint8_t i2c_slave, uint8_t *buf, uint16_t size, uint8_t silent );
int intelgpu_gmbus_write( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, intelgpu_port_id_t port, uint8_t i2c_slave, uint8_t *buf, uint16_t size, uint8_t silent );

/* gpio.c */
void intelgpu_gpio_init( intelgpu_gpio_port_mapping_t pm, intelgpu_gpio_port_name_t pn );
int intelgpu_gpio_read( disp_adapter_t *adapter, intelgpu_port_id_t port, uint8_t i2c_slave, uint8_t *buf, uint16_t size, uint8_t silent );
int intelgpu_gpio_write( disp_adapter_t *adapter, intelgpu_port_id_t port, uint8_t i2c_slave, uint8_t *buf, uint16_t size, uint8_t silent );

/* edid.c */
int intelgpu_get_edid( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, int mode, intelgpu_port_id_t port, uint8_t silent, int block_no, uint8_t *buf, uint16_t len );

/* irq.c */
void intelgpu_irq_setup_irq( intelgpu_drm_3d_t *ctx, int irq, int irq_id );
void intelgpu_irq_setup_vblank_event( intelgpu_drm_3d_t *ctx, struct sigevent *ev, uint32_t *vsync_counters, int vsync_counters_number );
int intelgpu_irq_vblank_enable( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );              /* Do not call before intelgpu_drm_init_3d_context_driver() */
void intelgpu_irq_vblank_setup( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, int pipe );    /* Must be called for each active pipe */
void intelgpu_irq_vblank_disable( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, int pipe );  /* Must be called for each active pipe */
void intelgpu_irg_fallback_to_polling_force( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
bool intelgpu_irg_fallback_to_polling( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
struct sigevent * intelgpu_irq_process( intelgpu_drm_3d_t *ctx );

/* drm.c */
void intelgpu_drm_set_debug_level( intelgpu_drm_3d_t *ctx, int level );
void intelgpu_drm_set_aperture_pointer( void * aperture );
void intelgpu_drm_init_adapter( intelgpu_drm_3d_t *ctx, disp_adapter_t *adapter );
void intelgpu_drm_init_regbase_driver( intelgpu_drm_3d_t *ctx, void *regbase );
void intelgpu_drm_fini_3d_context_driver( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
void intelgpu_drm_fini_client_driver( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, pid_t pid );
void intelgpu_drm_fini_client( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
int intelgpu_drm_init_3d_context_driver( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, alloc_surface_callback_t alloc_surface, blitter_forwarding_t *bcs );
int intelgpu_drm_init_3d_context_user( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
void intelgpu_drm_init_rings( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, blitter_forwarding_t *bcs, bool sysmet_call );
char * intelgpu_drm_decode_param_cmd( uint32_t param );
char * intelgpu_drm_decode_cmd( uint32_t cmd, void *data_in );
void intelgpu_drm_dump_cmd( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, uint32_t cmd, void *data_in );
int intelgpu_drm_ioctl_default( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, uint32_t cmd, void *data_in );
int intelgpu_drm_ioctl_gem_get_apperture( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size, uint32_t max, uint32_t avail );
int intelgpu_drm_ioctl_get_param_did( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size, uint32_t did );
int intelgpu_drm_ioctl_gem_create( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size, uint32_t page_size );
int intelgpu_drm_ioctl_gem_open( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size );
int intelgpu_drm_ioctl_gem_mmap( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size );
int intelgpu_drm_ioctl_gem_mmap_gtt( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size );
int intelgpu_drm_ioctl_gem_set_domain( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_in );
int intelgpu_drm_ioctl_gem_set_tiling( disp_adapter_t *adapter, void *data_out, int *size, intelgpu_drm_3d_t *ctx );
int intelgpu_drm_ioctl_gem_get_tiling( disp_adapter_t *adapter, void *data_out, int *size, intelgpu_drm_3d_t *ctx );
int intelgpu_drm_ioctl_gem_pwrite( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_in );
int intelgpu_drm_ioctl_gem_sw_finish( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_in );
int intelgpu_drm_ioctl_gem_wait( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size );
int intelgpu_drm_ioctl_gem_madvise( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size );
int intelgpu_drm_ioctl_gem_execbuffer2( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_in );
int intelgpu_drm_ioctl_gem_busy( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_out, int *size );
int intelgpu_drm_ioctl_gem_close( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_in );
int intelgpu_drm_ioctl_gem_context_create( disp_adapter_t *adapter, void *data_out, int *size, intelgpu_drm_3d_t *ctx );
int intelgpu_drm_ioctl_gem_context_destroy( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, void *data_in );
int intelgpu_drm_ioctl_reg_read( disp_adapter_t *adapter, void *data_in, intelgpu_drm_3d_t *ctx );

/* ring_uncore.c */
#if INTELGPU_VERSION_IS_484
void intel_uncore_forcewake_get( /* struct drm_i915_private * */ void * dev_priv, enum forcewake_domains fw_domains );
void intel_uncore_forcewake_put( /* struct drm_i915_private * */ void * dev_priv, enum forcewake_domains fw_domains );
#endif

/* gpu_pm.c */
int intelgpu_gpu_frequency_update( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx, uint8_t profile );

/* rings.c */
void intelgpu_drm_rings_forced_unlock( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
void intelgpu_drm_rings_flush_RCS( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );
void intelgpu_drm_rings_flush_wait_RCS( disp_adapter_t *adapter, intelgpu_drm_3d_t *ctx );


#endif  /* _INTELGPU_H_INCLUDED */
