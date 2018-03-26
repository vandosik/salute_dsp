#ifndef __RADEON_DRM_H__
#define __RADEON_DRM_H__

#include "drm.h"

#define DRM_RADEON_GEM_INFO             0x1c
#define DRM_RADEON_GEM_CREATE           0x1d
#define DRM_RADEON_GEM_MMAP             0x1e
#define DRM_RADEON_GEM_PREAD            0x21
#define DRM_RADEON_GEM_PWRITE           0x22
#define DRM_RADEON_GEM_SET_DOMAIN       0x23
#define DRM_RADEON_GEM_WAIT_IDLE        0x24
#define DRM_RADEON_CS                   0x26
#define DRM_RADEON_INFO                 0x27
#define DRM_RADEON_GEM_SET_TILING       0x28
#define DRM_RADEON_GEM_GET_TILING       0x29
#define DRM_RADEON_GEM_BUSY             0x2a
#define DRM_RADEON_GEM_VA               0x2b
#define DRM_RADEON_GEM_OP               0x2c
//#define DRM_RADEON_GEM_USERPTR          0x2d

#define DRM_IOCTL_RADEON_GEM_INFO       DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_INFO, struct drm_radeon_gem_info)
#define DRM_IOCTL_RADEON_GEM_CREATE     DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_CREATE, struct drm_radeon_gem_create)
#define DRM_IOCTL_RADEON_GEM_MMAP       DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_MMAP, struct drm_radeon_gem_mmap)
#define DRM_IOCTL_RADEON_GEM_PREAD      DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_PREAD, struct drm_radeon_gem_pread)
#define DRM_IOCTL_RADEON_GEM_PWRITE     DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_PWRITE, struct drm_radeon_gem_pwrite)
#define DRM_IOCTL_RADEON_GEM_SET_DOMAIN DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_SET_DOMAIN, struct drm_radeon_gem_set_domain)
#define DRM_IOCTL_RADEON_GEM_WAIT_IDLE  DRM_IOW(DRM_COMMAND_BASE + DRM_RADEON_GEM_WAIT_IDLE, struct drm_radeon_gem_wait_idle)
#define DRM_IOCTL_RADEON_CS             DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_CS, struct drm_radeon_cs)
#define DRM_IOCTL_RADEON_INFO           DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_INFO, struct drm_radeon_info)
#define DRM_IOCTL_RADEON_GEM_SET_TILING DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_SET_TILING, struct drm_radeon_gem_set_tiling)
#define DRM_IOCTL_RADEON_GEM_GET_TILING DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_GET_TILING, struct drm_radeon_gem_get_tiling)
#define DRM_IOCTL_RADEON_GEM_BUSY       DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_BUSY, struct drm_radeon_gem_busy)
#define DRM_IOCTL_RADEON_GEM_VA         DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_VA, struct drm_radeon_gem_va)
#define DRM_IOCTL_RADEON_GEM_OP         DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_OP, struct drm_radeon_gem_op)
//#define DRM_IOCTL_RADEON_GEM_USERPTR    DRM_IOWR(DRM_COMMAND_BASE + DRM_RADEON_GEM_USERPTR, struct drm_radeon_gem_userptr)

/*
 * Kernel modesetting world below.
 */
#define RADEON_GEM_DOMAIN_CPU           0x1
#define RADEON_GEM_DOMAIN_GTT           0x2
#define RADEON_GEM_DOMAIN_VRAM          0x4

struct drm_radeon_gem_info {
	uint64_t    gart_size;
	uint64_t    vram_size;
	uint64_t    vram_visible;
};

#define RADEON_GEM_NO_BACKING_STORE 1
#define RADEON_GEM_GTT_UC               (1 << 1)
#define RADEON_GEM_GTT_WC               (1 << 2)
/* BO is expected to be accessed by the CPU */
#define RADEON_GEM_CPU_ACCESS           (1 << 3)
/* CPU access is not expected to work for this BO */
#define RADEON_GEM_NO_CPU_ACCESS        (1 << 4)

