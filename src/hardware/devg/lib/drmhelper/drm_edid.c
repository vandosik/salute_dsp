/*
 * Copyright (c) 2006 Luc Verhaegen (quirks list)
 * Copyright (c) 2007-2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 * Copyright 2010 Red Hat, Inc.
 *
 * DDC probing routines (drm_ddc_read & drm_do_probe_ddc_edid) originally from
 * FB layer.
 *   Copyright (C) 2006 Dennis Munsie <dmunsie@cecropia.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <drm/drmP.h>

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#ifdef __QNXNTO__
#include <stdbool.h>
#endif

#include <linux/hdmi.h>
#include <linux/list.h>
#include <drm/drm_modes.h>
#include <drm/drm_crtc.h>
#include <drm/drm_edid.h>
#include <drm/drm_displayid.h>


#ifdef __QNX__
#define KERN_ERR "DRM error: "

#ifdef __QNXNTO__
#define DRM_DEBUG(fmt...) printf(fmt)
#define DRM_DEBUG_KMS(fmt...) printf(fmt)
#define DRM_ERROR(fmt...) printf(fmt)
#define printk(fmt...) printf(fmt)

#define dev_err(d,fmt...) printf(fmt)
#define dev_warn(d,fmt...) printf(fmt)
#else
#define DRM_DEBUG               printf
#define DRM_DEBUG_KMS           printf
#define DRM_ERROR               printf
#define printk                  printf

#define dev_err                 printf
#define dev_warn                printf
#endif

#define WARN_ON(e) (e)

#define PICOS2KHZ(a) (1000000000UL/(a))
#define KHZ2PICOS(a) (1000000000UL/(a))
#endif


/*
 * Probably taken from CEA-861 spec.
 * This table is converted from xorg's hw/xfree86/modes/xf86EdidModes.c.
 *
 * Index using the VIC.
 */
