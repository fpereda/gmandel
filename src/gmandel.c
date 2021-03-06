/* vim: set sts=4 sw=4 noet : */

/*
 * Copyright (c) 2007, Fernando J. Pereda <ferdy@ferdyx.org>
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

#include "xfuncs.h"
#include "gui.h"
#include "gui_callbacks.h"
#include "gui_menu.h"
#include "gui_status.h"
#include "color.h"

#include "gfract.h"

static void set_sensitive(gpointer data)
{
	gtk_widget_set_sensitive(data, TRUE);
}

static void set_insensitive(gpointer data)
{
	gtk_widget_set_sensitive(data, FALSE);
}

int main(int argc, char *argv[])
{
	unsigned width;
	unsigned height;
	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	} else {
		width = 900;
		height = 600;
	}

	struct gui_params gui_state = {
		.window = NULL,
		.fract = NULL,
	};

	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();

	gtk_init(&argc, &argv);

	gui_state.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *window = gui_state.window;
	gtk_window_set_title(GTK_WINDOW(window), "gmandel");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy",
			G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
	g_signal_connect(window, "key-press-event",
			G_CALLBACK(handle_keypress), &gui_state);

	gui_state.fract = gfract_new_mandel(width, height);
	gfract_set_ratios(gui_state.fract,
			color_get(COLOR_THEME_ICEBLUE)->red,
			color_get(COLOR_THEME_ICEBLUE)->blue,
			color_get(COLOR_THEME_ICEBLUE)->green);
	g_signal_connect(gui_state.fract, "button-press-event",
			G_CALLBACK(handle_click), NULL);

	GtkWidget *layout = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(layout),
			gui_menu_build(window, &gui_state), FALSE, FALSE, 0);

	GtkWidget *progbox = gtk_hbox_new(FALSE, 0);
	GtkWidget *prog = gtk_progress_bar_new();
	GtkWidget *stopb = gtk_button_new_from_stock(GTK_STOCK_STOP);
	g_signal_connect_swapped(stopb, "clicked",
			G_CALLBACK(gfract_stop), gui_state.fract);
	gtk_container_add(GTK_CONTAINER(progbox), prog);
	gtk_box_pack_start(GTK_BOX(progbox), stopb, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(layout), progbox);

	gfract_set_progress(gui_state.fract, prog);
	gfract_set_progress_hook_start(gui_state.fract,
			set_sensitive, stopb);
	gfract_set_progress_hook_finish(gui_state.fract,
			set_insensitive, stopb);

	gtk_container_add(GTK_CONTAINER(layout), gui_state.fract);
	gtk_container_add(GTK_CONTAINER(layout), gui_status_build());

	gtk_container_add(GTK_CONTAINER(window), layout);

	gtk_widget_show_all(window);

	gtk_main();

	gdk_threads_leave();

	return EXIT_SUCCESS;
}
