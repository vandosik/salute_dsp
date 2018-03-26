#ifndef _GRAPHICS_EXTRA_DEVCTL_H_INCLUDED
#define _GRAPHICS_EXTRA_DEVCTL_H_INCLUDED


/* disp_modefuncs_t::devctl() commands */

#define DEVCTL_DDC                              (0x1000)            /* driver side "ddc" utility support */
#define DEVCTL_DISPLAY_INFO                     (0x2000)            /* driver side "display-info" utility support */


/* disp_modefuncs_t::devctl() data types */

typedef struct devctl_ddc
{
    #define DEVCTL_DDC_MODE_DISPLAY             (0x0)
    #define DEVCTL_DDC_MODE_BUS                 (0x1)
    uint8_t                     mode;                               /* DDC request mode */
    uint8_t                     bus;                                /* DDC bus mode */
} devctl_ddc_t;
typedef devctl_ddc_t                            devctl_ddc_request_t;

typedef struct devctl_display_mode
{
    #define DEVCTL_DISPLAY_INFO_DISPLAYS    (0x0)
    #define DEVCTL_DISPLAY_INFO_DISPLAY     (0x1)
    #define DEVCTL_DISPLAY_INFO_LAYER       (0x2)
    struct devctl_display_mode_request {
        uint8_t                 mode;                               /* Display config request mode */
        uint8_t                 display;                            /* Display id */
        uint8_t                 layer;                              /* Display layer id */
        uint8_t                 unused;
    }                           __attribute__((packed)) request;

    #define DEVCTL_DISPLAY_INFO_IFACE_NONE  (0x0)
    #define DEVCTL_DISPLAY_INFO_IFACE_VGA   (0x1)
    #define DEVCTL_DISPLAY_INFO_IFACE_DVI   (0x2)
    #define DEVCTL_DISPLAY_INFO_IFACE_HDMI  (0x3)
    #define DEVCTL_DISPLAY_INFO_IFACE_DP    (0x4)
    #define DEVCTL_DISPLAY_INFO_IFACE_LVDS  (0x5)
    #define DEVCTL_DISPLAY_INFO_IFACE_eDP   (0x6)
    #define DEVCTL_DISPLAY_INFO_IFACE_DSI   (0x7)
    #define DEVCTL_DISPLAY_INFO_STATUS_FAIL (0xa)
    #define DEVCTL_DISPLAY_INFO_STATUS_OK   (0xb)
    #define DEVCTL_DISPLAY_INFO_STATE_OFF   (0xa)
    #define DEVCTL_DISPLAY_INFO_STATE_ON    (0xb)
    #define DEVCTL_DISPLAY_INFO_CHROMA_NONE (0x0)
    #define DEVCTL_DISPLAY_INFO_CHROMA_SRC  (0x1)
    #define DEVCTL_DISPLAY_INFO_CHROMA_DST  (0x2)
    struct devctl_display_mode_reply {
        uint8_t                 status;                             /* Request status (DEVCTL_DISPLAY_INFO_STATUS_*) */
        uint8_t                 displays;                           /* Display count */
        uint8_t                 unused[2];

        struct {                                                    /* Display info */
            uint8_t             id;                                 /* id */
            uint8_t             state;                              /* state (DEVCTL_DISPLAY_INFO_STATE_*) */
            uint8_t             layers;                             /* layer count */
            uint8_t             unused;
            uint32_t            width;                              /* width */
            uint32_t            height;                             /* height */
            uint32_t            refresh;                            /* refresh rate */
            uint32_t            interface;                          /* display intercafe (DEVCTL_DISPLAY_INFO_IFACE_*) */
        }                       __attribute__((packed)) display;

        struct {                                                    /* Display layer info */
            uint8_t             id;                                 /* id */
            uint8_t             state;                              /* state (DEVCTL_DISPLAY_INFO_STATE_*) */
            uint8_t             unused[2];
            uint32_t            sid;                                /* GF surface ID */
            uint32_t            format;                             /* format (DISP_LAYER_FORMAT_*) */
            uint32_t            width;                              /* width */
            uint32_t            height;                             /* height */
            uint32_t            stride;                             /* stride */
            uint32_t            x;                                  /* x position */
            uint32_t            y;                                  /* y position */
            uint64_t            addr;                               /* phys address */
            struct {
                struct {
                    uint32_t    x1;                                 /* x1 source viewport coordinate */
                    uint32_t    y1;                                 /* y1 source viewport coordinate */
                    uint32_t    x2;                                 /* x2 source viewport coordinate */
                    uint32_t    y2;                                 /* y2 source viewport coordinate */
                }               __attribute__((packed)) source;
                struct {
                    uint32_t    x1;                                 /* x1 source viewport coordinate */
                    uint32_t    y1;                                 /* y1 source viewport coordinate */
                    uint32_t    x2;                                 /* x2 source viewport coordinate */
                    uint32_t    y2;                                 /* y2 source viewport coordinate */
                }               __attribute__((packed)) destination;
            }                   __attribute__((packed)) viewport;
            struct {
                uint32_t        mode;                               /* chroma key mode (#define DEVCTL_DISPLAY_INFO_CHROMA_*) */
                uint32_t        color;                              /* chroma key color */
            }                   __attribute__((packed)) chroma_key;
        }                       __attribute__((packed)) layer;
    }                           __attribute__((packed)) reply;
} devctl_display_mode_t;
typedef struct devctl_display_mode_request      devctl_display_mode_request_t;
typedef struct devctl_display_mode_reply        devctl_display_mode_reply_t;


#endif /* _GRAPHICS_EXTRA_DEVCTL_H_INCLUDED */
