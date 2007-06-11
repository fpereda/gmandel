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
#include <stdio.h>
#include <math.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "color.h"

#define SIZEOF_ARRAY(a) (sizeof(a)/sizeof(a[0]))

static const double width = 900;
static const double height = 600;
static const unsigned maxit = 1000;

static double gulx = 0;
static double guly = 0;
static double glly = 0;

static GdkPixmap *gpixmap = NULL;

static unsigned depth = 1;

static unsigned mandel_it(
		long double *xc, long double *yc,
		long double *modulus)
{
	unsigned it = 1;

	long double x;
	long double y;
	long double x0;
	long double y0;
	long double x2;
	long double y2;

	x = x0 = *xc;
	y = y0 = *yc;

	x2 = x * x;
	y2 = y * y;

	while ((x2 + y2) < 4 && it++ < maxit) {
		y = 2 * x * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
	}

	/*
	 * When using the renormalized formula for the escape radius,
	 * a couple of additional iterations help reducing the size
	 * of the error term.
	 */
	unsigned n = 2;
	while (n--) {
		y = 2 * x * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
	}

	if (it >= maxit || it == 0)
		return 0;

	if (modulus)
		*modulus = sqrtl(x2 + y2);

	return it;
}

void draw_mandel(GdkPixmap *pixmap, double ulx, double uly, double lly)
{
	static long double **temp = NULL;

	if (temp == NULL) {
		temp = malloc(width * sizeof(*temp));
		unsigned i;
		for (i = 0; i < width; i++)
			temp[i] = malloc(height * sizeof(**temp));
	}

	GdkGC *other_gc = gdk_gc_new(pixmap);

	double inc = (uly - lly) / (height - 1);

	long double x;
	long double y;
	unsigned i;
	unsigned j;

	x = ulx;
	for (i = 0; i < width; i++) {
		y = uly;
		for (j = 0; j < height; j++) {
			long double modulus;
			unsigned it = mandel_it(&x, &y, &modulus);

			/*
			 * Renormalized formula for the escape radius.
			 * Optimize away the case where it == 0
			 */
			if (it > 0) {
				long double mu = it - logl(fabsl(logl(modulus)));
				mu /= log(2.0);
				temp[i][j] = mu;
			} else
				temp[i][j] = 0L;

			y -= inc;
		}
		x += inc;
	}

	long double avg = 0;
	unsigned navg = 0;
	for (i = 0; i < width; i++)
		for (j = 0; j < height; j++) {
			if (temp[i][j] == 0)
				continue;
			avg += temp[i][j];
			navg++;
		}
	if (navg > 0)
		avg /= navg;
	else
		avg = 1;

	long double avgfactor = 1000 / logl(avg);

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			long double factor = roundl(temp[i][j] * avgfactor);
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

			gdk_gc_set_rgb_fg_color(other_gc, &color);
			gdk_draw_point(pixmap, other_gc, i, j);
		}
	}
}

void repaint_mandel(GtkWidget *widget)
{
	static GdkGC *gc = NULL;

	if (!gpixmap) {
		gpixmap = gdk_pixmap_new(widget->window, width, height, -1);

		double *ulx = &gulx;
		double *uly = &guly;
		double *lly = &glly;

		if (*ulx == 0 && *uly == 0 && *lly == 0) {
			/* Painting algorithm */
			*ulx = -2.1;
			*uly = 1.1;
			*lly = -1.1;
#if 0
			/* Biggest replica */
			*ulx = -1.81;
			*uly = 0.025;
			*lly = -*uly;
#endif
#if 0
			/* Seahorse valley */
			*ulx = -1.38;
			*uly = 0.015;
			*lly = 0.008;
#endif
#if 0
			/* Rotated small replica */
			*ulx = -0.179;
			*uly = 1.046;
			*lly = 1.024;
#endif
#if 0
			/* Really small replica */
			*ulx = -1.744484;
			*uly = -0.022004;
			*lly = -0.022044;
#endif
		}

		draw_mandel(gpixmap, *ulx, *uly, *lly);
		gc = gdk_gc_new(gpixmap);
	}

	gdk_draw_drawable(widget->window,
			gc,
			gpixmap,
			0, 0, 0, 0, -1, -1);
}

gboolean handle_expose(
		GtkWidget *widget,
		GdkEventExpose *event,
		gpointer data)
{
	repaint_mandel(widget);
	return FALSE;
}

gboolean handle_click(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data)
{
	double inc_y = guly - glly;
	double inc = inc_y / (height - 1);
	double inc_x = width * inc;

	long double x = event->x * inc + gulx;
	long double y = -(event->y * inc - guly);

	fprintf(stderr, "x = %LF | y = %LF || inc_y = %g | inc_x = %g\n",
			x, y, inc_y, inc_x);

	if (event->button == 1) {
		inc_y /= 10;
		inc_x /= 10;

		guly = y + inc_y/2;
		glly = y - inc_y/2;
		gulx = x - inc_x/2;
	} else {
		/* TODO: Implement a stack of 'explorer states' */
		fprintf(stderr, "Come back later, thanks\n");
		return FALSE;
	}

	++depth;
	gpixmap = NULL;
	repaint_mandel(widget);

	return FALSE;
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),
			"Mandelbrot Set generator by Fernando J. Pereda");
	gtk_widget_set_size_request(window, width, height);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *drawing_area = gtk_drawing_area_new();
	g_signal_connect(drawing_area, "expose-event",
			G_CALLBACK(handle_expose), NULL);
	gtk_widget_add_events(drawing_area, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(drawing_area, "button-press-event",
			G_CALLBACK(handle_click), NULL);

	GtkWidget *lyout_top = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(lyout_top), drawing_area);
	gtk_container_add(GTK_CONTAINER(window), lyout_top);

	gtk_widget_show_all(window);
	gtk_main();

	return EXIT_SUCCESS;
}
