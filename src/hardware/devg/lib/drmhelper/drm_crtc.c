/*
 * Copyright (c) 2006-2008 Intel Corporation
 * Copyright (c) 2007 Dave Airlie <airlied@linux.ie>
 * Copyright (c) 2008 Red Hat Inc.
 *
 * DRM core CRTC related functions
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Authors:
 *      Keith Packard
 *	Eric Anholt <eric@anholt.net>
 *      Dave Airlie <airlied@linux.ie>
 *      Jesse Barnes <jesse.barnes@intel.com>
 */

#ifdef __QNX__
#include <drm/drmP.h>   /* Must be included before string.h */
#include <string.h>
#endif
//#include <linux/ctype.h>
#include <linux/list.h>
#include <linux/slab.h>
//#include <linux/export.h>
#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_edid.h>
#include <drm/drm_fourcc.h>
//#include <drm/drm_modeset_lock.h>
#include <drm/drm_atomic.h>
//#include <drm/drm_auth.h>

//#include "drm_crtc_internal.h"
//#include "drm_internal.h"

/**
 * drm_crtc_init_with_planes - Initialise a new CRTC object with
 *    specified primary and cursor planes.
 * @dev: DRM device
 * @crtc: CRTC object to init
 * @primary: Primary plane for CRTC
 * @cursor: Cursor plane for CRTC
 * @funcs: callbacks for the new CRTC
 * @name: printf style format string for the CRTC name, or NULL for default name
 *
 * Inits a new object created as base part of a driver crtc object.
 *
 * Returns:
 * Zero on success, error code on failure.
 */
int drm_crtc_init_with_planes(struct drm_device *dev, struct drm_crtc *crtc,
			      struct drm_plane *primary,
			      struct drm_plane *cursor,
			      const struct drm_crtc_funcs *funcs,
			      const char *name, ...)
{
	struct drm_mode_config *config = &dev->mode_config;
	int ret;

#ifndef __QNX__
	WARN_ON(primary && primary->type != DRM_PLANE_TYPE_PRIMARY);
	WARN_ON(cursor && cursor->type != DRM_PLANE_TYPE_CURSOR);
#endif

	crtc->dev = dev;
	crtc->funcs = funcs;

#ifndef __QNX__
	INIT_LIST_HEAD(&crtc->commit_list);
	spin_lock_init(&crtc->commit_lock);

	drm_modeset_lock_init(&crtc->mutex);
	ret = drm_mode_object_get(dev, &crtc->base, DRM_MODE_OBJECT_CRTC);
	if (ret)
		return ret;

	if (name) {
		va_list ap;

		va_start(ap, name);
		crtc->name = kvasprintf(GFP_KERNEL, name, ap);
		va_end(ap);
	} else {
		crtc->name = kasprintf(GFP_KERNEL, "crtc-%d",
				       drm_num_crtcs(dev));
	}
	if (!crtc->name) {
		drm_mode_object_unregister(dev, &crtc->base);
		return -ENOMEM;
	}

	crtc->base.properties = &crtc->properties;
#endif

	list_add_tail(&crtc->head, &config->crtc_list);
	crtc->index = config->num_crtc++;

	crtc->primary = primary;
	crtc->cursor = cursor;
	if (primary)
		primary->possible_crtcs = 1 << drm_crtc_index(crtc);
	if (cursor)
		cursor->possible_crtcs = 1 << drm_crtc_index(crtc);

#ifndef __QNX__
	if (drm_core_check_feature(dev, DRIVER_ATOMIC)) {
		drm_object_attach_property(&crtc->base, config->prop_active, 0);
		drm_object_attach_property(&crtc->base, config->prop_mode_id, 0);
	}
#endif

	return 0;
}

/**
 * drm_crtc_cleanup - Clean up the core crtc usage
 * @crtc: CRTC to cleanup
 *
 * This function cleans up @crtc and removes it from the DRM mode setting
 * core. Note that the function does *not* free the crtc structure itself,
 * this is the responsibility of the caller.
 */
