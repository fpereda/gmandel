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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "mandelbrot.h"
#include "paint.h"
#include "stack.h"
#include "xfuncs.h"

#define SIZEOF_ARRAY(a) (sizeof(a)/sizeof(a[0]))

static GtkWidget *drawing_area = NULL;
static stack *states = NULL;

gboolean handle_expose(
		GtkWidget *widget,
		GdkEventExpose *event,
		gpointer data)
{
	paint_mandel(widget);
	return FALSE;
}

gboolean handle_click(
		GtkWidget *widget,
		GdkEventButton *event,
		gpointer data)
{
	if (states == NULL)
		states = stack_alloc_init(free);

	double ulx;
	double uly;
	double lly;
	paint_get_limits(&ulx, &uly, &lly);

	unsigned width;
	unsigned height;
	paint_get_window_size(&width, &height);

	double inc_y = uly - lly;
	double inc = inc_y / (height - 1);
	double inc_x = width * inc;

	long double x = event->x * inc + ulx;
	long double y = -(event->y * inc - uly);

#if 0
	fprintf(stderr, "x = %LF | y = %LF || inc_y = %g | inc_x = %g\n",
			x, y, inc_y, inc_x);
#endif

	if (event->button == 1) {
		struct observer_state *cur = xmalloc(sizeof(*cur));
		paint_get_observer_state(cur);
		stack_push(states, cur);

		inc_y /= 10;
		inc_x /= 10;

		uly = y + inc_y/2;
		lly = y - inc_y/2;
		ulx = x - inc_x/2;
		paint_set_limits(ulx, uly, lly);
	} else {
		if (stack_empty(states))
			return FALSE;
		paint_set_observer_state(stack_peek(states));
		free(stack_pop(states));
	}

	paint_force_redraw(widget, 1);

	return FALSE;
}

gboolean handle_keypress(
		GtkWidget *widget,
		GdkEventKey *event,
		gpointer data)
{
	int nextmaxit = mandelbrot_get_maxit();
	switch (event->keyval) {
		case GDK_KP_Add:
		case GDK_plus:
			nextmaxit += 100;
			break;
		case GDK_KP_Subtract:
		case GDK_minus:
			nextmaxit -= 100;
			break;
#if 0
		case GDK_Up:
		case GDK_KP_Up:
			paint_move_up();
			break;
		case GDK_Down:
		case GDK_KP_Down:
			paint_move_down();
			break;
		case GDK_Left:
		case GDK_KP_Left:
			paint_move_left();
			break;
		case GDK_Right:
		case GDK_KP_Right:
			paint_move_right();
			break;
#endif
	}

	switch (event->keyval) {
		case GDK_R:
		case GDK_r:
#if 0
		case GDK_Up:
		case GDK_KP_Up:
		case GDK_Down:
		case GDK_KP_Down:
		case GDK_Left:
		case GDK_KP_Left:
		case GDK_Right:
		case GDK_KP_Right:
#endif
			printf("Repainting...\n");
			int clean = (event->keyval == GDK_R
					|| event->keyval == GDK_r);
			paint_force_redraw(drawing_area, clean);
			break;
	}


	if (nextmaxit != mandelbrot_get_maxit()) {
		mandelbrot_set_maxit(nextmaxit);
		printf("maxit = %d\n", mandelbrot_get_maxit());
	}
	
	return FALSE;
}

int main(int argc, char *argv[])
{
	unsigned width;
	unsigned height;
	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
		paint_set_window_size(width, height);
	} else
		paint_get_window_size(&width, &height);

	gtk_init(&argc, &argv);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),
			"Mandelbrot Set generator by Fernando J. Pereda");
	gtk_widget_set_size_request(window, width, height);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
	g_signal_connect(window, "key-press-event",
			G_CALLBACK(handle_keypress), NULL);

	drawing_area = gtk_drawing_area_new();
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

	if (states)
		stack_destroy(states);

	return EXIT_SUCCESS;
}