struct drm_radeon_gem_create {
	uint64_t size;
	uint64_t alignment;
	uint32_t handle;
	uint32_t initial_domain;
	uint32_t flags;
};

struct drm_radeon_gem_pread {
	/** Handle for the object being read. */
	uint32_t handle;
	uint32_t pad;
	/** Offset into the object to read from */
	uint64_t offset;
	/** Length of data to read */
	uint64_t size;
	/** Pointer to write the data into. */
	/* void *, but pointers are not 32/64 compatible */
	uint64_t data_ptr;
};

struct drm_radeon_gem_pwrite {
	/** Handle for the object being written to. */
	uint32_t handle;
	uint32_t pad;
	/** Offset into the object to write to */
	uint64_t offset;
	/** Length of data to write */
	uint64_t size;
	/** Pointer to read the data from. */
	/* void *, but pointers are not 32/64 compatible */
	uint64_t data_ptr;
};

#define RADEON_TILING_MACRO				0x1
#define RADEON_TILING_MICRO				0x2
#define RADEON_TILING_SWAP_16BIT			0x4
#define RADEON_TILING_R600_NO_SCANOUT       RADEON_TILING_SWAP_16BIT
#define RADEON_TILING_SWAP_32BIT			0x8
/* this object requires a surface when mapped - i.e. front buffer */
#define RADEON_TILING_SURFACE				0x10
#define RADEON_TILING_MICRO_SQUARE			0x20
#define RADEON_TILING_EG_BANKW_SHIFT			8
#define RADEON_TILING_EG_BANKW_MASK			0xf
#define RADEON_TILING_EG_BANKH_SHIFT			12
#define RADEON_TILING_EG_BANKH_MASK			0xf
#define RADEON_TILING_EG_MACRO_TILE_ASPECT_SHIFT	16
#define RADEON_TILING_EG_MACRO_TILE_ASPECT_MASK		0xf
#define RADEON_TILING_EG_TILE_SPLIT_SHIFT		24
#define RADEON_TILING_EG_TILE_SPLIT_MASK		0xf
#define RADEON_TILING_EG_STENCIL_TILE_SPLIT_SHIFT	28
#define RADEON_TILING_EG_STENCIL_TILE_SPLIT_MASK	0xf

struct drm_radeon_gem_set_tiling {
	uint32_t 	handle;
	uint32_t	tiling_flags;
	uint32_t	pitch;
};

struct drm_radeon_gem_get_tiling {
	uint32_t 	handle;
	uint32_t	tiling_flags;
	uint32_t	pitch;
};

struct drm_radeon_gem_mmap {
	uint32_t 	handle;
	uint32_t	pad;
	uint64_t	offset;
	uint64_t	size;
	uint64_t	addr_ptr;
};

struct drm_radeon_gem_set_domain {
	uint32_t 	handle;
	uint32_t	read_domains;
	uint32_t	write_domain;
};

struct drm_radeon_gem_wait_idle {
	uint32_t 	handle;
	uint32_t	pad;
};

struct drm_radeon_gem_busy {
	uint32_t 	handle;
	uint32_t     domain;
};

/* Sets or returns a value associated with a buffer. */
struct drm_radeon_gem_op {
	uint32_t 		handle; /* buffer */
	uint32_t        op;     /* RADEON_GEM_OP_* */
	uint64_t        value;  /* input or return value */
};

#define RADEON_GEM_OP_GET_INITIAL_DOMAIN        0
#define RADEON_GEM_OP_SET_INITIAL_DOMAIN        1

#define RADEON_VA_MAP                   1
#define RADEON_VA_UNMAP                 2

#define RADEON_VA_RESULT_OK             0
#define RADEON_VA_RESULT_ERROR          1
#define RADEON_VA_RESULT_VA_EXIST       2

#define RADEON_VM_PAGE_VALID            (1 << 0)
#define RADEON_VM_PAGE_READABLE         (1 << 1)
#define RADEON_VM_PAGE_WRITEABLE        (1 << 2)
#define RADEON_VM_PAGE_SYSTEM           (1 << 3)
#define RADEON_VM_PAGE_SNOOPED          (1 << 4)