void drm_crtc_cleanup(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;

	/* Note that the crtc_list is considered to be static; should we
	 * remove the drm_crtc at runtime we would have to decrement all
	 * the indices on the drm_crtc after us in the crtc_list.
	 */

#ifndef __QNX__
	kfree(crtc->gamma_store);
	crtc->gamma_store = NULL;

	drm_modeset_lock_fini(&crtc->mutex);

	drm_mode_object_unregister(dev, &crtc->base);
#endif
	list_del(&crtc->head);
	dev->mode_config.num_crtc--;

#ifndef __QNX__
	WARN_ON(crtc->state && !crtc->funcs->atomic_destroy_state);
#endif
	if (crtc->state && crtc->funcs->atomic_destroy_state)
		crtc->funcs->atomic_destroy_state(crtc, crtc->state);

#ifndef __QNX__
	kfree(crtc->name);
#endif

	memset(crtc, 0, sizeof(*crtc));
}

/**
 * drm_encoder_init - Init a preallocated encoder
 * @dev: drm device
 * @encoder: the encoder to init
 * @funcs: callbacks for this encoder
 * @encoder_type: user visible type of the encoder
 * @name: printf style format string for the encoder name, or NULL for default name
 *
 * Initialises a preallocated encoder. Encoder should be
 * subclassed as part of driver encoder objects.
 *
 * Returns:
 * Zero on success, error code on failure.
 */
int drm_encoder_init(struct drm_device *dev,
		      struct drm_encoder *encoder,
		      const struct drm_encoder_funcs *funcs,
		      int encoder_type, const char *name, ...)
{
	int ret;
	static unsigned lastid = 0;

#ifndef __QNX__
	drm_modeset_lock_all(dev);

	ret = drm_mode_object_get(dev, &encoder->base, DRM_MODE_OBJECT_ENCODER);
	if (ret)
		goto out_unlock;
#endif

	encoder->id = ++ lastid;
	encoder->dev = dev;
	encoder->encoder_type = encoder_type;
	encoder->funcs = funcs;
#ifndef __QNX__
	if (name) {
		va_list ap;

		va_start(ap, name);
		encoder->name = kvasprintf(GFP_KERNEL, name, ap);
		va_end(ap);
	} else {
		encoder->name = kasprintf(GFP_KERNEL, "%s-%d",
					  drm_encoder_enum_list[encoder_type].name,
					  encoder->base.id);
	}
	if (!encoder->name) {
		ret = -ENOMEM;
		goto out_put;
	}
#endif

	list_add_tail(&encoder->head, &dev->mode_config.encoder_list);
	encoder->index = dev->mode_config.num_encoder++;

#ifndef __QNX__
out_put:
	if (ret)
		drm_mode_object_unregister(dev, &encoder->base);
#endif

#ifndef __QNX__
out_unlock:
	drm_modeset_unlock_all(dev);
#endif

	return ret;
}

/**
 * drm_encoder_cleanup - cleans up an initialised encoder
 * @encoder: encoder to cleanup
 *
 * Cleans up the encoder but doesn't free the object.
 */
void drm_encoder_cleanup(struct drm_encoder *encoder)
{
	struct drm_device *dev = encoder->dev;

	/* Note that the encoder_list is considered to be static; should we
	 * remove the drm_encoder at runtime we would have to decrement all
	 * the indices on the drm_encoder after us in the encoder_list.
	 */

#ifndef __QNX__
	drm_modeset_lock_all(dev);
	drm_mode_object_unregister(dev, &encoder->base);
	kfree(encoder->name);
#endif
	list_del(&encoder->head);
	dev->mode_config.num_encoder--;
#ifndef __QNX__
	drm_modeset_unlock_all(dev);
#endif

	memset(encoder, 0, sizeof(*encoder));
}

