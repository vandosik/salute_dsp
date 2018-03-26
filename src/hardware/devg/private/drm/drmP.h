/*
 * Internal Header for the Direct Rendering Manager
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * Copyright (c) 2009-2010, Code Aurora Forum.
 * All rights reserved.
 *
 * Author: Rickard E. (Rik) Faith <faith@valinux.com>
 * Author: Gareth Hughes <gareth@valinux.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _DRM_P_H_
#define _DRM_P_H_

#ifdef __QNX__
#include <asm-generic/bitsperlong.h>
#include <drm/drm_common.h>
#include <linux/device.h>
#endif
//#include <linux/agp_backend.h>
//#include <linux/cdev.h>
//#include <linux/dma-mapping.h>
//#include <linux/file.h>
//#include <linux/fs.h>
//#include <linux/highmem.h>
//#include <linux/idr.h>
//#include <linux/init.h>
//#include <linux/io.h>
//#include <linux/jiffies.h>
#include <linux/kernel.h>
//#include <linux/kref.h>
//#include <linux/miscdevice.h>
//#include <linux/mm.h>
//#include <linux/mutex.h>
//#include <linux/pci.h>
//#include <linux/platform_device.h>
//#include <linux/poll.h>
//#include <linux/ratelimit.h>
//#include <linux/sched.h>
//#include <linux/slab.h>
//#include <linux/types.h>
//#include <linux/vmalloc.h>
//#include <linux/workqueue.h>
//#include <linux/fence.h>

//#include <asm/mman.h>
//#include <asm/pgalloc.h>
//#include <asm/uaccess.h>

//#include <uapi/drm/drm.h>
#include <uapi/drm/drm_mode.h>

//#include <drm/drm_agpsupport.h>
#include <drm/drm_crtc.h>
#include <drm/drm_fourcc.h>
//#include <drm/drm_global.h>
//#include <drm/drm_hashtab.h>
//#include <drm/drm_mem_util.h>
#include <drm/drm_mm.h>
//#include <drm/drm_os_linux.h>
//#include <drm/drm_sarea.h>
//#include <drm/drm_vma_manager.h>

/*
 * The following categories are defined:
 *
 * CORE: Used in the generic drm code: drm_ioctl.c, drm_mm.c, drm_memory.c, ...
 *	 This is the category used by the DRM_DEBUG() macro.
 *
 * DRIVER: Used in the vendor specific part of the driver: i915, radeon, ...
 *	   This is the category used by the DRM_DEBUG_DRIVER() macro.
 *
 * KMS: used in the modesetting code.
 *	This is the category used by the DRM_DEBUG_KMS() macro.
 *
 * PRIME: used in the prime code.
 *	  This is the category used by the DRM_DEBUG_PRIME() macro.
 *
 * ATOMIC: used in the atomic code.
 *	  This is the category used by the DRM_DEBUG_ATOMIC() macro.
 *
 * VBL: used for verbose debug message in the vblank code
 *	  This is the category used by the DRM_DEBUG_VBL() macro.
 *
 * Enabling verbose debug messages is done through the drm.debug parameter,
 * each category being enabled by a bit.
 *
 * drm.debug=0x1 will enable CORE messages
 * drm.debug=0x2 will enable DRIVER messages
 * drm.debug=0x3 will enable CORE and DRIVER messages
 * ...
 * drm.debug=0x3f will enable all messages
 *
 * An interesting feature is that it's possible to enable verbose logging at
 * run-time by echoing the debug value in its sysfs node:
 *   # echo 0xf > /sys/module/drm/parameters/debug
 */
#define DRM_UT_CORE 		0x01
#define DRM_UT_DRIVER		0x02
#define DRM_UT_KMS		0x04
#define DRM_UT_PRIME		0x08
#define DRM_UT_ATOMIC		0x10
#define DRM_UT_VBL		0x20

/**
 * DRM driver structure. This structure represent the common code for
 * a family of cards. There will one drm_device for each card present
 * in this family
 */
