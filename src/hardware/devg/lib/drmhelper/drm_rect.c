/*
 * Copyright (C) 2011-2013 Intel Corporation
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//#include <linux/errno.h>
//#include <linux/export.h>
#include <drm/drmP.h>
#include <linux/kernel.h>
#include <drm/drm_rect.h>

static int drm_calc_scale(int src, int dst)
{
	int scale = 0;

	if (src < 0 || dst < 0)
		return -EINVAL;

	if (dst == 0)
		return 0;

	scale = src / dst;

	return scale;
}

/**
 * drm_rect_calc_hscale - calculate the horizontal scaling factor
 * @src: source window rectangle
 * @dst: destination window rectangle
 * @min_hscale: minimum allowed horizontal scaling factor
 * @max_hscale: maximum allowed horizontal scaling factor
 *
 * Calculate the horizontal scaling factor as
 * (@src width) / (@dst width).
 *
 * RETURNS:
 * The horizontal scaling factor, or errno of out of limits.
 */
int drm_rect_calc_hscale(const struct drm_rect *src,
			 const struct drm_rect *dst,
			 int min_hscale, int max_hscale)
{
	int src_w = drm_rect_width(src);
	int dst_w = drm_rect_width(dst);
	int hscale = drm_calc_scale(src_w, dst_w);

	if (hscale < 0 || dst_w == 0)
		return hscale;

	if (hscale < min_hscale || hscale > max_hscale)
		return -ERANGE;

	return hscale;
}

/**
 * drm_rect_calc_vscale - calculate the vertical scaling factor
 * @src: source window rectangle
 * @dst: destination window rectangle
 * @min_vscale: minimum allowed vertical scaling factor
 * @max_vscale: maximum allowed vertical scaling factor
 *
 * Calculate the vertical scaling factor as
 * (@src height) / (@dst height).
 *
 * RETURNS:
 * The vertical scaling factor, or errno of out of limits.
 */
int drm_rect_calc_vscale(const struct drm_rect *src,
			 const struct drm_rect *dst,
			 int min_vscale, int max_vscale)
{
	int src_h = drm_rect_height(src);
	int dst_h = drm_rect_height(dst);
	int vscale = drm_calc_scale(src_h, dst_h);

	if (vscale < 0 || dst_h == 0)
		return vscale;

	if (vscale < min_vscale || vscale > max_vscale)
		return -ERANGE;

	return vscale;
}

/**
 * drm_calc_hscale_relaxed - calculate the horizontal scaling factor
 * @src: source window rectangle
 * @dst: destination window rectangle
 * @min_hscale: minimum allowed horizontal scaling factor
 * @max_hscale: maximum allowed horizontal scaling factor
 *
 * Calculate the horizontal scaling factor as
 * (@src width) / (@dst width).
 *
 * If the calculated scaling factor is below @min_vscale,
 * decrease the height of rectangle @dst to compensate.
 *
 * If the calculated scaling factor is above @max_vscale,
 * decrease the height of rectangle @src to compensate.
 *
 * RETURNS:
 * The horizontal scaling factor.
 */
int drm_rect_calc_hscale_relaxed(struct drm_rect *src,
				 struct drm_rect *dst,
				 int min_hscale, int max_hscale)
{
	int src_w = drm_rect_width(src);
	int dst_w = drm_rect_width(dst);
	int hscale = drm_calc_scale(src_w, dst_w);

	if (hscale < 0 || dst_w == 0)
		return hscale;

	if (hscale < min_hscale) {
		int max_dst_w = src_w / min_hscale;

		drm_rect_adjust_size(dst, max_dst_w - dst_w, 0);

		return min_hscale;
	}

	if (hscale > max_hscale) {
		int max_src_w = dst_w * max_hscale;

		drm_rect_adjust_size(src, max_src_w - src_w, 0);

		return max_hscale;
	}

	return hscale;
}

/**
 * drm_rect_calc_vscale_relaxed - calculate the vertical scaling factor
 * @src: source window rectangle
 * @dst: destination window rectangle
 * @min_vscale: minimum allowed vertical scaling factor
 * @max_vscale: maximum allowed vertical scaling factor
 *
 * Calculate the vertical scaling factor as
 * (@src height) / (@dst height).
 *
 * If the calculated scaling factor is below @min_vscale,
 * decrease the height of rectangle @dst to compensate.
 *
 * If the calculated scaling factor is above @max_vscale,
 * decrease the height of rectangle @src to compensate.
 *
 * RETURNS:
 * The vertical scaling factor.
 */