#ifndef __WATCOMC__
static __maybe_unused unsigned int drm_num_planes(struct drm_device *dev)
{
	unsigned int num = 0;
	struct drm_plane *tmp;

	drm_for_each_plane(tmp, dev) {
		num++;
	}

	return num;
}
#endif

/**
 * drm_universal_plane_init - Initialize a new universal plane object
 * @dev: DRM device
 * @plane: plane object to init
 * @possible_crtcs: bitmask of possible CRTCs
 * @funcs: callbacks for the new plane
 * @formats: array of supported formats (%DRM_FORMAT_*)
 * @format_count: number of elements in @formats
 * @type: type of plane (overlay, primary, cursor)
 * @name: printf style format string for the plane name, or NULL for default name
 *
 * Initializes a plane object of type @type.
 *
 * Returns:
 * Zero on success, error code on failure.
 */
int drm_universal_plane_init(struct drm_device *dev, struct drm_plane *plane,
			     unsigned long possible_crtcs,
			     const struct drm_plane_funcs *funcs,
			     const uint32_t *formats, unsigned int format_count,
			     enum drm_plane_type type,
			     const char *name, ...)
{
	struct drm_mode_config *config = &dev->mode_config;
#ifndef __QNX__
	int ret;

	ret = drm_mode_object_get(dev, &plane->base, DRM_MODE_OBJECT_PLANE);
	if (ret)
		return ret;

	drm_modeset_lock_init(&plane->mutex);

	plane->base.properties = &plane->properties;
#endif
	plane->dev = dev;
	plane->funcs = funcs;
	plane->format_types = kmalloc_array(format_count, sizeof(uint32_t),
					    GFP_KERNEL);
	if (!plane->format_types) {
#ifndef __QNX__
		DRM_DEBUG_KMS("out of memory when allocating plane\n");
		drm_mode_object_unregister(dev, &plane->base);
#else
        fprintf( stderr, "%s(): error: out of memory when allocating plane\n", __FUNCTION__ );
#endif
		return -ENOMEM;
	}

#ifndef __QNX__
	if (name) {
		va_list ap;

		va_start(ap, name);
		plane->name = kvasprintf(GFP_KERNEL, name, ap);
		va_end(ap);
	} else {
		plane->name = kasprintf(GFP_KERNEL, "plane-%d",
					drm_num_planes(dev));
	}
	if (!plane->name) {
		kfree(plane->format_types);
		drm_mode_object_unregister(dev, &plane->base);
		return -ENOMEM;
	}
#endif

	memcpy(plane->format_types, formats, format_count * sizeof(uint32_t));
	plane->format_count = format_count;
	plane->possible_crtcs = possible_crtcs;
	plane->type = type;

	list_add_tail(&plane->head, &config->plane_list);
	plane->index = config->num_total_plane++;
	if (plane->type == DRM_PLANE_TYPE_OVERLAY)
		config->num_overlay_plane++;

#ifndef __QNX__
	drm_object_attach_property(&plane->base,
				   config->plane_type_property,
				   plane->type);

	if (drm_core_check_feature(dev, DRIVER_ATOMIC)) {
		drm_object_attach_property(&plane->base, config->prop_fb_id, 0);
		drm_object_attach_property(&plane->base, config->prop_crtc_id, 0);
		drm_object_attach_property(&plane->base, config->prop_crtc_x, 0);
		drm_object_attach_property(&plane->base, config->prop_crtc_y, 0);
		drm_object_attach_property(&plane->base, config->prop_crtc_w, 0);
		drm_object_attach_property(&plane->base, config->prop_crtc_h, 0);
		drm_object_attach_property(&plane->base, config->prop_src_x, 0);
		drm_object_attach_property(&plane->base, config->prop_src_y, 0);
		drm_object_attach_property(&plane->base, config->prop_src_w, 0);
		drm_object_attach_property(&plane->base, config->prop_src_h, 0);
	}
#endif

	return 0;
}

