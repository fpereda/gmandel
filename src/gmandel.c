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
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "paint.h"
#include "stack.h"
#include "xfuncs.h"
#include "gui.h"
#include "gui_callbacks.h"
#include "gui_progress.h"
#include "gui_menu.h"
#include "gui_status.h"

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

	struct gui_params gui_state = {
		.do_orbits = false,
		.do_select = false,
	};

	gui_state.states = stack_alloc_init(free);

	gtk_init(&argc, &argv);

	gui_state.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *window = gui_state.window;
	gtk_window_set_title(GTK_WINDOW(window),
			"Mandelbrot Set generator by Fernando J. Pereda");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
	g_signal_connect(window, "key-press-event",
			G_CALLBACK(handle_keypress), &gui_state);

	gui_state.drawing_area = gtk_drawing_area_new();
	GtkWidget *drawing_area = gui_state.drawing_area;
	gtk_widget_set_size_request(drawing_area, width, height);
	g_signal_connect(drawing_area, "expose-event",
			G_CALLBACK(handle_expose), &gui_state);
	gtk_widget_add_events(drawing_area, 0
			| GDK_BUTTON_PRESS_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK);
	g_signal_connect(drawing_area, "button-press-event",
			G_CALLBACK(handle_click), &gui_state);
	g_signal_connect(drawing_area, "motion-notify-event",
			G_CALLBACK(handle_motion), &gui_state);

	GtkWidget *lyout_top = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(lyout_top),
			gui_menu_build(window, &gui_state), FALSE, FALSE, 0);
	gtk_box_pack_start_defaults(GTK_BOX(lyout_top), drawing_area);
	gtk_box_pack_start_defaults(GTK_BOX(lyout_top), gui_status_build());

	gtk_container_add(GTK_CONTAINER(window), lyout_top);

	gui_progress_set_parent(window);

	gtk_widget_show_all(window);
	gtk_main();

	if (gui_state.states)
		stack_destroy(gui_state.states);

	putchar('\n');

	return EXIT_SUCCESS;
}