struct drm_driver {
	//int (*load) (struct drm_device *, unsigned long flags);
	//int (*firstopen) (struct drm_device *);
	//int (*open) (struct drm_device *, struct drm_file *);
	//void (*preclose) (struct drm_device *, struct drm_file *file_priv);
	//void (*postclose) (struct drm_device *, struct drm_file *);
	//void (*lastclose) (struct drm_device *);
	//int (*unload) (struct drm_device *);
	//int (*dma_ioctl) (struct drm_device *dev, void *data, struct drm_file *file_priv);
	//int (*dma_quiescent) (struct drm_device *);
	//int (*context_dtor) (struct drm_device *dev, int context);
	//int (*set_busid)(struct drm_device *dev, struct drm_master *master);

	/**
	 * get_vblank_counter - get raw hardware vblank counter
	 * @dev: DRM device
	 * @pipe: counter to fetch
	 *
	 * Driver callback for fetching a raw hardware vblank counter for @crtc.
	 * If a device doesn't have a hardware counter, the driver can simply
	 * use drm_vblank_no_hw_counter() function. The DRM core will account for
	 * missed vblank events while interrupts where disabled based on system
	 * timestamps.
	 *
	 * Wraparound handling and loss of events due to modesetting is dealt
	 * with in the DRM core code.
	 *
	 * RETURNS
	 * Raw vblank counter value.
	 */
	u32 (*get_vblank_counter) (struct drm_device *dev, unsigned int pipe);

	/**
	 * enable_vblank - enable vblank interrupt events
	 * @dev: DRM device
	 * @pipe: which irq to enable
	 *
	 * Enable vblank interrupts for @crtc.  If the device doesn't have
	 * a hardware vblank counter, the driver should use the
	 * drm_vblank_no_hw_counter() function that keeps a virtual counter.
	 *
	 * RETURNS
	 * Zero on success, appropriate errno if the given @crtc's vblank
	 * interrupt cannot be enabled.
	 */
	//int (*enable_vblank) (struct drm_device *dev, unsigned int pipe);

	/**
	 * disable_vblank - disable vblank interrupt events
	 * @dev: DRM device
	 * @pipe: which irq to enable
	 *
	 * Disable vblank interrupts for @crtc.  If the device doesn't have
	 * a hardware vblank counter, the driver should use the
	 * drm_vblank_no_hw_counter() function that keeps a virtual counter.
	 */
	//void (*disable_vblank) (struct drm_device *dev, unsigned int pipe);

	/**
	 * Called by \c drm_device_is_agp.  Typically used to determine if a
	 * card is really attached to AGP or not.
	 *
	 * \param dev  DRM device handle
	 *
	 * \returns
	 * One of three values is returned depending on whether or not the
	 * card is absolutely \b not AGP (return of 0), absolutely \b is AGP
	 * (return of 1), or may or may not be AGP (return of 2).
	 */
	//int (*device_is_agp) (struct drm_device *dev);

	/**
	 * Called by vblank timestamping code.
	 *
	 * Return the current display scanout position from a crtc, and an
	 * optional accurate ktime_get timestamp of when position was measured.
	 *
	 * \param dev  DRM device.
	 * \param pipe Id of the crtc to query.
	 * \param flags Flags from the caller (DRM_CALLED_FROM_VBLIRQ or 0).
	 * \param *vpos Target location for current vertical scanout position.
	 * \param *hpos Target location for current horizontal scanout position.
	 * \param *stime Target location for timestamp taken immediately before
	 *               scanout position query. Can be NULL to skip timestamp.
	 * \param *etime Target location for timestamp taken immediately after
	 *               scanout position query. Can be NULL to skip timestamp.
	 * \param mode Current display timings.
	 *
	 * Returns vpos as a positive number while in active scanout area.
	 * Returns vpos as a negative number inside vblank, counting the number
	 * of scanlines to go until end of vblank, e.g., -1 means "one scanline
	 * until start of active scanout / end of vblank."
	 *
	 * \return Flags, or'ed together as follows:
	 *
	 * DRM_SCANOUTPOS_VALID = Query successful.
	 * DRM_SCANOUTPOS_INVBL = Inside vblank.
	 * DRM_SCANOUTPOS_ACCURATE = Returned position is accurate. A lack of
	 * this flag means that returned position may be offset by a constant
	 * but unknown small number of scanlines wrt. real scanout position.
	 *
	 */
	//int (*get_scanout_position) (struct drm_device *dev, unsigned int pipe,
				     //unsigned int flags, int *vpos, int *hpos,
				     //ktime_t *stime, ktime_t *etime,
				     //const struct drm_display_mode *mode);