/**
 * drm_plane_init - Initialize a legacy plane
 * @dev: DRM device
 * @plane: plane object to init
 * @possible_crtcs: bitmask of possible CRTCs
 * @funcs: callbacks for the new plane
 * @formats: array of supported formats (%DRM_FORMAT_*)
 * @format_count: number of elements in @formats
 * @is_primary: plane type (primary vs overlay)
 *
 * Legacy API to initialize a DRM plane.
 *
 * New drivers should call drm_universal_plane_init() instead.
 *
 * Returns:
 * Zero on success, error code on failure.
 */
int drm_plane_init(struct drm_device *dev, struct drm_plane *plane,
		   unsigned long possible_crtcs,
		   const struct drm_plane_funcs *funcs,
		   const uint32_t *formats, unsigned int format_count,
		   bool is_primary)
{
	enum drm_plane_type type;

	type = is_primary ? DRM_PLANE_TYPE_PRIMARY : DRM_PLANE_TYPE_OVERLAY;
	return drm_universal_plane_init(dev, plane, possible_crtcs, funcs,
					formats, format_count, type, NULL);
}

/**
 * drm_plane_cleanup - Clean up the core plane usage
 * @plane: plane to cleanup
 *
 * This function cleans up @plane and removes it from the DRM mode setting
 * core. Note that the function does *not* free the plane structure itself,
 * this is the responsibility of the caller.
 */
void drm_plane_cleanup(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;

#ifndef __QNX__
	drm_modeset_lock_all(dev);
#endif
	free(plane->format_types);
#ifndef __QNX__
	drm_mode_object_unregister(dev, &plane->base);
#endif

#ifndef __QNX__
	BUG_ON(list_empty(&plane->head));
#else
    if ( list_empty(&plane->head) )
        fprintf( stderr, "%s(): error: plane list is empty\n", __FUNCTION__ );
#endif

	/* Note that the plane_list is considered to be static; should we
	 * remove the drm_plane at runtime we would have to decrement all
	 * the indices on the drm_plane after us in the plane_list.
	 */

	list_del(&plane->head);
	dev->mode_config.num_total_plane--;
	if (plane->type == DRM_PLANE_TYPE_OVERLAY)
		dev->mode_config.num_overlay_plane--;
#ifndef __QNX__
	drm_modeset_unlock_all(dev);

	WARN_ON(plane->state && !plane->funcs->atomic_destroy_state);
#endif
	if (plane->state && plane->funcs->atomic_destroy_state)
		plane->funcs->atomic_destroy_state(plane, plane->state);

#ifndef __QNX__
	free(plane->name);
#endif

	memset(plane, 0, sizeof(*plane));
}

#ifndef __WATCOMC__
/**
 * drm_plane_from_index - find the registered plane at an index
 * @dev: DRM device
 * @idx: index of registered plane to find for
 *
 * Given a plane index, return the registered plane from DRM device's
 * list of planes with matching index.
 */
struct drm_plane *
drm_plane_from_index(struct drm_device *dev, int idx)
{
	struct drm_plane *plane;

	drm_for_each_plane(plane, dev)
		if (idx == plane->index)
			return plane;

	return NULL;
}
#endif

/**
 * drm_mode_config_init - initialize DRM mode_configuration structure
 * @dev: DRM device
 *
 * Initialize @dev's mode_config structure, used for tracking the graphics
 * configuration of @dev.
 *
 * Since this initializes the modeset locks, no locking is possible. Which is no
 * problem, since this should happen single threaded at init time. It is the
 * driver's problem to ensure this guarantee.
 *
 */
