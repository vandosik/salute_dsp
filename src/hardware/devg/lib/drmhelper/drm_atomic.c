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
#include <linux/err.h>
#endif
#include <drm/drmP.h>
#include <drm/drm_atomic.h>
//#include <drm/drm_mode.h>
//#include <drm/drm_plane_helper.h>

//#include "drm_crtc_internal.h"

/**
 * drm_atomic_state_default_release -
 * release memory initialized by drm_atomic_state_init
 * @state: atomic state
 *
 * Free all the memory allocated by drm_atomic_state_init.
 * This is useful for drivers that subclass the atomic state.
 */
void drm_atomic_state_default_release(struct drm_atomic_state *state)
{
#ifndef __QNX__
	kfree(state->connectors);
	kfree(state->crtcs);
	kfree(state->planes);
#else
    if ( state->crtcs )
        free( state->crtcs );
    if ( state->planes )
        free( state->planes );
#endif
}

/**
 * drm_atomic_state_init - init new atomic state
 * @dev: DRM device
 * @state: atomic state
 *
 * Default implementation for filling in a new atomic state.
 * This is useful for drivers that subclass the atomic state.
 */
int
drm_atomic_state_init(struct drm_device *dev, struct drm_atomic_state *state)
{
	/* TODO legacy paths should maybe do a better job about
	 * setting this appropriately?
	 */
#ifndef __QNX__
	state->allow_modeset = true;
#endif

#ifndef __QNX__
	state->crtcs = kcalloc(dev->mode_config.num_crtc,
			       sizeof(*state->crtcs), GFP_KERNEL);
	if (!state->crtcs)
		goto fail;
	state->planes = kcalloc(dev->mode_config.num_total_plane,
				sizeof(*state->planes), GFP_KERNEL);
	if (!state->planes)
		goto fail;
#else
    state->crtcs = calloc(dev->mode_config.num_crtc + 0x10 /* if not all CRTCs allocated */,
                   sizeof(*state->crtcs));
    if (!state->crtcs)
        goto fail;
    state->planes = calloc(dev->mode_config.num_total_plane + 0x10 /* if not all planes allocated */,
                    sizeof(*state->planes));
    if (!state->planes)
        goto fail;
#endif

	state->dev = dev;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("Allocated atomic state %p\n", state);
#endif

	return 0;
fail:
	drm_atomic_state_default_release(state);
	return -ENOMEM;
}

/**
 * drm_atomic_state_alloc - allocate atomic state
 * @dev: DRM device
 *
 * This allocates an empty atomic state to track updates.
 */
struct drm_atomic_state *
drm_atomic_state_alloc(struct drm_device *dev)
{
	struct drm_mode_config *config = &dev->mode_config;
	struct drm_atomic_state *state;

	if (!config->funcs->atomic_state_alloc) {
#ifndef __QNX__
		state = kzalloc(sizeof(*state), GFP_KERNEL);
#else
		state = calloc(1, sizeof(*state));
#endif
		if (!state)
			return NULL;
		if (drm_atomic_state_init(dev, state) < 0) {
#ifndef __QNX__
			kfree(state);
#else
			free(state);
#endif
			return NULL;
		}
		return state;
	}

	return config->funcs->atomic_state_alloc(dev);
}

/**
 * drm_atomic_state_default_clear - clear base atomic state
 * @state: atomic state
 *
 * Default implementation for clearing atomic state.
 * This is useful for drivers that subclass the atomic state.
 */
void drm_atomic_state_default_clear(struct drm_atomic_state *state)
{
	struct drm_device *dev = state->dev;
	struct drm_mode_config *config = &dev->mode_config;
	int i;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("Clearing atomic state %p\n", state);

	for (i = 0; i < state->num_connector; i++) {
		struct drm_connector *connector = state->connectors[i].ptr;

		if (!connector)
			continue;

		connector->funcs->atomic_destroy_state(connector,
						       state->connectors[i].state);
		state->connectors[i].ptr = NULL;
		state->connectors[i].state = NULL;
		drm_connector_unreference(connector);
	}
#endif

	for (i = 0; i < config->num_crtc; i++) {
		struct drm_crtc *crtc = state->crtcs[i].ptr;

		if (!crtc)
			continue;

		crtc->funcs->atomic_destroy_state(crtc,
						  state->crtcs[i].state);

#ifndef __QNX__
		if (state->crtcs[i].commit) {
			kfree(state->crtcs[i].commit->event);
			state->crtcs[i].commit->event = NULL;
			drm_crtc_commit_put(state->crtcs[i].commit);
		}

		state->crtcs[i].commit = NULL;
#endif
		state->crtcs[i].ptr = NULL;
		state->crtcs[i].state = NULL;
	}

	for (i = 0; i < config->num_total_plane; i++) {
		struct drm_plane *plane = state->planes[i].ptr;

		if (!plane)
			continue;

		plane->funcs->atomic_destroy_state(plane,
						   state->planes[i].state);
		state->planes[i].ptr = NULL;
		state->planes[i].state = NULL;
	}
}

