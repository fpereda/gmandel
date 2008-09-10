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

static gboolean handle_julia_click(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 2) {
		gfract_orbits_set_active(widget, !gfract_orbits_get_active(widget));
		gfract_clean(widget);
	} else
		return FALSE;

	return TRUE;
}

int main(int argc, char *argv[])
{
	unsigned width = 640;
	unsigned height = 480;
	unsigned maxit = 1000;
	double cx = 0.0;
	double cy = 0.0;
	double ulx = -2.0;
	double uly = 1.5;
	double lly = -1.5;

	GOptionEntry entries[] = 
	{
		{ "width", 'w', 0, G_OPTION_ARG_INT, &width, "Window width" },
		{ "height", 'h', 0, G_OPTION_ARG_INT, &height, "Window height" },
		{ "maxit", 'i', 0, G_OPTION_ARG_INT, &maxit, "Number of iterations" },
		{ "cx", 'x', 0, G_OPTION_ARG_DOUBLE, &cx, "Real part of c" },
		{ "cy", 'y', 0, G_OPTION_ARG_DOUBLE, &cy, "Imaginary part of c" },
		{ "ulx", 0, 0, G_OPTION_ARG_DOUBLE, &ulx,
			"Real part of the upper left corner" },
		{ "uly", 0, 0, G_OPTION_ARG_DOUBLE, &uly,
			"Imaginary part of the upper left corner" },
		{ "lly", 0, 0, G_OPTION_ARG_DOUBLE, &lly,
			"Imaginary part of the lower left corner" },
		{ NULL }
	};

	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();

	GError *error = NULL;
	if (!gtk_init_with_args(&argc, &argv, NULL, entries, NULL, &error)) {
		g_print("error: %s\n", error->message);
		return EXIT_FAILURE;
	}

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	GtkWidget *f = gfract_new_julia(window, width, height);
	gfract_set_limits(f, ulx, uly, lly);
	gfract_set_maxit(f, maxit);

	g_signal_connect(f, "button-press-event",
			G_CALLBACK(handle_julia_click), NULL);

	gfract_set_ratios(f,
			color_get(COLOR_THEME_GREENPARK)->red,
			color_get(COLOR_THEME_GREENPARK)->blue,
			color_get(COLOR_THEME_GREENPARK)->green);

	gfract_set_center(f, cx, cy);

	gchar *title = g_strdup_printf("Julia Set for c(%G, %G)", cx, cy);
	gtk_window_set_title(GTK_WINDOW(window), title);
	g_free(title);

	gtk_container_add(GTK_CONTAINER(window), f);
	gtk_widget_show_all(window);

	gtk_main();

	gdk_threads_leave();

	return EXIT_SUCCESS;
}