void drm_mode_config_init(struct drm_device *dev)
{
#ifndef __QNX__
	mutex_init(&dev->mode_config.mutex);
	drm_modeset_lock_init(&dev->mode_config.connection_mutex);
	mutex_init(&dev->mode_config.idr_mutex);
	mutex_init(&dev->mode_config.fb_lock);
	mutex_init(&dev->mode_config.blob_lock);
	INIT_LIST_HEAD(&dev->mode_config.fb_list);
#endif
	INIT_LIST_HEAD(&dev->mode_config.crtc_list);
	INIT_LIST_HEAD(&dev->mode_config.connector_list);
	INIT_LIST_HEAD(&dev->mode_config.encoder_list);
#ifndef __QNX__
	INIT_LIST_HEAD(&dev->mode_config.property_list);
	INIT_LIST_HEAD(&dev->mode_config.property_blob_list);
#endif
	INIT_LIST_HEAD(&dev->mode_config.plane_list);
#ifndef __QNX__
	idr_init(&dev->mode_config.crtc_idr);
	idr_init(&dev->mode_config.tile_idr);
	ida_init(&dev->mode_config.connector_ida);

	drm_modeset_lock_all(dev);
	drm_mode_create_standard_properties(dev);
	drm_modeset_unlock_all(dev);

	/* Just to be sure */
	dev->mode_config.num_fb = 0;
	dev->mode_config.num_connector = 0;
	dev->mode_config.num_crtc = 0;
	dev->mode_config.num_encoder = 0;
#endif
	dev->mode_config.num_overlay_plane = 0;
	dev->mode_config.num_total_plane = 0;
}

#ifndef __WATCOMC__
/**
 * drm_mode_config_cleanup - free up DRM mode_config info
 * @dev: DRM device
 *
 * Free up all the connectors and CRTCs associated with this DRM device, then
 * free up the framebuffers and associated buffer objects.
 *
 * Note that since this /should/ happen single-threaded at driver/device
 * teardown time, no locking is required. It's the driver's job to ensure that
 * this guarantee actually holds true.
 *
 * FIXME: cleanup any dangling user buffer objects too
 */
void drm_mode_config_cleanup(struct drm_device *dev)
{
	struct drm_connector *connector, *ot;
	struct drm_crtc *crtc, *ct;
	struct drm_encoder *encoder, *enct;
#ifndef __QNX__
	struct drm_framebuffer *fb, *fbt;
	struct drm_property *property, *pt;
	struct drm_property_blob *blob, *bt;
#endif
	struct drm_plane *plane, *plt;

	list_for_each_entry_safe(encoder, enct, &dev->mode_config.encoder_list,
				 head) {
		encoder->funcs->destroy(encoder);
	}

	list_for_each_entry_safe(connector, ot,
				 &dev->mode_config.connector_list, head) {
		connector->funcs->destroy(connector);
	}
	
#ifndef __QNX__
	list_for_each_entry_safe(property, pt, &dev->mode_config.property_list,
				 head) {
		drm_property_destroy(dev, property);
	}
#endif

	list_for_each_entry_safe(plane, plt, &dev->mode_config.plane_list,
				 head) {
		plane->funcs->destroy(plane);
	}

	list_for_each_entry_safe(crtc, ct, &dev->mode_config.crtc_list, head) {
		crtc->funcs->destroy(crtc);
	}

#ifndef __QNX__
	list_for_each_entry_safe(blob, bt, &dev->mode_config.property_blob_list,
				 head_global) {
		drm_property_unreference_blob(blob);
	}

	/*
	 * Single-threaded teardown context, so it's not required to grab the
	 * fb_lock to protect against concurrent fb_list access. Contrary, it
	 * would actually deadlock with the drm_framebuffer_cleanup function.
	 *
	 * Also, if there are any framebuffers left, that's a driver leak now,
	 * so politely WARN about this.
	 */
	WARN_ON(!list_empty(&dev->mode_config.fb_list));
	list_for_each_entry_safe(fb, fbt, &dev->mode_config.fb_list, head) {
		drm_framebuffer_free(&fb->base.refcount);
	}

	ida_destroy(&dev->mode_config.connector_ida);
	idr_destroy(&dev->mode_config.tile_idr);
	idr_destroy(&dev->mode_config.crtc_idr);
	drm_modeset_lock_fini(&dev->mode_config.connection_mutex);
#endif
}
#endif