	/**
	 * Called by \c drm_get_last_vbltimestamp. Should return a precise
	 * timestamp when the most recent VBLANK interval ended or will end.
	 *
	 * Specifically, the timestamp in @vblank_time should correspond as
	 * closely as possible to the time when the first video scanline of
	 * the video frame after the end of VBLANK will start scanning out,
	 * the time immediately after end of the VBLANK interval. If the
	 * @crtc is currently inside VBLANK, this will be a time in the future.
	 * If the @crtc is currently scanning out a frame, this will be the
	 * past start time of the current scanout. This is meant to adhere
	 * to the OpenML OML_sync_control extension specification.
	 *
	 * \param dev dev DRM device handle.
	 * \param pipe crtc for which timestamp should be returned.
	 * \param *max_error Maximum allowable timestamp error in nanoseconds.
	 *                   Implementation should strive to provide timestamp
	 *                   with an error of at most *max_error nanoseconds.
	 *                   Returns true upper bound on error for timestamp.
	 * \param *vblank_time Target location for returned vblank timestamp.
	 * \param flags 0 = Defaults, no special treatment needed.
	 * \param       DRM_CALLED_FROM_VBLIRQ = Function is called from vblank
	 *	        irq handler. Some drivers need to apply some workarounds
	 *              for gpu-specific vblank irq quirks if flag is set.
	 *
	 * \returns
	 * Zero if timestamping isn't supported in current display mode or a
	 * negative number on failure. A positive status code on success,
	 * which describes how the vblank_time timestamp was computed.
	 */
	//int (*get_vblank_timestamp) (struct drm_device *dev, unsigned int pipe,
				     //int *max_error,
				     //struct timeval *vblank_time,
				     //unsigned flags);

	/* these have to be filled in */

	//irqreturn_t(*irq_handler) (int irq, void *arg);
	//void (*irq_preinstall) (struct drm_device *dev);
	//int (*irq_postinstall) (struct drm_device *dev);
	//void (*irq_uninstall) (struct drm_device *dev);

	/* Master routines */
	//int (*master_create)(struct drm_device *dev, struct drm_master *master);
	//void (*master_destroy)(struct drm_device *dev, struct drm_master *master);
	/**
	 * master_set is called whenever the minor master is set.
	 * master_drop is called whenever the minor master is dropped.
	 */

	//int (*master_set)(struct drm_device *dev, struct drm_file *file_priv,
			  //bool from_open);
	//void (*master_drop)(struct drm_device *dev, struct drm_file *file_priv);

	//int (*debugfs_init)(struct drm_minor *minor);
	//void (*debugfs_cleanup)(struct drm_minor *minor);

	/**
	 * @gem_free_object: deconstructor for drm_gem_objects
	 *
	 * This is deprecated and should not be used by new drivers. Use
	 * @gem_free_object_unlocked instead.
	 */
	//void (*gem_free_object) (struct drm_gem_object *obj);

	/**
	 * @gem_free_object_unlocked: deconstructor for drm_gem_objects
	 *
	 * This is for drivers which are not encumbered with dev->struct_mutex
	 * legacy locking schemes. Use this hook instead of @gem_free_object.
	 */
	//void (*gem_free_object_unlocked) (struct drm_gem_object *obj);