/**
 * drm_atomic_state_clear - clear state object
 * @state: atomic state
 *
 * When the w/w mutex algorithm detects a deadlock we need to back off and drop
 * all locks. So someone else could sneak in and change the current modeset
 * configuration. Which means that all the state assembled in @state is no
 * longer an atomic update to the current state, but to some arbitrary earlier
 * state. Which could break assumptions the driver's ->atomic_check likely
 * relies on.
 *
 * Hence we must clear all cached state and completely start over, using this
 * function.
 */
void drm_atomic_state_clear(struct drm_atomic_state *state)
{
	struct drm_device *dev = state->dev;
	struct drm_mode_config *config = &dev->mode_config;

	if (config->funcs->atomic_state_clear)
		config->funcs->atomic_state_clear(state);
	else
		drm_atomic_state_default_clear(state);
}

/**
 * drm_atomic_get_crtc_state - get crtc state
 * @state: global atomic state object
 * @crtc: crtc to get state object for
 *
 * This function returns the crtc state for the given crtc, allocating it if
 * needed. It will also grab the relevant crtc lock to make sure that the state
 * is consistent.
 *
 * Returns:
 *
 * Either the allocated state or the error code encoded into the pointer. When
 * the error is EDEADLK then the w/w mutex code has detected a deadlock and the
 * entire atomic sequence must be restarted. All other errors are fatal.
 */
struct drm_crtc_state *
drm_atomic_get_crtc_state(struct drm_atomic_state *state,
			  struct drm_crtc *crtc)
{
	int ret, index = drm_crtc_index(crtc);
	struct drm_crtc_state *crtc_state;

#ifndef __QNX__
	WARN_ON(!state->acquire_ctx);
#endif

	crtc_state = drm_atomic_get_existing_crtc_state(state, crtc);
	if (crtc_state)
		return crtc_state;

#ifndef __QNX__
	ret = drm_modeset_lock(&crtc->mutex, state->acquire_ctx);
	if (ret)
		return ERR_PTR(ret);
#endif

	crtc_state = crtc->funcs->atomic_duplicate_state(crtc);
	if (!crtc_state)
		return ERR_PTR(-ENOMEM);

	state->crtcs[index].state = crtc_state;
	state->crtcs[index].ptr = crtc;
	crtc_state->state = state;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("Added [CRTC:%d:%s] %p state to %p\n",
			 crtc->base.id, crtc->name, crtc_state, state);
#endif

	return crtc_state;
}

/**
 * drm_atomic_get_plane_state - get plane state
 * @state: global atomic state object
 * @plane: plane to get state object for
 *
 * This function returns the plane state for the given plane, allocating it if
 * needed. It will also grab the relevant plane lock to make sure that the state
 * is consistent.
 *
 * Returns:
 *
 * Either the allocated state or the error code encoded into the pointer. When
 * the error is EDEADLK then the w/w mutex code has detected a deadlock and the
 * entire atomic sequence must be restarted. All other errors are fatal.
 */
struct drm_plane_state *
drm_atomic_get_plane_state(struct drm_atomic_state *state,
			  struct drm_plane *plane)
{
	int ret, index = drm_plane_index(plane);
	struct drm_plane_state *plane_state;

#ifndef __QNX__
	WARN_ON(!state->acquire_ctx);
#endif

	plane_state = drm_atomic_get_existing_plane_state(state, plane);
	if (plane_state)
		return plane_state;

#ifndef __QNX__
	ret = drm_modeset_lock(&plane->mutex, state->acquire_ctx);
	if (ret)
		return ERR_PTR(ret);
#endif

	plane_state = plane->funcs->atomic_duplicate_state(plane);
	if (!plane_state)
		return ERR_PTR(-ENOMEM);

	state->planes[index].state = plane_state;
	state->planes[index].ptr = plane;
	plane_state->state = state;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("Added [PLANE:%d:%s] %p state to %p\n",
			 plane->base.id, plane->name, plane_state, state);
#endif

	if (plane_state->crtc) {
		struct drm_crtc_state *crtc_state;

		crtc_state = drm_atomic_get_crtc_state(state,
						       plane_state->crtc);
		if (IS_ERR(crtc_state))
			return ERR_CAST(crtc_state);
	}

	return plane_state;
}