struct drm_radeon_gem_va {
	uint32_t    handle;
	uint32_t    operation;
	uint32_t    vm_id;
	uint32_t    flags;
	uint64_t    offset;
};


#define RADEON_CHUNK_ID_RELOCS	0x01
#define RADEON_CHUNK_ID_IB	0x02
#define RADEON_CHUNK_ID_FLAGS	0x03
#define RADEON_CHUNK_ID_CONST_IB	0x04

/* The first dword of RADEON_CHUNK_ID_FLAGS is a uint32 of these flags: */
#define RADEON_CS_KEEP_TILING_FLAGS 0x01
#define RADEON_CS_USE_VM            0x02
#define RADEON_CS_END_OF_FRAME      0x04 /* a hint from userspace which CS is the last one */
/* The second dword of RADEON_CHUNK_ID_FLAGS is a uint32 that sets the ring type */
#define RADEON_CS_RING_GFX          0
#define RADEON_CS_RING_COMPUTE      1
#define RADEON_CS_RING_DMA          2
#define RADEON_CS_RING_UVD          3
#define RADEON_CS_RING_VCE			4
/* The third dword of RADEON_CHUNK_ID_FLAGS is a sint32 that sets the priority */
/* 0 = normal, + = higher priority, - = lower priority */

struct drm_radeon_cs_chunk {
	uint32_t		chunk_id;
	uint32_t		length_dw;
	uint64_t		chunk_data;
};

/* drm_radeon_cs_reloc.flags */
#define RADEON_RELOC_PRIO_MASK          (0xf << 0)

struct drm_radeon_cs_reloc {
	uint32_t		handle;
	uint32_t		read_domains;
	uint32_t		write_domain;
	uint32_t		flags;
};

struct drm_radeon_cs {
	uint32_t		num_chunks;
	uint32_t		cs_id;
	/* this points to uint64_t * which point to cs chunks */
	uint64_t		chunks;
	/* updates to the limits after this CS ioctl */
	uint64_t		gart_limit;
	uint64_t		vram_limit;
};

#define RADEON_INFO_DEVICE_ID		0x00
#define RADEON_INFO_NUM_GB_PIPES	0x01
#define RADEON_INFO_NUM_Z_PIPES 	0x02
#define RADEON_INFO_ACCEL_WORKING	0x03
#define RADEON_INFO_CRTC_FROM_ID	0x04
#define RADEON_INFO_ACCEL_WORKING2	0x05
#define RADEON_INFO_TILING_CONFIG	0x06
#define RADEON_INFO_WANT_HYPERZ		0x07
#define RADEON_INFO_WANT_CMASK		0x08 /* get access to CMASK on r300 */
#define RADEON_INFO_CLOCK_CRYSTAL_FREQ	0x09 /* clock crystal frequency */
#define RADEON_INFO_NUM_BACKENDS	0x0a /* DB/backends for r600+ - need for OQ */
#define RADEON_INFO_NUM_TILE_PIPES	0x0b /* tile pipes for r600+ */
#define RADEON_INFO_FUSION_GART_WORKING	0x0c /* fusion writes to GTT were broken before this */
#define RADEON_INFO_BACKEND_MAP		0x0d /* pipe to backend map, needed by mesa */
/* virtual address start, va < start are reserved by the kernel */
#define RADEON_INFO_VA_START		0x0e
/* maximum size of ib using the virtual memory cs */
#define RADEON_INFO_IB_VM_MAX_SIZE	0x0f
/* max pipes - needed for compute shaders */
#define RADEON_INFO_MAX_PIPES		0x10
/* timestamp for GL_ARB_timer_query (OpenGL), returns the current GPU clock */
#define RADEON_INFO_TIMESTAMP		0x11
/* max shader engines (SE) - needed for geometry shaders, etc. */
#define RADEON_INFO_MAX_SE		0x12
/* max SH per SE */
#define RADEON_INFO_MAX_SH_PER_SE	0x13
/* fast fb access is enabled */
#define RADEON_INFO_FASTFB_WORKING	0x14
/* query if a RADEON_CS_RING_* submission is supported */
#define RADEON_INFO_RING_WORKING	0x15
/* SI tile mode array */
#define RADEON_INFO_SI_TILE_MODE_ARRAY	0x16
/* query if CP DMA is supported on the compute ring */
#define RADEON_INFO_SI_CP_DMA_COMPUTE   0x17
/* CIK macrotile mode array */
#define RADEON_INFO_CIK_MACROTILE_MODE_ARRAY    0x18
/* query the number of render backends */
#define RADEON_INFO_SI_BACKEND_ENABLED_MASK     0x19
/* max engine clock - needed for OpenCL */
#define RADEON_INFO_MAX_SCLK            0x1a
/* version of VCE firmware */
#define RADEON_INFO_VCE_FW_VERSION      0x1b
/* version of VCE feedback */
#define RADEON_INFO_VCE_FB_VERSION      0x1c
#define RADEON_INFO_NUM_BYTES_MOVED     0x1d
#define RADEON_INFO_VRAM_USAGE          0x1e
#define RADEON_INFO_GTT_USAGE           0x1f
#define RADEON_INFO_ACTIVE_CU_COUNT     0x20
#define RADEON_INFO_CURRENT_GPU_TEMP	0x21
#define RADEON_INFO_CURRENT_GPU_SCLK	0x22
#define RADEON_INFO_CURRENT_GPU_MCLK	0x23
#define RADEON_INFO_READ_REG		0x24
#define RADEON_INFO_VA_UNMAP_WORKING	0x25

