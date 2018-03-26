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

#ifndef DRM_ATOMIC_HELPER_H_
#define DRM_ATOMIC_HELPER_H_

#include <drm/drm_crtc.h>

struct drm_atomic_state;

int drm_atomic_helper_check_modeset(struct drm_device *dev,
				struct drm_atomic_state *state);
#ifndef __WATCOMC__
int drm_atomic_helper_check_planes(struct drm_device *dev,
			       struct drm_atomic_state *state);

void
drm_atomic_helper_update_legacy_modeset_state(struct drm_device *dev,
					      struct drm_atomic_state *old_state);

void drm_atomic_helper_commit_planes_on_crtc(struct drm_crtc_state *old_crtc_state);

/* default implementations for state handling */
void __drm_atomic_helper_crtc_destroy_state(struct drm_crtc_state *state);
void drm_atomic_helper_crtc_destroy_state(struct drm_crtc *crtc,
					  struct drm_crtc_state *state);

void __drm_atomic_helper_plane_destroy_state(struct drm_plane_state *state);
void drm_atomic_helper_plane_destroy_state(struct drm_plane *plane,
					  struct drm_plane_state *state);
#endif

/*
 * drm_atomic_plane_disabling - check whether a plane is being disabled
 * @plane: plane object
 * @old_state: previous atomic state
 *
 * Checks the atomic state of a plane to determine whether it's being disabled
 * or not. This also WARNs if it detects an invalid state (both CRTC and FB
 * need to either both be NULL or both be non-NULL).
 *
 * RETURNS:
 * True if the plane is being disabled, false otherwise.
 */
static inline bool
drm_atomic_plane_disabling(struct drm_plane *plane,
			   struct drm_plane_state *old_state)
{
#ifndef __QNX__
	/*
	 * When disabling a plane, CRTC and FB should always be NULL together.
	 * Anything else should be considered a bug in the atomic core, so we
	 * gently warn about it.
	 */
	WARN_ON((plane->state->crtc == NULL && plane->state->fb != NULL) ||
		(plane->state->crtc != NULL && plane->state->fb == NULL));
#endif

	/*
	 * When using the transitional helpers, old_state may be NULL. If so,
	 * we know nothing about the current state and have to assume that it
	 * might be enabled.
	 *
	 * When using the atomic helpers, old_state won't be NULL. Therefore
	 * this check assumes that either the driver will have reconstructed
	 * the correct state in ->reset() or that the driver will have taken
	 * appropriate measures to disable all planes.
	 */
	return (!old_state || old_state->crtc) && !plane->state->crtc;
}

#endif /* DRM_ATOMIC_HELPER_H_ */
