/* vim: set sts=4 sw=4 noet : */

/*
 * Copyright (c) 2007, Fernando J. Pereda <ferdy@gentoo.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the program nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "paint.h"
#include "mandelbrot.h"
#include "color.h"

static struct {
	double ulx;
	double uly;
	double lly;
} paint_limits = {
	.ulx = -2.1,
	.uly = 1.1,
	.lly = -1.1,
};

static struct {
	unsigned width;
	unsigned height;
} window_size = {
	.width = 900,
	.height = 600,
};

static long double **mupoint = NULL;

static GdkPixmap *pixmap = NULL;

static void clean_mupoint(void)
{
	for (unsigned i = 0; i < window_size.width; i++)
		for (unsigned j = 0; j < window_size.height; j++)
			mupoint[i][j] = -1;
}

void paint_set_window_size(unsigned width, unsigned height)
{
	window_size.width = width;
	window_size.height = height;
}

void paint_get_window_size(unsigned *width, unsigned *height)
{
	if (width)
		*width = window_size.width;
	if (height)
		*height = window_size.height;
}

void paint_set_limits(double ulx, double uly, double lly)
{
	paint_limits.ulx = ulx;
	paint_limits.uly = uly;
	paint_limits.lly = lly;
}

void paint_get_limits(double *ulx, double *uly, double *lly)
{
	if (ulx)
		*ulx = paint_limits.ulx;
	if (uly)
		*uly = paint_limits.uly;
	if (lly)
		*lly = paint_limits.lly;
}

void paint_force_redraw(GtkWidget *widget)
{
	pixmap = NULL;
	clean_mupoint();
	paint_mandel(widget);
}

static long double do_avgfactor(void)
{
	long double avg = 0;
	unsigned navg = 0;
	for (unsigned i = 0; i < window_size.width; i++)
		for (unsigned j = 0; j < window_size.height; j++) {
			if (mupoint[i][j] == 0)
				continue;
			avg += mupoint[i][j];
			navg++;
		}
	if (navg > 0)
		avg /= navg;
	else
		avg = 1;

	long double squarefactor = sqrtl(log10l(avg * sqrtl(expl(avg))));
	long double avgfactor = 1000 / squarefactor;

	return avgfactor;
}

static void plot_points(void)
{
	static GdkGC *gc = NULL;

	if (gc == NULL)
		gc = gdk_gc_new(pixmap);

	long double avgfactor = do_avgfactor();
	for (unsigned i = 0; i < window_size.width; i++) {
		for (unsigned j = 0; j < window_size.height; j++) {
			long double factor = mupoint[i][j] * avgfactor;
			guint32 red = color_ratio.red * factor;
			guint32 blue = color_ratio.blue * factor;
			guint32 green = color_ratio.green * factor;

			static const guint16 cmax = ~0;

			red = red > cmax ? cmax : red;
			blue = blue > cmax ? cmax : blue;
			green = green > cmax ? cmax : green;

			GdkColor color;
			color.red = red;
			color.blue = blue;
			color.green = green;

			gdk_gc_set_rgb_fg_color(gc, &color);
			gdk_draw_point(pixmap, gc, i, j);
		}
	}
}

static void paint_do_mandel(void)
{
	unsigned width = window_size.width;
	unsigned height = window_size.height;
	double inc = (paint_limits.uly - paint_limits.lly) / (height - 1);

	long double x;
	long double y;

	x = paint_limits.ulx;
	for (unsigned i = 0; i < width; i++) {
		y = paint_limits.uly;
		for (unsigned j = 0; j < height; j++) {
			if (mupoint[i][j] != -1)
				continue;

			long double modulus;
			unsigned it = mandelbrot_it(&x, &y, &modulus);

			/*
			 * Renormalized formula for the escape radius.
			 * Optimize away the case where it == 0
			 */
			if (it > 0) {
				long double mu = it - logl(fabsl(logl(modulus)));
				mu /= log(2.0);
				mupoint[i][j] = mu;
			} else
				mupoint[i][j] = 0L;

			y -= inc;
		}
		x += inc;
	}

	plot_points();
}

void paint_mandel(GtkWidget *widget)
{
	static GdkGC *gc = NULL;

	if (!mupoint) {
		mupoint = malloc(window_size.width * sizeof(*mupoint));
		for (unsigned i = 0; i < window_size.width; i++)
			mupoint[i] = malloc(window_size.height * sizeof(**mupoint));
		clean_mupoint();
	}

	if (!pixmap) {
		pixmap = gdk_pixmap_new(widget->window,
				window_size.width, window_size.height, -1);
		paint_do_mandel();
		gc = gdk_gc_new(pixmap);
	}

	gdk_draw_drawable(widget->window,
			gc,
			pixmap,
			0, 0, 0, 0, -1, -1);
}
