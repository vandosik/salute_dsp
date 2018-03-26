/*
 * Copyright (C) 2014 Red Hat
 * Copyright (C) 2014 Intel Corp.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 * Rob Clark <robdclark@gmail.com>
 * Daniel Vetter <daniel.vetter@ffwll.ch>
 */

#ifdef __QNX__
#include <linux/compiler.h>
#include <drm/drm_fb_helper.h>
#endif
#include <drm/drmP.h>
#include <drm/drm_atomic.h>
//#include <drm/drm_plane_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_atomic_helper.h>
//#include <linux/fence.h>

//#include "drm_crtc_internal.h"

#ifndef __WATCOMC__

/**
 * drm_atomic_helper_check_planes - validate state object for planes changes
 * @dev: DRM device
 * @state: the driver state object
 *
 * Check the state object to see if the requested state is physically possible.
 * This does all the plane update related checks using by calling into the
 * ->atomic_check hooks provided by the driver.
 *
 * It also sets crtc_state->planes_changed to indicate that a crtc has
 * updated planes.
 *
 * RETURNS:
 * Zero for success or -errno
 */
int
drm_atomic_helper_check_planes(struct drm_device *dev,
			       struct drm_atomic_state *state)
{
	struct drm_crtc *crtc;
	struct drm_crtc_state *crtc_state;
	struct drm_plane *plane;
	struct drm_plane_state *plane_state;
	int i, ret = 0;

#ifndef __QNX__
	ret = drm_atomic_helper_normalize_zpos(dev, state);
	if (ret)
		return ret;
#endif

	for_each_plane_in_state(state, plane, plane_state, i) {
		const struct drm_plane_helper_funcs *funcs;

		funcs = plane->helper_private;

#ifndef __QNX__
		drm_atomic_helper_plane_changed(state, plane_state, plane);
#endif

		if (!funcs || !funcs->atomic_check)
			continue;

		ret = funcs->atomic_check(plane, plane_state);
		if (ret) {
#ifndef __QNX__
			DRM_DEBUG_ATOMIC("[PLANE:%d:%s] atomic driver check failed\n",
					 plane->base.id, plane->name);
#endif
			return ret;
		}
	}

	for_each_crtc_in_state(state, crtc, crtc_state, i) {
		const struct drm_crtc_helper_funcs *funcs;

		funcs = crtc->helper_private;

		if (!funcs || !funcs->atomic_check)
			continue;

		ret = funcs->atomic_check(crtc, crtc_state);
		if (ret) {
#ifndef __QNX__
			DRM_DEBUG_ATOMIC("[CRTC:%d:%s] atomic driver check failed\n",
					 crtc->base.id, crtc->name);
#endif
			return ret;
		}
	}

	return ret;
}

/**
 * drm_atomic_helper_update_legacy_modeset_state - update legacy modeset state
 * @dev: DRM device
 * @old_state: atomic state object with old state structures
 *
 * This function updates all the various legacy modeset state pointers in
 * connectors, encoders and crtcs. It also updates the timestamping constants
 * used for precise vblank timestamps by calling
 * drm_calc_timestamping_constants().
 *
 * Drivers can use this for building their own atomic commit if they don't have
 * a pure helper-based modeset implementation.
 */
void
drm_atomic_helper_update_legacy_modeset_state(struct drm_device *dev,
					      struct drm_atomic_state *old_state)
{
#ifndef __QNX__
	struct drm_connector *connector;
	struct drm_connector_state *old_conn_state;
#endif
	struct drm_crtc *crtc;
	struct drm_crtc_state *old_crtc_state;
	int i;

#ifndef __QNX__
	/* clear out existing links and update dpms */
	for_each_connector_in_state(old_state, connector, old_conn_state, i) {
		if (connector->encoder) {
			WARN_ON(!connector->encoder->crtc);

			connector->encoder->crtc = NULL;
			connector->encoder = NULL;
		}

		crtc = connector->state->crtc;
		if ((!crtc && old_conn_state->crtc) ||
		    (crtc && drm_atomic_crtc_needs_modeset(crtc->state))) {
			struct drm_property *dpms_prop =
				dev->mode_config.dpms_property;
			int mode = DRM_MODE_DPMS_OFF;

			if (crtc && crtc->state->active)
				mode = DRM_MODE_DPMS_ON;

			connector->dpms = mode;
			drm_object_property_set_value(&connector->base,
						      dpms_prop, mode);
		}
	}

	/* set new links */
	for_each_connector_in_state(old_state, connector, old_conn_state, i) {
		if (!connector->state->crtc)
			continue;

		if (WARN_ON(!connector->state->best_encoder))
			continue;

		connector->encoder = connector->state->best_encoder;
		connector->encoder->crtc = connector->state->crtc;
	}
#endif

	/* set legacy state in the crtc structure */
	for_each_crtc_in_state(old_state, crtc, old_crtc_state, i) {
		struct drm_plane *primary = crtc->primary;

#ifndef __QNX__
		crtc->mode = crtc->state->mode;
#endif
		crtc->enabled = crtc->state->enable;

		if (drm_atomic_get_existing_plane_state(old_state, primary) &&
		    primary->state->crtc == crtc) {
#ifndef __QNX__
			crtc->x = primary->state->src_x >> 16;
			crtc->y = primary->state->src_y >> 16;
#endif
		}

#ifndef __QNX__
		if (crtc->state->enable)
			drm_calc_timestamping_constants(crtc,
							&crtc->state->adjusted_mode);
#endif
	}
}