/**
 * drm_atomic_plane_check - check plane state
 * @plane: plane to check
 * @state: plane state to check
 *
 * Provides core sanity checks for plane state.
 *
 * RETURNS:
 * Zero on success, error code on failure
 */
static int drm_atomic_plane_check(struct drm_plane *plane,
		struct drm_plane_state *state)
{
	unsigned int fb_width, fb_height;
	int ret;

#ifndef __QNX__
	/* either *both* CRTC and FB must be set, or neither */
	if (WARN_ON(state->crtc && !state->fb)) {
		DRM_DEBUG_ATOMIC("CRTC set but no FB\n");
		return -EINVAL;
	} else if (WARN_ON(state->fb && !state->crtc)) {
		DRM_DEBUG_ATOMIC("FB set but no CRTC\n");
		return -EINVAL;
	}
#endif

	/* if disabled, we don't care about the rest of the state: */
	if (!state->crtc)
		return 0;

	/* Check whether this plane is usable on this CRTC */
	if (!(plane->possible_crtcs & drm_crtc_mask(state->crtc))) {
#ifndef __QNX__
		DRM_DEBUG_ATOMIC("Invalid crtc for plane\n");
#endif
		return -EINVAL;
	}

#ifndef __QNX__
	/* Check whether this plane supports the fb pixel format. */
	ret = drm_plane_check_pixel_format(plane, state->fb->pixel_format);
	if (ret) {
		DRM_DEBUG_ATOMIC("Invalid pixel format %s\n",
				 drm_get_format_name(state->fb->pixel_format));
		return ret;
	}

	/* Give drivers some help against integer overflows */
	if (state->crtc_w > INT_MAX ||
	    state->crtc_x > INT_MAX - (int32_t) state->crtc_w ||
	    state->crtc_h > INT_MAX ||
	    state->crtc_y > INT_MAX - (int32_t) state->crtc_h) {
		DRM_DEBUG_ATOMIC("Invalid CRTC coordinates %ux%u+%d+%d\n",
				 state->crtc_w, state->crtc_h,
				 state->crtc_x, state->crtc_y);
		return -ERANGE;
	}

	fb_width = state->fb->width << 16;
	fb_height = state->fb->height << 16;

	/* Make sure source coordinates are inside the fb. */
	if (state->src_w > fb_width ||
	    state->src_x > fb_width - state->src_w ||
	    state->src_h > fb_height ||
	    state->src_y > fb_height - state->src_h) {
		DRM_DEBUG_ATOMIC("Invalid source coordinates "
				 "%u.%06ux%u.%06u+%u.%06u+%u.%06u\n",
				 state->src_w >> 16, ((state->src_w & 0xffff) * 15625) >> 10,
				 state->src_h >> 16, ((state->src_h & 0xffff) * 15625) >> 10,
				 state->src_x >> 16, ((state->src_x & 0xffff) * 15625) >> 10,
				 state->src_y >> 16, ((state->src_y & 0xffff) * 15625) >> 10);
		return -ENOSPC;
	}

	if (plane_switching_crtc(state->state, plane, state)) {
		DRM_DEBUG_ATOMIC("[PLANE:%d:%s] switching CRTC directly\n",
				 plane->base.id, plane->name);
		return -EINVAL;
	}
#endif

	return 0;
}

/**
 * drm_atomic_set_crtc_for_plane - set crtc for plane
 * @plane_state: the plane whose incoming state to update
 * @crtc: crtc to use for the plane
 *
 * Changing the assigned crtc for a plane requires us to grab the lock and state
 * for the new crtc, as needed. This function takes care of all these details
 * besides updating the pointer in the state object itself.
 *
 * Returns:
 * 0 on success or can fail with -EDEADLK or -ENOMEM. When the error is EDEADLK
 * then the w/w mutex code has detected a deadlock and the entire atomic
 * sequence must be restarted. All other errors are fatal.
 */
int
drm_atomic_set_crtc_for_plane(struct drm_plane_state *plane_state,
			      struct drm_crtc *crtc)
{
	struct drm_plane *plane = plane_state->plane;
	struct drm_crtc_state *crtc_state;

	if (plane_state->crtc) {
		crtc_state = drm_atomic_get_crtc_state(plane_state->state,
						       plane_state->crtc);
#ifndef __QNX__
		if (WARN_ON(IS_ERR(crtc_state)))
#else
		if (IS_ERR(crtc_state))
#endif
			return PTR_ERR(crtc_state);

		crtc_state->plane_mask &= ~(1 << drm_plane_index(plane));
	}

	plane_state->crtc = crtc;

