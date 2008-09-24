/* vim: set sts=4 sw=4 noet : */

/*
 * Copyright (c) 2008, Fernando J. Pereda <ferdy@ferdyx.org>
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

#include <gtk/gtk.h>

#include "gfract.h"
#include "color.h"

#include <sys/time.h>
#include <math.h>

struct point {
	double x;
	double y;
};

static inline struct point imaginary_cardioid_from_theta(double theta)
{
	double rho = 2 * 0.25 * (1 - cos(theta));

	struct point p = {
		.x = rho * cos(theta) + 0.25,
		.y = rho * sin(theta),
	};

	return p;
}

static inline void
do_image_for_point(GtkWidget *w, GtkWidget *f, unsigned i, struct point p)
{
	static char filename[100];

	gfract_set_center(f, p.x, p.y);
	printf("[ %4u ] cx = %G | cy = %G\n", i, p.x, p.y);

	gfract_compute(f);
	gtk_widget_show_all(w);

	snprintf(filename, sizeof(filename), "gjulia_video_%.4u.png", i);

	GdkPixbuf *buf = gfract_get_pixbuf(f);
	GError *err = NULL;
	if (!gdk_pixbuf_save(buf, filename, "png", &err, NULL)) {
		fprintf(stderr, "Could not save to file '%s': %s",
				filename, err->message);
		exit(EXIT_FAILURE);
	}
	g_object_unref(buf);
}

int main(int argc, char *argv[])
{
	unsigned width = 640;
	unsigned height = 480;
	unsigned maxit = 100;
	double ulx = -2.0;
	double uly = 1.5;
	double lly = -1.5;

	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Julia set video");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

	GtkWidget *f = gfract_new_julia(window, width, height);
	gfract_set_limits(f, ulx, uly, lly);
	gfract_set_maxit(f, maxit);
	gfract_set_ratios(f,
			color_get(COLOR_THEME_GREENPARK)->red,
			color_get(COLOR_THEME_GREENPARK)->blue,
			color_get(COLOR_THEME_GREENPARK)->green);
	gfract_set_do_progress(f, FALSE);

	gtk_container_add(GTK_CONTAINER(window), f);
	gtk_widget_show_all(window);

	double granularity = 0.1;
	double t;
	unsigned i = 0;

	/* following cardioid -> neck */
	for (t = 0.0; t < M_PI; t += granularity)
		do_image_for_point(window, f, i++,
				imaginary_cardioid_from_theta(t));

	/* neck -> above neck */
	for (t = 0.0; t < 0.5; t += granularity/10) {
		struct point p = { .x = -0.75, .y = t };
		do_image_for_point(window, f, i++, p);
	}

	/* above neck -> neck */
	for (; t > 0.0; t -= granularity/10) {
		struct point p = { .x = -0.75, .y = t };
		do_image_for_point(window, f, i++, p);
	}

	/* neck -> end */
	for (t = -0.75; t > -(2.0 + 2 * granularity); t -= granularity) {
		struct point p = { .x = t, .y = 0 };
		do_image_for_point(window, f, i++, p);
	}

	/* end -> neck -> cardioid -> back */
	for (; t < 1; t += granularity) {
		struct point p = { .x = t, .y = 0 };
		do_image_for_point(window, f, i++, p);
	}

	return EXIT_SUCCESS;
}
