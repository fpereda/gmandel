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
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "paint.h"
#include "parallel_paint.h"
#include "mandelbrot.h"
#include "color.h"
#include "xfuncs.h"

static struct observer_state paint_limits = {
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

static struct {
	long double v;
	unsigned n;
	parallel_lockable;
} avgfactor = {
	.v = 0,
	.n = 0,
	parallel_lockable_init
};

static long double **mupoint = NULL;

static GdkPixmap *pixmap = NULL;

static inline double paint_inc(void)
{
	return (paint_limits.uly - paint_limits.lly) / (window_size.height - 1);
}

static void clean_mupoint_col(unsigned i)
{
	for (unsigned j = 0; j < window_size.height; j++)
		mupoint[i][j] = -1L;
}

static void clean_mupoint(void)
{
	for (unsigned i = 0; i < window_size.width; i++)
		clean_mupoint_col(i);
}

static void mupoint_move_up(void)
{
	size_t num = (window_size.height - 1) * sizeof(**mupoint);
	for (unsigned i = 0; i < window_size.width; i++) {
		memmove(&mupoint[i][1], &mupoint[i][0], num);
		mupoint[i][0] = -1L;
	}
}

static void mupoint_move_down(void)
{
	size_t num = (window_size.height - 1) * sizeof(**mupoint);
	for (unsigned i = 0; i < window_size.width; i++) {
		memmove(&mupoint[i][0], &mupoint[i][1], num);
		mupoint[i][window_size.height - 1] = -1L;
	}
}

static void mupoint_move_right(void)
{
	unsigned width = window_size.width;
	size_t num = (width - 1) * sizeof(*mupoint);

	void *p = mupoint[0];
	memmove(&mupoint[0], &mupoint[1], num);
	mupoint[width - 1] = p;
	clean_mupoint_col(width - 1);
}

static void mupoint_move_left(void)
{
	unsigned width = window_size.width;
	size_t num = (width - 1) * sizeof(*mupoint);

	void *p = mupoint[width - 1];
	memmove(&mupoint[1], &mupoint[0], num);
	mupoint[0] = p;
	clean_mupoint_col(0);
}

void paint_move_up(unsigned n)
{
	paint_limits.uly += n * paint_inc();
	paint_limits.lly += n * paint_inc();
	while (n--)
		mupoint_move_up();
}

void paint_move_down(unsigned n)
{
	paint_limits.uly -= n * paint_inc();
	paint_limits.lly -= n * paint_inc();
	while (n--)
		mupoint_move_down();
}

void paint_move_right(unsigned n)
{
	paint_limits.ulx += n * paint_inc();
	while (n--)
		mupoint_move_right();
}

void paint_move_left(unsigned n)
{
	paint_limits.ulx -= n *paint_inc();
	while (n--)
		mupoint_move_left();
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

void paint_set_observer_state(struct observer_state *s)
{
	memcpy(&paint_limits, s, sizeof(*s));
}

void paint_get_observer_state(struct observer_state *s)
{
	memcpy(s, &paint_limits, sizeof(*s));
}

void paint_force_redraw(GtkWidget *widget, bool clean)
{
	pixmap = NULL;
	if (clean)
		clean_mupoint();
	GdkRectangle area = {
		.x = 0,
		.y = 0,
		.width = -1,
		.height = -1,
	};
	paint_mandel(widget, area, !clean);
}

static void recalculate_average_energy(void)
{
	avgfactor.v = 0L;
	avgfactor.n = 0;
	for (unsigned i = 0; i < window_size.width; i++)
		for (unsigned j = 0; j < window_size.height; j++) {
			if (mupoint[i][j] == 0)
				continue;
			avgfactor.v += mupoint[i][j];
			avgfactor.n++;
		}
}

static long double do_energyfactor(void)
{
	if (avgfactor.n > 0)
		avgfactor.v /= avgfactor.n;
	else
		avgfactor.v = 1;

	long double squarefactor = sqrtl(
			log10l(avgfactor.v * sqrtl(expl(avgfactor.v))));
	long double ret = 1000 / squarefactor;

	/*
	 * avgfactor has to be reset here. otherwise subsequent calls to
	 * do_avgfactor will reuse the previously computed energy average values.
	 * There is no need to synchronize here because all threads have already
	 * been joined.
	 */
	avgfactor.v = 0L;
	avgfactor.n = 0;

	return ret;
}

static void plot_points(void)
{
	static GdkGC *gc = NULL;

	if (gc == NULL)
		gc = gdk_gc_new(pixmap);

	long double energyfactor = do_energyfactor();
	for (unsigned i = 0; i < window_size.width; i++) {
		for (unsigned j = 0; j < window_size.height; j++) {
			long double factor = mupoint[i][j] * energyfactor;
			guint32 red = color_ratio.red * factor;
			guint32 blue = color_ratio.blue * factor;
			guint32 green = color_ratio.green * factor;

			static const guint16 cmax = ~0;

			red = red > cmax ? cmax : red;
			blue = blue > cmax ? cmax : blue;
			green = green > cmax ? cmax : green;

			GdkColor color = {
				.red = red,
				.blue = blue,
				.green = green,
			};

			gdk_gc_set_rgb_fg_color(gc, &color);
			gdk_draw_point(pixmap, gc, i, j);
		}
	}
}

void paint_do_mu(unsigned begin, size_t n)
{
	long double x;
	long double y;
	long double acc = 0;
	unsigned nacc = 0;

	x = paint_limits.ulx + begin * paint_inc();
	for (unsigned i = 0; i < n; i++) {
		y = paint_limits.uly;
		for (unsigned j = 0; j < window_size.height; j++) {
			if (mupoint[i + begin][j] != -1L)
				goto inc_and_cont;

			long double modulus;
			unsigned it = mandelbrot_it(&x, &y, &modulus);

			/*
			 * Renormalized formula for the escape radius.
			 * Optimize away the case where it == 0
			 */
			if (it > 0) {
				long double mu = it - logl(fabsl(logl(modulus)));
				mu /= M_LN2;
				mupoint[i + begin][j] = mu;
				acc += mu;
				nacc++;
			} else
				mupoint[i + begin][j] = 0L;
inc_and_cont:
			y -= paint_inc();
		}
		x += paint_inc();
	}

	parallel_lock(&avgfactor);
	avgfactor.v += acc;
	avgfactor.n += nacc;
	parallel_unlock(&avgfactor);
}

static void paint_do_mandel(bool redoenergy)
{
	parallel_paint_do_mu(window_size.width);
	if (redoenergy)
		recalculate_average_energy();
	plot_points();
}

void paint_mandel(GtkWidget *widget, GdkRectangle area, bool redoenergy)
{
	static GdkGC *gc = NULL;

	if (!mupoint) {
		mupoint = xmalloc(window_size.width * sizeof(*mupoint));
		for (unsigned i = 0; i < window_size.width; i++)
			mupoint[i] = xmalloc(window_size.height * sizeof(**mupoint));
		clean_mupoint();
	}

	if (!pixmap) {
		pixmap = gdk_pixmap_new(widget->window,
				window_size.width, window_size.height, -1);
		paint_do_mandel(redoenergy);
		gc = gdk_gc_new(pixmap);
	}

	gdk_draw_drawable(widget->window,
			gc,
			pixmap,
			area.x, area.y,
			area.x, area.y,
			area.width, area.height);
}