struct drm_radeon_info {
	uint32_t		request;
	uint32_t		pad;
	uint32_t		*value;
};

struct drm_radeon_sinfo {
	uint32_t		request;
	uint32_t		pad;
	union
	{
		uint64_t		value;
		uint32_t		value64;
	};
};

/* Those correspond to the tile index to use, this is to explicitly state
 * the API that is implicitly defined by the tile mode array.
 */
#define SI_TILE_MODE_COLOR_LINEAR_ALIGNED	8
#define SI_TILE_MODE_COLOR_1D			13
#define SI_TILE_MODE_COLOR_1D_SCANOUT		9
#define SI_TILE_MODE_COLOR_2D_8BPP		14
#define SI_TILE_MODE_COLOR_2D_16BPP		15
#define SI_TILE_MODE_COLOR_2D_32BPP		16
#define SI_TILE_MODE_COLOR_2D_64BPP		17
#define SI_TILE_MODE_COLOR_2D_SCANOUT_16BPP	11
#define SI_TILE_MODE_COLOR_2D_SCANOUT_32BPP	12
#define SI_TILE_MODE_DEPTH_STENCIL_1D		4
#define SI_TILE_MODE_DEPTH_STENCIL_2D		0
#define SI_TILE_MODE_DEPTH_STENCIL_2D_2AA	3
#define SI_TILE_MODE_DEPTH_STENCIL_2D_4AA	3
#define SI_TILE_MODE_DEPTH_STENCIL_2D_8AA	2

#define CIK_TILE_MODE_COLOR_2D			14
#define CIK_TILE_MODE_COLOR_2D_SCANOUT		10
#define CIK_TILE_MODE_DEPTH_STENCIL_2D_TILESPLIT_64       0
#define CIK_TILE_MODE_DEPTH_STENCIL_2D_TILESPLIT_128      1
#define CIK_TILE_MODE_DEPTH_STENCIL_2D_TILESPLIT_256      2
#define CIK_TILE_MODE_DEPTH_STENCIL_2D_TILESPLIT_512      3
#define CIK_TILE_MODE_DEPTH_STENCIL_2D_TILESPLIT_ROW_SIZE 4
#define CIK_TILE_MODE_DEPTH_STENCIL_1D		5

#endif