/**
 * DOC: overview
 *
 * This helper library provides implementations of check and commit functions on
 * top of the CRTC modeset helper callbacks and the plane helper callbacks. It
 * also provides convenience implementations for the atomic state handling
 * callbacks for drivers which don't need to subclass the drm core structures to
 * add their own additional internal state.
 *
 * This library also provides default implementations for the check callback in
 * drm_atomic_helper_check() and for the commit callback with
 * drm_atomic_helper_commit(). But the individual stages and callbacks are
 * exposed to allow drivers to mix and match and e.g. use the plane helpers only
 * together with a driver private modeset implementation.
 *
 * This library also provides implementations for all the legacy driver
 * interfaces on top of the atomic interface. See drm_atomic_helper_set_config(),
 * drm_atomic_helper_disable_plane(), drm_atomic_helper_disable_plane() and the
 * various functions to implement set_property callbacks. New drivers must not
 * implement these functions themselves but must use the provided helpers.
 *
 * The atomic helper uses the same function table structures as all other
 * modesetting helpers. See the documentation for struct &drm_crtc_helper_funcs,
 * struct &drm_encoder_helper_funcs and struct &drm_connector_helper_funcs. It
 * also shares the struct &drm_plane_helper_funcs function table with the plane
 * helpers.
 */

/**
 * drm_atomic_helper_commit_planes_on_crtc - commit plane state for a crtc
 * @old_crtc_state: atomic state object with the old crtc state
 *
 * This function commits the new plane state using the plane and atomic helper
 * functions for planes on the specific crtc. It assumes that the atomic state
 * has already been pushed into the relevant object state pointers, since this
 * step can no longer fail.
 *
 * This function is useful when plane updates should be done crtc-by-crtc
 * instead of one global step like drm_atomic_helper_commit_planes() does.
 *
 * This function can only be savely used when planes are not allowed to move
 * between different CRTCs because this function doesn't handle inter-CRTC
 * depencies. Callers need to ensure that either no such depencies exist,
 * resolve them through ordering of commit calls or through some other means.
 */
void
drm_atomic_helper_commit_planes_on_crtc(struct drm_crtc_state *old_crtc_state)
{
	const struct drm_crtc_helper_funcs *crtc_funcs;
	struct drm_crtc *crtc = old_crtc_state->crtc;
	struct drm_atomic_state *old_state = old_crtc_state->state;
	struct drm_plane *plane;
	unsigned plane_mask;

	plane_mask = old_crtc_state->plane_mask;
	plane_mask |= crtc->state->plane_mask;

	crtc_funcs = crtc->helper_private;
	if (crtc_funcs && crtc_funcs->atomic_begin)
		crtc_funcs->atomic_begin(crtc, old_crtc_state);

	drm_for_each_plane_mask(plane, crtc->dev, plane_mask) {
		struct drm_plane_state *old_plane_state =
			drm_atomic_get_existing_plane_state(old_state, plane);
		const struct drm_plane_helper_funcs *plane_funcs;

		plane_funcs = plane->helper_private;

		if (!old_plane_state || !plane_funcs)
			continue;

#ifndef __QNX__
		WARN_ON(plane->state->crtc && plane->state->crtc != crtc);
#else
        if ( plane->state->crtc && plane->state->crtc != crtc )
            fprintf( stderr, "%s(): warning: invalid CRTC configuration\n", __FUNCTION__ );
#endif

		if (drm_atomic_plane_disabling(plane, old_plane_state) &&
		    plane_funcs->atomic_disable)
			plane_funcs->atomic_disable(plane, old_plane_state);
		else if (plane->state->crtc ||
			 drm_atomic_plane_disabling(plane, old_plane_state))
			plane_funcs->atomic_update(plane, old_plane_state);
	}

	if (crtc_funcs && crtc_funcs->atomic_flush)
		crtc_funcs->atomic_flush(crtc, old_crtc_state);
}

/**
 * __drm_atomic_helper_crtc_destroy_state - release CRTC state
 * @state: CRTC state object to release
 *
 * Releases all resources stored in the CRTC state without actually freeing
 * the memory of the CRTC state. This is useful for drivers that subclass the
 * CRTC state.
 */
void __drm_atomic_helper_crtc_destroy_state(struct drm_crtc_state *state)
{
#ifndef __QNX__
	drm_property_unreference_blob(state->mode_blob);
	drm_property_unreference_blob(state->degamma_lut);
	drm_property_unreference_blob(state->ctm);
	drm_property_unreference_blob(state->gamma_lut);
#endif
}

/**
 * drm_atomic_helper_crtc_destroy_state - default state destroy hook
 * @crtc: drm CRTC
 * @state: CRTC state object to release
 *
 * Default CRTC state destroy hook for drivers which don't have their own
 * subclassed CRTC state structure.
 */
void drm_atomic_helper_crtc_destroy_state(struct drm_crtc *crtc,
					  struct drm_crtc_state *state)
{
	__drm_atomic_helper_crtc_destroy_state(state);
	free(state);
}


/**
 * __drm_atomic_helper_plane_destroy_state - release plane state
 * @state: plane state object to release
 *
 * Releases all resources stored in the plane state without actually freeing
 * the memory of the plane state. This is useful for drivers that subclass the
 * plane state.
 */
void __drm_atomic_helper_plane_destroy_state(struct drm_plane_state *state)
{
#ifndef __QNX__
	if (state->fb)
		drm_framebuffer_unreference(state->fb);
#endif
}

/**
 * drm_atomic_helper_plane_destroy_state - default state destroy hook
 * @plane: drm plane
 * @state: plane state object to release
 *
 * Default plane state destroy hook for drivers which don't have their own
 * subclassed plane state structure.
 */
void drm_atomic_helper_plane_destroy_state(struct drm_plane *plane,
					   struct drm_plane_state *state)
{
	__drm_atomic_helper_plane_destroy_state(state);
	free(state);
}

#endif  /* __WATCOMC__ */