	if (crtc) {
		crtc_state = drm_atomic_get_crtc_state(plane_state->state,
						       crtc);
		if (IS_ERR(crtc_state))
			return PTR_ERR(crtc_state);
		crtc_state->plane_mask |= (1 << drm_plane_index(plane));
	}

#ifndef __QNX__
	if (crtc)
		DRM_DEBUG_ATOMIC("Link plane state %p to [CRTC:%d:%s]\n",
				 plane_state, crtc->base.id, crtc->name);
	else
		DRM_DEBUG_ATOMIC("Link plane state %p to [NOCRTC]\n",
				 plane_state);
#endif

	return 0;
}

/**
 * drm_atomic_check_only - check whether a given config would work
 * @state: atomic configuration to check
 *
 * Note that this function can return -EDEADLK if the driver needed to acquire
 * more locks but encountered a deadlock. The caller must then do the usual w/w
 * backoff dance and restart. All other errors are fatal.
 *
 * Returns:
 * 0 on success, negative error code on failure.
 */
int drm_atomic_check_only(struct drm_atomic_state *state)
{
	struct drm_device *dev = state->dev;
	struct drm_mode_config *config = &dev->mode_config;
	struct drm_plane *plane;
	struct drm_plane_state *plane_state;
#ifndef __QNX__
	struct drm_crtc *crtc;
	struct drm_crtc_state *crtc_state;
#endif
	int i, ret = 0;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("checking %p\n", state);
#endif

	for_each_plane_in_state(state, plane, plane_state, i) {
		ret = drm_atomic_plane_check(plane, plane_state);
		if (ret) {
#ifndef __QNX__
			DRM_DEBUG_ATOMIC("[PLANE:%d:%s] atomic core check failed\n",
					 plane->base.id, plane->name);
#else
            fprintf( stderr, "%s(): error: [PLANE:%d:%s] atomic core check failed\n", __FUNCTION__, plane->type, "" );
#endif
			return ret;
		}
	}

#ifndef __QNX__
	for_each_crtc_in_state(state, crtc, crtc_state, i) {
		ret = drm_atomic_crtc_check(crtc, crtc_state);
		if (ret) {
			DRM_DEBUG_ATOMIC("[CRTC:%d:%s] atomic core check failed\n",
					 crtc->base.id, crtc->name);
			return ret;
		}
	}
#endif

	if (config->funcs->atomic_check)
		ret = config->funcs->atomic_check(state->dev, state);

#ifndef __QNX__
	if (!state->allow_modeset) {
		for_each_crtc_in_state(state, crtc, crtc_state, i) {
			if (drm_atomic_crtc_needs_modeset(crtc_state)) {
				DRM_DEBUG_ATOMIC("[CRTC:%d:%s] requires full modeset\n",
						 crtc->base.id, crtc->name);
				return -EINVAL;
			}
		}
	}
#endif

	return ret;
}

/**
 * drm_atomic_commit - commit configuration atomically
 * @state: atomic configuration to check
 *
 * Note that this function can return -EDEADLK if the driver needed to acquire
 * more locks but encountered a deadlock. The caller must then do the usual w/w
 * backoff dance and restart. All other errors are fatal.
 *
 * Also note that on successful execution ownership of @state is transferred
 * from the caller of this function to the function itself. The caller must not
 * free or in any other way access @state. If the function fails then the caller
 * must clean up @state itself.
 *
 * Returns:
 * 0 on success, negative error code on failure.
 */
int drm_atomic_commit(struct drm_atomic_state *state)
{
	struct drm_mode_config *config = &state->dev->mode_config;
	int ret;

	ret = drm_atomic_check_only(state);
	if (ret)
		return ret;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("commiting %p\n", state);
#endif

	return config->funcs->atomic_commit(state->dev, state, false);
}

/**
 * drm_atomic_nonblocking_commit - atomic&nonblocking configuration commit
 * @state: atomic configuration to check
 *
 * Note that this function can return -EDEADLK if the driver needed to acquire
 * more locks but encountered a deadlock. The caller must then do the usual w/w
 * backoff dance and restart. All other errors are fatal.
 *
 * Also note that on successful execution ownership of @state is transferred
 * from the caller of this function to the function itself. The caller must not
 * free or in any other way access @state. If the function fails then the caller
 * must clean up @state itself.
 *
 * Returns:
 * 0 on success, negative error code on failure.
 */
int drm_atomic_nonblocking_commit(struct drm_atomic_state *state)
{
	struct drm_mode_config *config = &state->dev->mode_config;
	int ret;

	ret = drm_atomic_check_only(state);
	if (ret)
		return ret;

#ifndef __QNX__
	DRM_DEBUG_ATOMIC("commiting %p nonblocking\n", state);
#endif

	return config->funcs->atomic_commit(state->dev, state, true);
}
