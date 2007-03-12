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

static const double width = 900;
static const double height = 600;

static inline unsigned mandel_it(
		long double *xc, long double *yc,
		long double *xl, long double *yl)
{
	static const unsigned maxit = 500;
	unsigned it = 0;

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

	while ((x2 + y2 < 4) && it++ < maxit) {
		y = 2 * x * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
	}

	if (xl)
		*xl = x;
	if (yl)
		*yl = y;

	if (it < maxit)
		return it;
	else
		return 0;
}

void draw_mandel(GdkPixmap *pixmap, double ulx, double uly, double lly)
{
	GdkGC *black = gdk_gc_new(pixmap);

	GdkColor other_color;
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
			long double lx;
			long double ly;
			unsigned color = mandel_it(&x, &y, &lx, &ly);
			if (color == 0)
				gdk_draw_point(pixmap, black, i, j);
			else {
				double mu = color + 1
					- log(log(sqrt(lx * lx + ly * ly)))/log(2);
				mu *= 50;
				other_color.blue = mu * color_ratio.blue;
				other_color.green = mu * color_ratio.green;
				other_color.red = mu * color_ratio.red;
				gdk_gc_set_rgb_fg_color(other_gc, &other_color);
				gdk_draw_point(pixmap, other_gc, i, j);
			}
			y -= inc;
		}
		x += inc;
	}
	printf("x = %LF | y = %LF\n", x, y);

	fprintf(stderr, "Finished computing the mandelbrot set!\n");
}

gboolean handle_expose(
		GtkWidget *widget,
		GdkEventExpose *event,
		gpointer data)
{
	static GdkPixmap *pixmap = NULL;
	static GdkGC *gc = NULL;

	if (!pixmap) {
		pixmap = gdk_pixmap_new(widget->window, width, height, -1);

		/* Painting algorithm */
		double ulx = -2.1;
		double uly = 1.1;
		double lly = -1.1;
#if 0
		/* Biggest replica */
		ulx = -1.81;
		uly = 0.025;
		lly = -uly;
#endif
#if 0
		/* Seahorse valley */
		ulx = -1.38;
		uly = 0.015;
		lly = 0.008;
#endif
#if 0
		/* Rotated small replica */
		ulx = -0.179;
		uly = 1.046;
		lly = 1.024;
#endif
#if 0
		/* Really small replica */
		ulx = -1.744484;
		uly = -0.022004;
		lly = -0.022044;
#endif
		draw_mandel(pixmap, ulx, uly, lly);
		gc = gdk_gc_new(pixmap);
	}

	gdk_draw_drawable(widget->window,
			gc,
			pixmap,
			0, 0, 0, 0, -1, -1);

	return FALSE;
}

gboolean handle_click(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data)
{
	fprintf(stderr, "Click! x = %g | y = %g\n",
			event->x, event->y);

	// -2+(3x0/900))-i+(2y0/600)i
	fprintf(stderr, "ulx = %g | uly = %g\n",
			-2.1 + (3 * event->x / width),
			1.1 - (2 * event->y / height));

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