int drm_rect_calc_vscale_relaxed(struct drm_rect *src,
				 struct drm_rect *dst,
				 int min_vscale, int max_vscale)
{
	int src_h = drm_rect_height(src);
	int dst_h = drm_rect_height(dst);
	int vscale = drm_calc_scale(src_h, dst_h);

	if (vscale < 0 || dst_h == 0)
		return vscale;

	if (vscale < min_vscale) {
		int max_dst_h = src_h / min_vscale;

		drm_rect_adjust_size(dst, 0, max_dst_h - dst_h);

		return min_vscale;
	}

	if (vscale > max_vscale) {
		int max_src_h = dst_h * max_vscale;

		drm_rect_adjust_size(src, 0, max_src_h - src_h);

		return max_vscale;
	}

	return vscale;
}

/**
 * drm_rect_rotate - Rotate the rectangle
 * @r: rectangle to be rotated
 * @width: Width of the coordinate space
 * @height: Height of the coordinate space
 * @rotation: Transformation to be applied
 *
 * Apply @rotation to the coordinates of rectangle @r.
 *
 * @width and @height combined with @rotation define
 * the location of the new origin.
 *
 * @width correcsponds to the horizontal and @height
 * to the vertical axis of the untransformed coordinate
 * space.
 */
void drm_rect_rotate(struct drm_rect *r,
		     int width, int height,
		     unsigned int rotation)
{
	struct drm_rect tmp;

	if (rotation & (BIT(DRM_REFLECT_X) | BIT(DRM_REFLECT_Y))) {
		tmp = *r;

		if (rotation & BIT(DRM_REFLECT_X)) {
			r->x1 = width - tmp.x2;
			r->x2 = width - tmp.x1;
		}

		if (rotation & BIT(DRM_REFLECT_Y)) {
			r->y1 = height - tmp.y2;
			r->y2 = height - tmp.y1;
		}
	}

	switch (rotation & DRM_ROTATE_MASK) {
	case BIT(DRM_ROTATE_0):
		break;
	case BIT(DRM_ROTATE_90):
		tmp = *r;
		r->x1 = tmp.y1;
		r->x2 = tmp.y2;
		r->y1 = width - tmp.x2;
		r->y2 = width - tmp.x1;
		break;
	case BIT(DRM_ROTATE_180):
		tmp = *r;
		r->x1 = width - tmp.x2;
		r->x2 = width - tmp.x1;
		r->y1 = height - tmp.y2;
		r->y2 = height - tmp.y1;
		break;
	case BIT(DRM_ROTATE_270):
		tmp = *r;
		r->x1 = height - tmp.y2;
		r->x2 = height - tmp.y1;
		r->y1 = tmp.x1;
		r->y2 = tmp.x2;
		break;
	default:
		break;
	}
}

/**
 * drm_rect_rotate_inv - Inverse rotate the rectangle
 * @r: rectangle to be rotated
 * @width: Width of the coordinate space
 * @height: Height of the coordinate space
 * @rotation: Transformation whose inverse is to be applied
 *
 * Apply the inverse of @rotation to the coordinates
 * of rectangle @r.
 *
 * @width and @height combined with @rotation define
 * the location of the new origin.
 *
 * @width correcsponds to the horizontal and @height
 * to the vertical axis of the original untransformed
 * coordinate space, so that you never have to flip
 * them when doing a rotatation and its inverse.
 * That is, if you do:
 *
 * drm_rotate(&r, width, height, rotation);
 * drm_rotate_inv(&r, width, height, rotation);
 *
 * you will always get back the original rectangle.
 */
void drm_rect_rotate_inv(struct drm_rect *r,
			 int width, int height,
			 unsigned int rotation)
{
	struct drm_rect tmp;

	switch (rotation & DRM_ROTATE_MASK) {
	case BIT(DRM_ROTATE_0):
		break;
	case BIT(DRM_ROTATE_90):
		tmp = *r;
		r->x1 = width - tmp.y2;
		r->x2 = width - tmp.y1;
		r->y1 = tmp.x1;
		r->y2 = tmp.x2;
		break;
	case BIT(DRM_ROTATE_180):
		tmp = *r;
		r->x1 = width - tmp.x2;
		r->x2 = width - tmp.x1;
		r->y1 = height - tmp.y2;
		r->y2 = height - tmp.y1;
		break;
	case BIT(DRM_ROTATE_270):
		tmp = *r;
		r->x1 = tmp.y1;
		r->x2 = tmp.y2;
		r->y1 = height - tmp.x2;
		r->y2 = height - tmp.x1;
		break;
	default:
		break;
	}

	if (rotation & (BIT(DRM_REFLECT_X) | BIT(DRM_REFLECT_Y))) {
		tmp = *r;

		if (rotation & BIT(DRM_REFLECT_X)) {
			r->x1 = width - tmp.x2;
			r->x2 = width - tmp.x1;
		}

		if (rotation & BIT(DRM_REFLECT_Y)) {
			r->y1 = height - tmp.y2;
			r->y2 = height - tmp.y1;
		}
	}
}
