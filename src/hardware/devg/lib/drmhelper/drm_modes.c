/*
 * Copyright © 1997-2003 by The XFree86 Project, Inc.
 * Copyright © 2007 Dave Airlie
 * Copyright © 2007-2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 * Copyright 2005-2006 Luc Verhaegen
 * Copyright (c) 2001, Andy Ritger  aritger@nvidia.com
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
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef __QNX__
#include <drm/drmP.h>
#include <string.h>
#include <linux/hdmi.h>
#endif
#include <linux/list.h>
#include <drm/drm_modes.h>
#ifdef __QNX__
#include <drm/drm_edid.h>
#endif

/**
 * drm_mode_duplicate - allocate and duplicate an existing mode
 * @dev: drm_device to allocate the duplicated mode for
 * @mode: mode to duplicate
 *
 * Just allocate a new mode, copy the existing mode into it, and return
 * a pointer to it.  Used to create new instances of established modes.
 *
 * Returns:
 * Pointer to duplicated mode on success, NULL on error.
 */
#ifndef __QNX__
struct drm_display_mode *drm_mode_duplicate(struct drm_device *dev,
					    const struct drm_display_mode *mode);
#else
void drm_mode_duplicate( const disp_crtc_settings_t *settings, struct drm_display_mode *mode )
#endif
{
#ifndef __QNX__
	struct drm_display_mode *nmode;

	nmode = drm_mode_create(dev);
	if (!nmode)
		return NULL;

	drm_mode_copy(nmode, mode);

	return nmode;
#else
	memset( mode, 0, sizeof(*mode) );
	
	/* Proposed mode values */
	mode->clock = settings->pixel_clock;
	mode->hdisplay = settings->xres;
	mode->hsync_start = settings->h_sync_start;
	mode->hsync_end = settings->h_sync_start + settings->h_sync_len;
	mode->htotal = settings->h_total;
	//mode->hskew;
	mode->vdisplay = settings->yres;
	mode->vsync_start = settings->v_sync_start;
	mode->vsync_end = settings->v_sync_start + settings->v_sync_len;
	mode->vtotal = settings->v_total;
	//mode->vscan;
	mode->flags = 0;
	
	if ( settings->sync_polarity & DISP_SYNC_POLARITY_H_POS )
		mode->flags |= DRM_MODE_FLAG_PHSYNC;
	else
		mode->flags |= DRM_MODE_FLAG_NHSYNC;
		
	if ( settings->sync_polarity & DISP_SYNC_POLARITY_V_POS )
		mode->flags |= DRM_MODE_FLAG_PVSYNC;
	else
		mode->flags |= DRM_MODE_FLAG_NVSYNC;
#ifdef DISP_SYNC_INTERLACED
	if ( settings->sync_polarity & DISP_SYNC_INTERLACED )
		mode->flags |= DRM_MODE_FLAG_INTERLACE;
#endif

	/* Addressable image size (may be 0 for projectors, etc.) */
	/*int width_mm;
	int height_mm;*/

	/* Actual mode we give to hw */
	/*int clock_index;
	int synth_clock;*/
	mode->crtc_hdisplay = mode->hdisplay;
	mode->crtc_hblank_start = settings->h_blank_start;
	mode->crtc_hblank_end = settings->h_blank_start + settings->h_blank_len;
	mode->crtc_hsync_start = mode->hsync_start;
	mode->crtc_hsync_end = mode->hsync_end;
	mode->crtc_htotal = mode->htotal;
	//mode->crtc_hskew;
	mode->crtc_vdisplay = mode->vdisplay;
	mode->crtc_vblank_start = settings->v_blank_start;
	mode->crtc_vblank_end = settings->v_blank_start + settings->v_blank_len;
	mode->crtc_vsync_start = mode->vsync_start;
	mode->crtc_vsync_end = mode->vsync_end;
	mode->crtc_vtotal = mode->vtotal;
#if 0
	mode->crtc_hadjusted = mode->hdisplay;
	mode->crtc_vadjusted = mode->vdisplay;
#endif

	mode->vrefresh = settings->refresh;     /* in Hz */
	//mode->hsync = settings->pixel_clock / (settings->h_freq ? settings->h_freq : settings->xres);   /* in kHz */
#endif
}

/**
 * drm_mode_equal_no_clocks_no_stereo - test modes for equality
 * @mode1: first mode
 * @mode2: second mode
 *
 * Check to see if @mode1 and @mode2 are equivalent, but
 * don't check the pixel clocks nor the stereo layout.
 *
 * Returns:
 * True if the modes are equal, false otherwise.
 */
bool drm_mode_equal_no_clocks_no_stereo(const struct drm_display_mode *mode1,
					const struct drm_display_mode *mode2)
{
	if (mode1->hdisplay == mode2->hdisplay &&
	    mode1->hsync_start == mode2->hsync_start &&
	    mode1->hsync_end == mode2->hsync_end &&
	    mode1->htotal == mode2->htotal &&
	    mode1->hskew == mode2->hskew &&
	    mode1->vdisplay == mode2->vdisplay &&
	    mode1->vsync_start == mode2->vsync_start &&
	    mode1->vsync_end == mode2->vsync_end &&
	    mode1->vtotal == mode2->vtotal &&
	    mode1->vscan == mode2->vscan &&
	    (mode1->flags & ~DRM_MODE_FLAG_3D_MASK) ==
	     (mode2->flags & ~DRM_MODE_FLAG_3D_MASK))
		return true;

	return false;
}
