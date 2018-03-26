#ifndef _DRM_H_
#define _DRM_H_

#ifdef __QNX__

#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#if defined(__KERNEL__) || defined(__linux__)
#include <linux/types.h>
#else /* One of the BSDs */
typedef int8_t      __s8;
typedef uint8_t     __u8;
typedef int16_t     __s16;
typedef uint16_t    __u16;
typedef int32_t     __s32;
typedef uint32_t    __u32;
typedef int64_t     __s64;
typedef uint64_t    __u64;
#endif
typedef unsigned long drm_handle_t;

#ifndef likely
#define likely(x)      __builtin_expect( !!(x), 1 )
#endif
#ifndef unlikely
#define unlikely(x)    __builtin_expect( !!(x), 0 )
#endif

#endif

#ifndef __NOT_HAVE_DRM_H    /* mesa DRI driver use self-defined types */
#define DRM_CONTEXT_T_U32_DEFINED
typedef unsigned int drm_context_t;
#define DRM_DRAWABLE_T_U32_DEFINED
typedef unsigned int drm_drawable_t;
#endif
typedef unsigned int drm_magic_t;

/**
 * Cliprect.
 *
 * \warning: If you change this structure, make sure you change
 * XF86DRIClipRectRec in the server as well
 *
 * \note KW: Actually it's illegal to change either for
 * backwards-compatibility reasons.
 */
struct drm_clip_rect {
	unsigned short x1;
	unsigned short y1;
	unsigned short x2;
	unsigned short y2;
};

/**
 * Texture region,
 */
struct drm_tex_region {
	unsigned char next;
	unsigned char prev;
	unsigned char in_use;
	unsigned char padding;
	unsigned int age;
};

enum drm_vblank_seq_type {
	_DRM_VBLANK_ABSOLUTE = 0x0,	/**< Wait for specific vblank sequence number */
	_DRM_VBLANK_RELATIVE = 0x1,	/**< Wait for given number of vblanks */
	_DRM_VBLANK_EVENT = 0x4000000,   /**< Send event instead of blocking */
	_DRM_VBLANK_FLIP = 0x8000000,   /**< Scheduled buffer swap should flip */
	_DRM_VBLANK_NEXTONMISS = 0x10000000,	/**< If missed, wait for next vblank */
	_DRM_VBLANK_SECONDARY = 0x20000000,	/**< Secondary display controller */
	_DRM_VBLANK_SIGNAL = 0x40000000	/**< Send signal instead of blocking, unsupported */
};

/**
 * DRM_IOCTL_VERSION ioctl argument type.
 *
 * \sa drmGetVersion().
 */
struct drm_version
{
	int version_major;        /**< Major version */
	int version_minor;        /**< Minor version */
	int version_patchlevel;   /**< Patch level */
};


/** DRM_IOCTL_GEM_CLOSE ioctl argument type */
struct drm_gem_close {
	/** Handle of the object to be closed. */
	__u32 handle;
	__u32 pad;
};

/** DRM_IOCTL_GEM_FLINK ioctl argument type */
struct drm_gem_flink {
	/** Handle for the object being named */
	__u32 handle;

	/** Returned global name */
#ifdef __QNXNTO__
	void *name;
#else
	__u32 name;
#endif
};

/** DRM_IOCTL_GEM_OPEN ioctl argument type */
struct drm_gem_open {
	/** Name of object being opened */
#ifdef __QNXNTO__
	void *name;
#else
	__u32 name;
#endif

	/** Returned handle for the object */
	__u32 handle;

	/** Returned size of the object */
	__u64 size;
};

#define DRM_CLOEXEC O_CLOEXEC
struct drm_prime_handle {
	__u32 handle;

	/** Flags.. only applicable for handle->fd */
	__u32 flags;

	/** Returned dmabuf file descriptor */
	__s32 fd;
};

#define DRM_IOCTL_BASE                  'd'
#define DRM_IO(nr)                      _IO(DRM_IOCTL_BASE,nr)
#define DRM_IOR(nr,type)                _IOR(DRM_IOCTL_BASE,nr,type)
#define DRM_IOW(nr,type)                _IOW(DRM_IOCTL_BASE,nr,type)
#define DRM_IOWR(nr,type)               _IOWR(DRM_IOCTL_BASE,nr,type)

#define DRM_IOCTL_VERSION               DRM_IOWR(0x00, struct drm_version)
#define DRM_IOCTL_GEM_CLOSE             DRM_IOW (0x09, struct drm_gem_close)
#define DRM_IOCTL_GEM_FLINK             DRM_IOWR(0x0a, struct drm_gem_flink)
#define DRM_IOCTL_GEM_OPEN              DRM_IOWR(0x0b, struct drm_gem_open)

#define DRM_IOCTL_PRIME_HANDLE_TO_FD    DRM_IOWR(0x2d, struct drm_prime_handle)
#define DRM_IOCTL_PRIME_FD_TO_HANDLE    DRM_IOWR(0x2e, struct drm_prime_handle)

/**
 * Device specific ioctls should only be in their respective headers
 * The device specific ioctl range is from 0x40 to 0x99.
 * Generic IOCTLS restart at 0xA0.
 *
 * \sa drmCommandNone(), drmCommandRead(), drmCommandWrite(), and
 * drmCommandReadWrite().
 */
#define DRM_COMMAND_BASE                0x40
#define DRM_COMMAND_END                 0xA0


#ifdef __QNXNTO__

#include <gf/gf.h>
#include <gf/gf_devctl.h>

/* QNX io-display/driver interface */
static inline int drmCommand(gf_dev_t dev, unsigned long drmCommandIndex, void *data, unsigned long size )
{
	return gf_devctl( dev, drmCommandIndex, data, size, NULL, 0 );
}

#define drmIoctl( d, c, m ) \
	drmCommand( (d), (c), (m), sizeof(*(m)) )

#define drmCommandWriteRead( d, c, b, s ) \
	drmCommand( (d), _IOC( IOC_INOUT, DRM_IOCTL_BASE, (DRM_COMMAND_BASE + c), s ), (b), s )
#define drmCommandWrite( d, c, b, s ) \
	drmCommand( (d), _IOC( IOC_IN,    DRM_IOCTL_BASE, (DRM_COMMAND_BASE + c), s ), (b), s )
#define drmCommandRead( d, c, b, s ) \
	drmCommand( (d), _IOC( IOC_OUT,   DRM_IOCTL_BASE, (DRM_COMMAND_BASE + c), s ), (b), s )
#define drmCommandNone( d, c ) \
	drmCommand( (d), _IOC( IOC_VOID,  DRM_IOCTL_BASE, (DRM_COMMAND_BASE + c), 0 ), (NULL), 0 )

#endif


/* typedef area */
#ifndef __NOT_HAVE_DRM_H    /* mesa DRI driver use self-defined types */
#define DRM_CLIP_RECT_T_U32_DEFINED
typedef struct drm_clip_rect drm_clip_rect_t;
#endif


#define DRM_DEV_NAME                    "io-display"
typedef struct _drmDevice {
#ifndef __QNX__
    char **nodes; /* DRM_NODE_MAX sized array */
    int available_nodes; /* DRM_NODE_* bitmask */
    int bustype;
    union {
        drmPciBusInfoPtr pci;
    } businfo;
    union {
        drmPciDeviceInfoPtr pci;
    } deviceinfo;
#else
#ifdef __WATCOMC__
    int                         unused;     /* QNX4 compiller silence fix */
#endif
#endif
} drmDevice, *drmDevicePtr;


#endif