static const struct drm_display_mode edid_cea_modes[] = {
	/* 0 - dummy, VICs start at 1 */
#ifndef __WATCOMC__
	{ },
#else
    { DRM_MODE("0", DRM_MODE_TYPE_DRIVER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ), .vrefresh = 0, .picture_aspect_ratio = 0, },
#endif
	/* 1 - 640x480@60Hz */
	{ DRM_MODE("640x480", DRM_MODE_TYPE_DRIVER, 25175, 640, 656,
		   752, 800, 0, 480, 490, 492, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 2 - 720x480@60Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 27000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 3 - 720x480@60Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 27000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 4 - 1280x720@60Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1390,
		   1430, 1650, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 5 - 1920x1080i@60Hz */
	{ DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2008,
		   2052, 2200, 0, 1080, 1084, 1094, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 6 - 720(1440)x480i@60Hz */
	{ DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 13500, 720, 739,
		   801, 858, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 7 - 720(1440)x480i@60Hz */
	{ DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 13500, 720, 739,
		   801, 858, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 8 - 720(1440)x240@60Hz */
	{ DRM_MODE("720x240", DRM_MODE_TYPE_DRIVER, 13500, 720, 739,
		   801, 858, 0, 240, 244, 247, 262, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 9 - 720(1440)x240@60Hz */
	{ DRM_MODE("720x240", DRM_MODE_TYPE_DRIVER, 13500, 720, 739,
		   801, 858, 0, 240, 244, 247, 262, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 10 - 2880x480i@60Hz */
	{ DRM_MODE("2880x480i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956,
		   3204, 3432, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 11 - 2880x480i@60Hz */
	{ DRM_MODE("2880x480i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956,
		   3204, 3432, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 12 - 2880x240@60Hz */
	{ DRM_MODE("2880x240", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956,
		   3204, 3432, 0, 240, 244, 247, 262, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 13 - 2880x240@60Hz */
	{ DRM_MODE("2880x240", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2956,
		   3204, 3432, 0, 240, 244, 247, 262, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 14 - 1440x480@60Hz */
	{ DRM_MODE("1440x480", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1472,
		   1596, 1716, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 15 - 1440x480@60Hz */
	{ DRM_MODE("1440x480", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1472,
		   1596, 1716, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 16 - 1920x1080@60Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008,
		   2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 17 - 720x576@50Hz */
	{ DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 27000, 720, 732,
		   796, 864, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 18 - 720x576@50Hz */
	{ DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 27000, 720, 732,
		   796, 864, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 19 - 1280x720@50Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 1720,
		   1760, 1980, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 20 - 1920x1080i@50Hz */
	{ DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2448,
		   2492, 2640, 0, 1080, 1084, 1094, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 21 - 720(1440)x576i@50Hz */
	{ DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 13500, 720, 732,
		   795, 864, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 22 - 720(1440)x576i@50Hz */
	{ DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 13500, 720, 732,
		   795, 864, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 23 - 720(1440)x288@50Hz */
	{ DRM_MODE("720x288", DRM_MODE_TYPE_DRIVER, 13500, 720, 732,
		   795, 864, 0, 288, 290, 293, 312, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 24 - 720(1440)x288@50Hz */
	{ DRM_MODE("720x288", DRM_MODE_TYPE_DRIVER, 13500, 720, 732,
		   795, 864, 0, 288, 290, 293, 312, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 25 - 2880x576i@50Hz */
	{ DRM_MODE("2880x576i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928,
		   3180, 3456, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 26 - 2880x576i@50Hz */
	{ DRM_MODE("2880x576i", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928,
		   3180, 3456, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 27 - 2880x288@50Hz */
	{ DRM_MODE("2880x288", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928,
		   3180, 3456, 0, 288, 290, 293, 312, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 28 - 2880x288@50Hz */
	{ DRM_MODE("2880x288", DRM_MODE_TYPE_DRIVER, 54000, 2880, 2928,
		   3180, 3456, 0, 288, 290, 293, 312, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 29 - 1440x576@50Hz */
	{ DRM_MODE("1440x576", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1464,
		   1592, 1728, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 30 - 1440x576@50Hz */
	{ DRM_MODE("1440x576", DRM_MODE_TYPE_DRIVER, 54000, 1440, 1464,
		   1592, 1728, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 31 - 1920x1080@50Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2448,
		   2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 32 - 1920x1080@24Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2558,
		   2602, 2750, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 24, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 33 - 1920x1080@25Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2448,
		   2492, 2640, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 25, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 34 - 1920x1080@30Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 74250, 1920, 2008,
		   2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 30, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 35 - 2880x480@60Hz */
	{ DRM_MODE("2880x480", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2944,
		   3192, 3432, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 36 - 2880x480@60Hz */
	{ DRM_MODE("2880x480", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2944,
		   3192, 3432, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 60, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 37 - 2880x576@50Hz */
	{ DRM_MODE("2880x576", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2928,
		   3184, 3456, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 38 - 2880x576@50Hz */
	{ DRM_MODE("2880x576", DRM_MODE_TYPE_DRIVER, 108000, 2880, 2928,
		   3184, 3456, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 39 - 1920x1080i@50Hz */
	{ DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 72000, 1920, 1952,
		   2120, 2304, 0, 1080, 1126, 1136, 1250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 50, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 40 - 1920x1080i@100Hz */
	{ DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2448,
		   2492, 2640, 0, 1080, 1084, 1094, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 41 - 1280x720@100Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1720,
		   1760, 1980, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 42 - 720x576@100Hz */
	{ DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 54000, 720, 732,
		   796, 864, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 43 - 720x576@100Hz */
	{ DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 54000, 720, 732,
		   796, 864, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 44 - 720(1440)x576i@100Hz */
	{ DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 27000, 720, 732,
		   795, 864, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 45 - 720(1440)x576i@100Hz */
	{ DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 27000, 720, 732,
		   795, 864, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 46 - 1920x1080i@120Hz */
	{ DRM_MODE("1920x1080i", DRM_MODE_TYPE_DRIVER, 148500, 1920, 2008,
		   2052, 2200, 0, 1080, 1084, 1094, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC |
			DRM_MODE_FLAG_INTERLACE),
	  .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 47 - 1280x720@120Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 148500, 1280, 1390,
		   1430, 1650, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 48 - 720x480@120Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 54000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 49 - 720x480@120Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 54000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 50 - 720(1440)x480i@120Hz */
	{ DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 27000, 720, 739,
		   801, 858, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 51 - 720(1440)x480i@120Hz */
	{ DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 27000, 720, 739,
		   801, 858, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 52 - 720x576@200Hz */
	{ DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 108000, 720, 732,
		   796, 864, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 200, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 53 - 720x576@200Hz */
	{ DRM_MODE("720x576", DRM_MODE_TYPE_DRIVER, 108000, 720, 732,
		   796, 864, 0, 576, 581, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 200, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 54 - 720(1440)x576i@200Hz */
	{ DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 54000, 720, 732,
		   795, 864, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 200, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 55 - 720(1440)x576i@200Hz */
	{ DRM_MODE("720x576i", DRM_MODE_TYPE_DRIVER, 54000, 720, 732,
		   795, 864, 0, 576, 580, 586, 625, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 200, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 56 - 720x480@240Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 108000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 240, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 57 - 720x480@240Hz */
	{ DRM_MODE("720x480", DRM_MODE_TYPE_DRIVER, 108000, 720, 736,
		   798, 858, 0, 480, 489, 495, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC),
	  .vrefresh = 240, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 58 - 720(1440)x480i@240 */
	{ DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 54000, 720, 739,
		   801, 858, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 240, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_4_3, },
	/* 59 - 720(1440)x480i@240 */
	{ DRM_MODE("720x480i", DRM_MODE_TYPE_DRIVER, 54000, 720, 739,
		   801, 858, 0, 480, 488, 494, 525, 0,
		   DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC |
			DRM_MODE_FLAG_INTERLACE | DRM_MODE_FLAG_DBLCLK),
	  .vrefresh = 240, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 60 - 1280x720@24Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 59400, 1280, 3040,
		   3080, 3300, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 24, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 61 - 1280x720@25Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 3700,
		   3740, 3960, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 25, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 62 - 1280x720@30Hz */
	{ DRM_MODE("1280x720", DRM_MODE_TYPE_DRIVER, 74250, 1280, 3040,
		   3080, 3300, 0, 720, 725, 730, 750, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 30, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 63 - 1920x1080@120Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2008,
		   2052, 2200, 0, 1080, 1084, 1089, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	 .vrefresh = 120, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
	/* 64 - 1920x1080@100Hz */
	{ DRM_MODE("1920x1080", DRM_MODE_TYPE_DRIVER, 297000, 1920, 2448,
		   2492, 2640, 0, 1080, 1084, 1094, 1125, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	 .vrefresh = 100, .picture_aspect_ratio = HDMI_PICTURE_ASPECT_16_9, },
};

/*
 * HDMI 1.4 4k modes. Index using the VIC.
 */
static const struct drm_display_mode edid_4k_modes[] = {
	/* 0 - dummy, VICs start at 1 */
#ifndef __WATCOMC__
	{ },
#else
    { DRM_MODE("0", DRM_MODE_TYPE_DRIVER, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ), .vrefresh = 0, .picture_aspect_ratio = 0, },
#endif
	/* 1 - 3840x2160@30Hz */
	{ DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000,
		   3840, 4016, 4104, 4400, 0,
		   2160, 2168, 2178, 2250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 30, },
	/* 2 - 3840x2160@25Hz */
	{ DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000,
		   3840, 4896, 4984, 5280, 0,
		   2160, 2168, 2178, 2250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 25, },
	/* 3 - 3840x2160@24Hz */
	{ DRM_MODE("3840x2160", DRM_MODE_TYPE_DRIVER, 297000,
		   3840, 5116, 5204, 5500, 0,
		   2160, 2168, 2178, 2250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 24, },
	/* 4 - 4096x2160@24Hz (SMPTE) */
	{ DRM_MODE("4096x2160", DRM_MODE_TYPE_DRIVER, 297000,
		   4096, 5116, 5204, 5500, 0,
		   2160, 2168, 2178, 2250, 0,
		   DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_PVSYNC),
	  .vrefresh = 24, },
};


int i2c_transfer(struct i2c_adapter *i2c, struct i2c_msg *msgs, int num)
{
	if ( !i2c->xfer )
		return -ENOSYS;

	return i2c->xfer(i2c, msgs, num);
}

static unsigned int drm_debug = DRM_UT_KMS;

/*** DDC fetch and block validation ***/

static const u8 edid_header[] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

/**
 * drm_edid_header_is_valid - sanity check the header of the base EDID block
 * @raw_edid: pointer to raw base EDID block
 *
 * Sanity check the header of the base EDID block.
 *
 * Return: 8 if the header is perfect, down to 0 if it's totally wrong.
 */
int drm_edid_header_is_valid(const u8 *raw_edid)
{
	int i, score = 0;

	for (i = 0; i < sizeof(edid_header); i++)
		if (raw_edid[i] == edid_header[i])
			score++;

	return score;
}

static int edid_fixup = 6;

static void drm_get_displayid(struct drm_connector *connector, struct edid *edid);

static int drm_edid_block_checksum(const u8 *raw_edid)
{
	int i;
	u8 csum = 0;
	for (i = 0; i < EDID_LENGTH; i++)
		csum += raw_edid[i];

	return csum;
}

static bool drm_edid_is_zero(const u8 *in_edid, int length)
{
	int i;
	
	for ( i = 0; i < length; i ++ )
	{
		if ( in_edid[i] )
			return false;
	}
	
	return true;
}

/**
 * drm_edid_block_valid - Sanity check the EDID block (base or extension)
 * @raw_edid: pointer to raw EDID block
 * @block: type of block to validate (0 for base, extension otherwise)
 * @print_bad_edid: if true, dump bad EDID blocks to the console
 * @edid_corrupt: if true, the header or checksum is invalid
 *
 * Validate a base or extension EDID block and optionally dump bad blocks to
 * the console.
 *
 * Return: True if the block is valid, false otherwise.
 */
bool drm_edid_block_valid(u8 *raw_edid, int block, bool print_bad_edid,
			  bool *edid_corrupt)
{
	u8 csum;
	struct edid *edid = (struct edid *)raw_edid;

	if (WARN_ON(!raw_edid))
		return false;

	if (edid_fixup > 8 || edid_fixup < 0)
		edid_fixup = 6;

	if (block == 0) {
		int score = drm_edid_header_is_valid(raw_edid);
		if (score == 8) {
			if (edid_corrupt)
				*edid_corrupt = false;
		} else if (score >= edid_fixup) {
			/* Displayport Link CTS Core 1.2 rev1.1 test 4.2.2.6
			 * The corrupt flag needs to be set here otherwise, the
			 * fix-up code here will correct the problem, the
			 * checksum is correct and the test fails
			 */
			if (edid_corrupt)
				*edid_corrupt = true;
			DRM_DEBUG("Fixing EDID header, your hardware may be failing\n");
			memcpy(raw_edid, edid_header, sizeof(edid_header));
		} else {
			if (edid_corrupt)
				*edid_corrupt = true;
			goto bad;
		}
	}

	csum = drm_edid_block_checksum(raw_edid);
	if (csum) {
		if (print_bad_edid) {
			DRM_ERROR("EDID checksum is invalid, remainder is %d\n", csum);
		}

		if (edid_corrupt)
			*edid_corrupt = true;

		/* allow CEA to slide through, switches mangle this */
		if (raw_edid[0] != 0x02)
			goto bad;
	}

	/* per-block-type checks */
	switch (raw_edid[0]) {
	case 0: /* base */
		if (edid->version != 1) {
			DRM_ERROR("EDID has major version %d, instead of 1\n", edid->version);
			goto bad;
		}

		if (edid->revision > 4)
			DRM_DEBUG("EDID minor > 4, assuming backward compatibility\n");
		break;

	default:
		break;
	}

	return true;

bad:
	if (print_bad_edid) {
		if (drm_edid_is_zero(raw_edid, EDID_LENGTH)) {
			printk(KERN_ERR "EDID block is all zeroes\n");
		} else {
			printk(KERN_ERR "Raw EDID:\n");
			/*print_hex_dump(KERN_ERR, " \t", DUMP_PREFIX_NONE, 16, 1,
			       raw_edid, EDID_LENGTH, false);*/
		}
	}
	return false;
}

/**
 * drm_edid_is_valid - sanity check EDID data
 * @edid: EDID data
 *
 * Sanity-check an entire EDID record (including extensions)
 *
 * Return: True if the EDID data is valid, false otherwise.
 */
bool drm_edid_is_valid(struct edid *edid)
{
	int i;
	u8 *raw = (u8 *)edid;

	if (!edid)
		return false;

	for (i = 0; i <= edid->extensions; i++)
		if (!drm_edid_block_valid(raw + i * EDID_LENGTH, i, true, NULL))
			return false;

	return true;
}

#define DDC_SEGMENT_ADDR 0x30

/**
 * drm_do_probe_ddc_edid() - get EDID information via I2C
 * @data: I2C device adapter
 * @buf: EDID data buffer to be filled
 * @block: 128 byte EDID block to start fetching from
 * @len: EDID data buffer length to fetch
 *
 * Try to fetch EDID information by calling I2C driver functions.
 *
 * Return: 0 on success or -1 on failure.
 */
static int
drm_do_probe_ddc_edid(void *data, u8 *buf, unsigned int block, size_t len)
{
	struct i2c_adapter *adapter = data;
	unsigned char start = block * EDID_LENGTH;
	unsigned char segment = block >> 1;
	unsigned char xfers = segment ? 3 : 2;
	int ret, retries = 5;

	/*
	 * The core I2C driver will automatically retry the transfer if the
	 * adapter reports EAGAIN. However, we find that bit-banging transfers
	 * are susceptible to errors under a heavily loaded machine and
	 * generate spurious NAKs and timeouts. Retrying the transfer
	 * of the individual block a few times seems to overcome this.
	 */
	do {
#ifdef __QNXNTO__
		struct i2c_msg msgs[] = {
			{
				.addr   = DDC_SEGMENT_ADDR,
				.flags  = 0,
				.len    = 1,
				.buf    = &segment,
			}, {
				.addr   = DDC_ADDR,
				.flags  = 0,
				.len    = 1,
				.buf    = &start,
			}, {
				.addr   = DDC_ADDR,
				.flags  = I2C_M_RD,
				.len    = len,
				.buf    = buf,
			}
		};
#else
		struct i2c_msg msgs[3];

		msgs[0].addr   = DDC_SEGMENT_ADDR;
		msgs[0].flags  = 0;
		msgs[0].len    = 1;
		msgs[0].buf    = &segment;

		msgs[1].addr   = DDC_ADDR;
		msgs[1].flags  = 0;
		msgs[1].len    = 1;
		msgs[1].buf    = &start;

		msgs[2].addr   = DDC_ADDR;
		msgs[2].flags  = I2C_M_RD;
		msgs[2].len    = len;
		msgs[2].buf    = buf;
#endif

		/*
		 * Avoid sending the segment addr to not upset non-compliant
		 * DDC monitors.
		 */
		ret = i2c_transfer(adapter, &msgs[3 - xfers], xfers);

		if (ret == -ENXIO) {
			//DRM_DEBUG_KMS("drm: skipping non-existent adapter\n");
			break;
		}
		
	} while (ret != xfers && --retries);
	
	return ret == xfers ? 0 : -1;
}

/**
 * drm_do_get_edid - get EDID data using a custom EDID block read function
 * @connector: connector we're probing
 * @get_edid_block: EDID block read function
 * @data: private data passed to the block read function
 *
 * When the I2C adapter connected to the DDC bus is hidden behind a device that
 * exposes a different interface to read EDID blocks this function can be used
 * to get EDID data using a custom block read function.
 *
 * As in the general case the DDC bus is accessible by the kernel at the I2C
 * level, drivers must make all reasonable efforts to expose it as an I2C
 * adapter and use drm_get_edid() instead of abusing this function.
 *
 * Return: Pointer to valid EDID or NULL if we couldn't find any.
 */
struct edid *drm_do_get_edid(struct drm_connector *connector,
	int (*get_edid_block)(void *data, u8 *buf, unsigned int block,
			      size_t len),
	void *data)
{
	int i, j = 0, valid_extensions = 0;
	u8 *block, *new;
	bool print_bad_edid = !connector->bad_edid_counter || (drm_debug & DRM_UT_KMS);

	if ((block = malloc(EDID_LENGTH)) == NULL)
		return NULL;

	/* base block fetch */
	for (i = 0; i < 4; i++) {
		if (get_edid_block(data, block, 0, EDID_LENGTH))
			goto out;
		
		if (drm_edid_block_valid(block, 0, print_bad_edid,
					 &connector->edid_corrupt))
			break;
		if (i == 0 && drm_edid_is_zero(block, EDID_LENGTH)) {
			connector->null_edid_counter++;
			goto carp;
		}
	}
	if (i == 4)
		goto carp;

	/* if there's no extensions, we're done */
	if (block[0x7e] == 0)
		return (struct edid *)block;

	new = realloc(block, (block[0x7e] + 1) * EDID_LENGTH);
	if (!new)
		goto out;
	block = new;
	for (j = 1; j <= block[0x7e]; j++) {
		for (i = 0; i < 4; i++) {
			if (get_edid_block(data,
				  block + (valid_extensions + 1) * EDID_LENGTH,
				  j, EDID_LENGTH))
				goto out;
			if (drm_edid_block_valid(block + (valid_extensions + 1)
						 * EDID_LENGTH, j,
						 print_bad_edid,
						 NULL)) {
				valid_extensions++;
				break;
			}
		}

		if (i == 4 && print_bad_edid) {
#ifndef __WATCOMC__
			dev_warn(connector->dev->dev,
			 "%s: Ignoring invalid EDID block %d.\n",
			 connector->name, j);
#endif

			connector->bad_edid_counter++;
		}
	}

	if (valid_extensions != block[0x7e]) {
		block[EDID_LENGTH-1] += block[0x7e] - valid_extensions;
		block[0x7e] = valid_extensions;
		new = realloc(block, (valid_extensions + 1) * EDID_LENGTH);
		if (!new)
			goto out;
		block = new;
	}

	return (struct edid *)block;

carp:
	if (print_bad_edid) {
#ifndef __WATCOMC__
		dev_warn(connector->dev->dev, "%s: EDID block %d invalid.\n",
			 connector->name, j);
#endif
	}
	connector->bad_edid_counter++;

out:
	free(block);
	return NULL;
}

/**
 * drm_probe_ddc() - probe DDC presence
 * @adapter: I2C adapter to probe
 *
 * Return: True on success, false on failure.
 */
bool
drm_probe_ddc(struct i2c_adapter *adapter)
{
	unsigned char out;
	
	return (drm_do_probe_ddc_edid(adapter, &out, 0, 1) == 0);
}

struct edid *drm_get_edid(struct drm_connector *connector,
			  struct i2c_adapter *adapter)
{
	struct edid *edid;
	
	if (!drm_probe_ddc(adapter))
		return NULL;

	edid = drm_do_get_edid(connector, drm_do_probe_ddc_edid, adapter);
	if (edid)
		drm_get_displayid(connector, edid);
	return edid;
}

/**
 * drm_edid_duplicate - duplicate an EDID and the extensions
 * @edid: EDID to duplicate
 *
 * Return: Pointer to duplicated EDID or NULL on allocation failure.
 */
struct edid *drm_edid_duplicate(const struct edid *edid)
{
#ifndef __QNX__
        return kmemdup(edid, (edid->extensions + 1) * EDID_LENGTH, GFP_KERNEL);
#else
        void        *ptr = malloc( (edid->extensions + 1) * EDID_LENGTH );

        if ( ptr )
            memcpy( ptr, edid, (edid->extensions + 1) * EDID_LENGTH );

        return ptr;
#endif
}

struct edid *drm_get_edid_size( struct drm_connector *connector, struct i2c_adapter *adapter, int size )
{
	struct edid *edid;
	int r;

	edid = malloc( size );
	if ( edid == NULL )
		return NULL;

	r = drm_do_probe_ddc_edid( adapter, (void *)edid, 0, size );
	if ( r )
		goto out;

	return edid;

out:
	free( edid );

	return NULL;
}


#define AUDIO_BLOCK	0x01
#define VIDEO_BLOCK     0x02
#define VENDOR_BLOCK    0x03
#define SPEAKER_BLOCK	0x04
#define VIDEO_CAPABILITY_BLOCK	0x07
#define EDID_BASIC_AUDIO	(1 << 6)
#define EDID_CEA_YCRCB444	(1 << 5)
#define EDID_CEA_YCRCB422	(1 << 4)
#define EDID_CEA_VCDB_QS	(1 << 6)

/*
 * Search EDID for CEA extension block.
 */
static u8 *drm_find_edid_extension(struct edid *edid, int ext_id)
{
	u8 *edid_ext = NULL;
	int i;

	/* No EDID or EDID extensions */
	if (edid == NULL || edid->extensions == 0)
		return NULL;

	/* Find CEA extension */
	for (i = 0; i < edid->extensions; i++) {
		edid_ext = (u8 *)edid + EDID_LENGTH * (i + 1);
		if (edid_ext[0] == ext_id)
			break;
	}

	if (i == edid->extensions)
		return NULL;

	return edid_ext;
}

static u8 *drm_find_cea_extension(struct edid *edid)
{
	return drm_find_edid_extension(edid, CEA_EXT);
}

static u8 *drm_find_displayid_extension(struct edid *edid)
{
	return drm_find_edid_extension(edid, DISPLAYID_EXT);
}

static int
cea_db_payload_len(const u8 *db)
{
	return db[0] & 0x1f;
}

static int
cea_db_tag(const u8 *db)
{
	return db[0] >> 5;
}

/*static int
cea_revision(const u8 *cea)
{
	return cea[1];
}*/

static int
cea_db_offsets(const u8 *cea, int *start, int *end)
{
	/* Data block offset in CEA extension block */
	*start = 4;
	*end = cea[2];
	if (*end == 0)
		*end = 127;
	if (*end < 4 || *end > 127)
		return -ERANGE;
	return 0;
}

#define HDMI_IEEE_OUI 0x000c03

static bool cea_db_is_hdmi_vsdb(const u8 *db)
{
	int hdmi_id;

	if (cea_db_tag(db) != VENDOR_BLOCK)
		return false;

	if (cea_db_payload_len(db) < 5)
		return false;

	hdmi_id = db[1] | (db[2] << 8) | (db[3] << 16);

	return hdmi_id == HDMI_IEEE_OUI;
}

#define for_each_cea_db(cea, i, start, end) \
	for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); (i) += cea_db_payload_len(&(cea)[(i)]) + 1)

/**
 * drm_detect_hdmi_monitor - detect whether monitor is HDMI
 * @edid: monitor EDID information
 *
 * Parse the CEA extension according to CEA-861-B.
 *
 * Return: True if the monitor is HDMI, false if not or unknown.
 */
bool drm_detect_hdmi_monitor(struct edid *edid)
{
	u8 *edid_ext;
	int i;
	int start_offset, end_offset;
	
	edid_ext = drm_find_cea_extension(edid);
	if (!edid_ext)
		return false;
		
	if (cea_db_offsets(edid_ext, &start_offset, &end_offset))
		return false;
	
	/*
	 * Because HDMI identifier is in Vendor Specific Block,
	 * search it from all data blocks of CEA extension.
	 */
	for_each_cea_db(edid_ext, i, start_offset, end_offset) {
		if (cea_db_is_hdmi_vsdb(&edid_ext[i]))
			return true;
	}
	
	return false;
}

static int drm_parse_display_id(struct drm_connector *connector,
				u8 *displayid, int length,
				bool is_edid_extension)
{
	/* if this is an EDID extension the first byte will be 0x70 */
	int idx = 0;
	struct displayid_hdr *base;
	struct displayid_block *block;
	u8 csum = 0;
	int i;

	if (is_edid_extension)
		idx = 1;

	base = (struct displayid_hdr *)&displayid[idx];

	DRM_DEBUG_KMS("base revision 0x%x, length %d, %d %d\n",
		      base->rev, base->bytes, base->prod_id, base->ext_count);

	if (base->bytes + 5 > length - idx)
		return -EINVAL;

	for (i = idx; i <= base->bytes + 5; i++) {
		csum += displayid[i];
	}
	if (csum) {
		DRM_ERROR("DisplayID checksum invalid, remainder is %d\n", csum);
		return -EINVAL;
	}

	block = (struct displayid_block *)&displayid[idx + 4];
	DRM_DEBUG_KMS("block id %d, rev %d, len %d\n",
		      block->tag, block->rev, block->num_bytes);

	switch (block->tag) {
	case DATA_BLOCK_TILED_DISPLAY: {
		struct displayid_tiled_block *tile = (struct displayid_tiled_block *)block;

		u16 w, h;
		u8 tile_v_loc, tile_h_loc;
		u8 num_v_tile, num_h_tile;
		/* struct drm_tile_group *tg; */

		w = tile->tile_size[0] | tile->tile_size[1] << 8;
		h = tile->tile_size[2] | tile->tile_size[3] << 8;

		num_v_tile = (tile->topo[0] & 0xf) | (tile->topo[2] & 0x30);
		num_h_tile = (tile->topo[0] >> 4) | ((tile->topo[2] >> 2) & 0x30);
		tile_v_loc = (tile->topo[1] & 0xf) | ((tile->topo[2] & 0x3) << 4);
		tile_h_loc = (tile->topo[1] >> 4) | (((tile->topo[2] >> 2) & 0x3) << 4);

		connector->has_tile = true;
		if (tile->tile_cap & 0x80)
			connector->tile_is_single_monitor = true;

		connector->num_h_tile = num_h_tile + 1;
		connector->num_v_tile = num_v_tile + 1;
		connector->tile_h_loc = tile_h_loc;
		connector->tile_v_loc = tile_v_loc;
		connector->tile_h_size = w + 1;
		connector->tile_v_size = h + 1;

		DRM_DEBUG_KMS("tile cap 0x%x\n", tile->tile_cap);
		DRM_DEBUG_KMS("tile_size %d x %d\n", w + 1, h + 1);
		DRM_DEBUG_KMS("topo num tiles %dx%d, location %dx%d\n",
		       num_h_tile + 1, num_v_tile + 1, tile_h_loc, tile_v_loc);
		DRM_DEBUG_KMS("vend %c%c%c\n", tile->topology_id[0], tile->topology_id[1], tile->topology_id[2]);
#if 0
		tg = drm_mode_get_tile_group(connector->dev, tile->topology_id);
		if (!tg) {
			tg = drm_mode_create_tile_group(connector->dev, tile->topology_id);
		}
		if (!tg)
			return -ENOMEM;

		if (connector->tile_group != tg) {
			/* if we haven't got a pointer,
			   take the reference, drop ref to old tile group */
			if (connector->tile_group) {
				drm_mode_put_tile_group(connector->dev, connector->tile_group);
			}
			connector->tile_group = tg;
		} else
			/* if same tile group, then release the ref we just took. */
			drm_mode_put_tile_group(connector->dev, tg);
#endif
	}
		break;
	default:
		printk("unknown displayid tag %d\n", block->tag);
		break;
	}
	return 0;
}

static void drm_get_displayid(struct drm_connector *connector,
			      struct edid *edid)
{
	void *displayid = NULL;
	int ret;
	connector->has_tile = false;
	displayid = drm_find_displayid_extension(edid);
	if (!displayid) {
		/* drop reference to any tile group we had */
		goto out_drop_ref;
	}

	ret = drm_parse_display_id(connector, displayid, EDID_LENGTH, true);
	if (ret < 0)
		goto out_drop_ref;
	if (!connector->has_tile)
		goto out_drop_ref;
	return;
out_drop_ref:
#if 0
	if (connector->tile_group) {
		drm_mode_put_tile_group(connector->dev, connector->tile_group);
		connector->tile_group = NULL;
	}
#endif
	return;
}


/*
 * Calculate the alternate clock for the CEA mode
 * (60Hz vs. 59.94Hz etc.)
 */
static unsigned int
cea_mode_alternate_clock(const struct drm_display_mode *cea_mode)
{
	unsigned int clock = cea_mode->clock;

	if (cea_mode->vrefresh % 6 != 0)
		return clock;

	/*
	 * edid_cea_modes contains the 59.94Hz
	 * variant for 240 and 480 line modes,
	 * and the 60Hz variant otherwise.
	 */
	if (cea_mode->vdisplay == 240 || cea_mode->vdisplay == 480)
		clock = DIV_ROUND_CLOSEST(clock * 1001, 1000);
	else
		clock = DIV_ROUND_CLOSEST(clock * 1000, 1001);

	return clock;
}


/**
 * drm_get_cea_aspect_ratio - get the picture aspect ratio corresponding to
 * the input VIC from the CEA mode list
 * @video_code: ID given to each of the CEA modes
 *
 * Returns picture aspect ratio
 */
enum hdmi_picture_aspect drm_get_cea_aspect_ratio(const u8 video_code)
{
	return edid_cea_modes[video_code].picture_aspect_ratio;
}


/*
 * Calculate the alternate clock for HDMI modes (those from the HDMI vendor
 * specific block).
 *
 * It's almost like cea_mode_alternate_clock(), we just need to add an
 * exception for the VIC 4 mode (4096x2160@24Hz): no alternate clock for this
 * one.
 */
static unsigned int
hdmi_mode_alternate_clock(const struct drm_display_mode *hdmi_mode)
{
	if (hdmi_mode->vdisplay == 4096 && hdmi_mode->hdisplay == 2160)
		return hdmi_mode->clock;

	return cea_mode_alternate_clock(hdmi_mode);
}


/*
 * drm_match_hdmi_mode - look for a HDMI mode matching given mode
 * @to_match: display mode
 *
 * An HDMI mode is one defined in the HDMI vendor specific block.
 *
 * Returns the HDMI Video ID (VIC) of the mode or 0 if it isn't one.
 */
static u8 drm_match_hdmi_mode(const struct drm_display_mode *to_match)
{
	u8 vic;

	if (!to_match->clock)
		return 0;

	for (vic = 1; vic < ARRAY_SIZE(edid_4k_modes); vic++) {
		const struct drm_display_mode *hdmi_mode = &edid_4k_modes[vic];
		unsigned int clock1, clock2;

		/* Make sure to also match alternate clocks */
		clock1 = hdmi_mode->clock;
		clock2 = hdmi_mode_alternate_clock(hdmi_mode);

		if ((KHZ2PICOS(to_match->clock) == KHZ2PICOS(clock1) ||
		     KHZ2PICOS(to_match->clock) == KHZ2PICOS(clock2)) &&
		    drm_mode_equal_no_clocks_no_stereo(to_match, hdmi_mode))
			return vic;
	}
	return 0;
}


/**
 * drm_match_cea_mode - look for a CEA mode matching given mode
 * @to_match: display mode
 *
 * Return: The CEA Video ID (VIC) of the mode or 0 if it isn't a CEA-861
 * mode.
 */
u8 drm_match_cea_mode(const struct drm_display_mode *to_match)
{
	u8 vic;

	if (!to_match->clock)
		return 0;

	for (vic = 1; vic < ARRAY_SIZE(edid_cea_modes); vic++) {
		const struct drm_display_mode *cea_mode = &edid_cea_modes[vic];
		unsigned int clock1, clock2;

		/* Check both 60Hz and 59.94Hz */
		clock1 = cea_mode->clock;
		clock2 = cea_mode_alternate_clock(cea_mode);

		if ((KHZ2PICOS(to_match->clock) == KHZ2PICOS(clock1) ||
		     KHZ2PICOS(to_match->clock) == KHZ2PICOS(clock2)) &&
		    drm_mode_equal_no_clocks_no_stereo(to_match, cea_mode))
			return vic;
	}
	return 0;
}


static enum hdmi_3d_structure
s3d_structure_from_display_mode(const struct drm_display_mode *mode)
{
	u32 layout = mode->flags & DRM_MODE_FLAG_3D_MASK;

	switch (layout) {
	case DRM_MODE_FLAG_3D_FRAME_PACKING:
		return HDMI_3D_STRUCTURE_FRAME_PACKING;
	case DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE:
		return HDMI_3D_STRUCTURE_FIELD_ALTERNATIVE;
	case DRM_MODE_FLAG_3D_LINE_ALTERNATIVE:
		return HDMI_3D_STRUCTURE_LINE_ALTERNATIVE;
	case DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL:
		return HDMI_3D_STRUCTURE_SIDE_BY_SIDE_FULL;
	case DRM_MODE_FLAG_3D_L_DEPTH:
		return HDMI_3D_STRUCTURE_L_DEPTH;
	case DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH:
		return HDMI_3D_STRUCTURE_L_DEPTH_GFX_GFX_DEPTH;
	case DRM_MODE_FLAG_3D_TOP_AND_BOTTOM:
		return HDMI_3D_STRUCTURE_TOP_AND_BOTTOM;
	case DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF:
		return HDMI_3D_STRUCTURE_SIDE_BY_SIDE_HALF;
	default:
		return HDMI_3D_STRUCTURE_INVALID;
	}
}


/**
* drm_hdmi_avi_infoframe_from_display_mode() - fill an HDMI AVI infoframe with
*                                              data from a DRM display mode
* @frame: HDMI AVI infoframe
* @mode: DRM display mode
*
* Return: 0 on success or a negative error code on failure.
*/
int
drm_hdmi_avi_infoframe_from_display_mode(struct hdmi_avi_infoframe *frame,
                                      const struct drm_display_mode *mode)
{
    int err;

    if (!frame || !mode)
             return -EINVAL;

    err = hdmi_avi_infoframe_init(frame);
    if (err < 0)
        return err;

    if (mode->flags & DRM_MODE_FLAG_DBLCLK)
        frame->pixel_repeat = 1;

    frame->video_code = drm_match_cea_mode(mode);

    frame->picture_aspect = HDMI_PICTURE_ASPECT_NONE;

    /*
     * Populate picture aspect ratio from either
     * user input (if specified) or from the CEA mode list.
     */
    if (mode->picture_aspect_ratio == HDMI_PICTURE_ASPECT_4_3 ||
            mode->picture_aspect_ratio == HDMI_PICTURE_ASPECT_16_9)
            frame->picture_aspect = mode->picture_aspect_ratio;
    else if (frame->video_code > 0)
            frame->picture_aspect = drm_get_cea_aspect_ratio(
                                            frame->video_code);

    frame->active_aspect = HDMI_ACTIVE_ASPECT_PICTURE;
    frame->scan_mode = HDMI_SCAN_MODE_UNDERSCAN;

    return 0;
}

/**
 * drm_hdmi_vendor_infoframe_from_display_mode() - fill an HDMI infoframe with
 * data from a DRM display mode
 * @frame: HDMI vendor infoframe
 * @mode: DRM display mode
 *
 * Note that there's is a need to send HDMI vendor infoframes only when using a
 * 4k or stereoscopic 3D mode. So when giving any other mode as input this
 * function will return -EINVAL, error that can be safely ignored.
 *
 * Return: 0 on success or a negative error code on failure.
 */
int
drm_hdmi_vendor_infoframe_from_display_mode(struct hdmi_vendor_infoframe *frame,
					    const struct drm_display_mode *mode)
{
	int err;
	u32 s3d_flags;
	u8 vic;

	if (!frame || !mode)
		return -EINVAL;

	vic = drm_match_hdmi_mode(mode);
	s3d_flags = mode->flags & DRM_MODE_FLAG_3D_MASK;

	if (!vic && !s3d_flags)
		return -EINVAL;

	if (vic && s3d_flags)
		return -EINVAL;

	err = hdmi_vendor_infoframe_init(frame);
	if (err < 0)
		return err;

	if (vic)
		frame->vic = vic;
	else
		frame->s3d_struct = s3d_structure_from_display_mode(mode);

	return 0;
}