	//int (*gem_open_object) (struct drm_gem_object *, struct drm_file *);
	//void (*gem_close_object) (struct drm_gem_object *, struct drm_file *);

	/**
	 * Hook for allocating the GEM object struct, for use by core
	 * helpers.
	 */
	//struct drm_gem_object *(*gem_create_object)(struct drm_device *dev,
						    //size_t size);

	/* prime: */
	/* export handle -> fd (see drm_gem_prime_handle_to_fd() helper) */
	//int (*prime_handle_to_fd)(struct drm_device *dev, struct drm_file *file_priv,
				//uint32_t handle, uint32_t flags, int *prime_fd);
	/* import fd -> handle (see drm_gem_prime_fd_to_handle() helper) */
	//int (*prime_fd_to_handle)(struct drm_device *dev, struct drm_file *file_priv,
				//int prime_fd, uint32_t *handle);
	/* export GEM -> dmabuf */
	//struct dma_buf * (*gem_prime_export)(struct drm_device *dev,
				//struct drm_gem_object *obj, int flags);
	/* import dmabuf -> GEM */
	//struct drm_gem_object * (*gem_prime_import)(struct drm_device *dev,
				//struct dma_buf *dma_buf);
	/* low-level interface used by drm_gem_prime_{import,export} */
	//int (*gem_prime_pin)(struct drm_gem_object *obj);
	//void (*gem_prime_unpin)(struct drm_gem_object *obj);
	//struct reservation_object * (*gem_prime_res_obj)(
				//struct drm_gem_object *obj);
	//struct sg_table *(*gem_prime_get_sg_table)(struct drm_gem_object *obj);
	//struct drm_gem_object *(*gem_prime_import_sg_table)(
				//struct drm_device *dev,
				//struct dma_buf_attachment *attach,
				//struct sg_table *sgt);
	//void *(*gem_prime_vmap)(struct drm_gem_object *obj);
	//void (*gem_prime_vunmap)(struct drm_gem_object *obj, void *vaddr);
	//int (*gem_prime_mmap)(struct drm_gem_object *obj,
				//struct vm_area_struct *vma);

	/* vga arb irq handler */
	//void (*vgaarb_irq)(struct drm_device *dev, bool state);

	/* dumb alloc support */
	//int (*dumb_create)(struct drm_file *file_priv,
			   //struct drm_device *dev,
			   //struct drm_mode_create_dumb *args);
	//int (*dumb_map_offset)(struct drm_file *file_priv,
			       //struct drm_device *dev, uint32_t handle,
			       //uint64_t *offset);
	//int (*dumb_destroy)(struct drm_file *file_priv,
			    //struct drm_device *dev,
			    //uint32_t handle);

	/* Driver private ops for this object */
	//const struct vm_operations_struct *gem_vm_ops;

	//int major;
	//int minor;
	//int patchlevel;
	//char *name;
	//char *desc;
	//char *date;

	//u32 driver_features;
	//int dev_priv_size;
	//const struct drm_ioctl_desc *ioctls;
	//int num_ioctls;
	//const struct file_operations *fops;

	/* List of devices hanging off this driver with stealth attach. */
	//struct list_head legacy_dev_list;
};

/**
 * DRM device structure. This structure represent a complete card that
 * may contain multiple heads.
 */
struct drm_device {
	//struct list_head legacy_dev_list;/**< list of devices per driver for stealth attach cleanup */
	//int if_version;			/**< Highest interface version set */

	/** \name Lifetime Management */
	/*@{ */
	//struct kref ref;		/**< Object ref-count */
	//struct device *dev;		/**< Device structure of bus-device */
	struct drm_driver *driver;	/**< DRM driver managing the device */
	void *dev_private;		/**< DRM driver private data */
	//struct drm_minor *control;		/**< Control node */
	//struct drm_minor *primary;		/**< Primary node */
	//struct drm_minor *render;		/**< Render node */

	/* currently active master for this device. Protected by master_mutex */
	//struct drm_master *master;

	//atomic_t unplugged;			/**< Flag whether dev is dead */
	//struct inode *anon_inode;		/**< inode for private address-space */
	//char *unique;				/**< unique name of the device */
	/*@} */

	/** \name Locks */
	/*@{ */
	//struct mutex struct_mutex;	/**< For others */
	//struct mutex master_mutex;      /**< For drm_minor::master and drm_file::is_master */
	/*@} */

	/** \name Usage Counters */
	/*@{ */
	//int open_count;			/**< Outstanding files open, protected by drm_global_mutex. */
	//spinlock_t buf_lock;		/**< For drm_device::buf_use and a few other things. */
	//int buf_use;			/**< Buffers in use -- cannot alloc */
	//atomic_t buf_alloc;		/**< Buffer allocation in progress */
	/*@} */

	//struct mutex filelist_mutex;
	//struct list_head filelist;

	/** \name Memory management */
	/*@{ */
	//struct list_head maplist;	/**< Linked list of regions */
	//struct drm_open_hash map_hash;	/**< User token hash table for maps */

	/** \name Context handle management */
	/*@{ */
	//struct list_head ctxlist;	/**< Linked list of context handles */
	//struct mutex ctxlist_mutex;	/**< For ctxlist */

	//struct idr ctx_idr;

	//struct list_head vmalist;	/**< List of vmas (for debugging) */

	/*@} */

	/** \name DMA support */
	/*@{ */
	//struct drm_device_dma *dma;		/**< Optional pointer for DMA support */
	/*@} */

	/** \name Context support */
	/*@{ */

	//__volatile__ long context_flag;	/**< Context swapping flag */
	//int last_context;		/**< Last current context */
	/*@} */

	/** \name VBLANK IRQ support */
	/*@{ */
	//bool irq_enabled;
	//int irq;

	/*
	 * If true, vblank interrupt will be disabled immediately when the
	 * refcount drops to zero, as opposed to via the vblank disable
	 * timer.
	 * This can be set to true it the hardware has a working vblank
	 * counter and the driver uses drm_vblank_on() and drm_vblank_off()
	 * appropriately.
	 */
	//bool vblank_disable_immediate;

	/* array of size num_crtcs */
	//struct drm_vblank_crtc *vblank;

	//spinlock_t vblank_time_lock;    /**< Protects vblank count and time updates during vblank enable/disable */
	//spinlock_t vbl_lock;

	u32 max_vblank_count;           /**< size of vblank counter register */

	/**
	 * List of events
	 */
	//struct list_head vblank_event_list;
	//spinlock_t event_lock;

	/*@} */

	//struct drm_agp_head *agp;	/**< AGP data */

	//struct pci_dev *pdev;		/**< PCI device structure */
#ifdef __alpha__
	//struct pci_controller *hose;
#endif

	//struct platform_device *platformdev; /**< Platform device struture */
	//struct virtio_device *virtdev;

	//struct drm_sg_mem *sg;	/**< Scatter gather memory */
	//unsigned int num_crtcs;                  /**< Number of CRTCs on this device */

	//struct {
		//int context;
		//struct drm_hw_lock *lock;
	//} sigdata;

	//struct drm_local_map *agp_buffer_map;
	//unsigned int agp_buffer_token;

	struct drm_mode_config mode_config;	/**< Current mode config */

	/** \name GEM information */
	/*@{ */
	//struct mutex object_name_lock;
	//struct idr object_name_idr;
	//struct drm_vma_offset_manager *vma_offset_manager;
	/*@} */
	//int switch_power_state;
};

/* Cache management (drm_cache.c) */
void drm_clflush_virt_range(void *addr, unsigned long length);


int drm_dev_init(struct drm_device *dev,
		 struct drm_driver *driver,
		 struct device *parent);

/* helper for handling conditionals in various for_each macros */
#define for_each_if(condition) if (!(condition)) {} else

#endif  /* _DRM_P_H_ */
