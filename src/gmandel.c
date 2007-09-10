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
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "mandelbrot.h"
#include "paint.h"
#include "stack.h"
#include "xfuncs.h"
#include "color.h"
#include "gui_progress.h"
#include "gui_about.h"
#include "gui_save.h"
#include "gui_menu.h"
#include "gui_callbacks.h"

static GtkWidget *window = NULL;
static GtkWidget *drawing_area = NULL;
static stack *states = NULL;
static bool do_orbits = false;

static unsigned select_orig_x = 0;
static unsigned select_orig_y = 0;
static bool do_select = false;

void clean_mandel(void)
{
	GdkRectangle all = { .x = 0, .y = 0, .width = -1, .height = -1};
	paint_mandel(drawing_area, all, false);
}

gboolean handle_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	static stack *pending = NULL;
	if (!pending)
		pending = stack_alloc_init(NULL);
	if (gui_progress_active()) {
		GdkEvent *e = gtk_get_current_event();
		stack_push(pending, e);
		return FALSE;
	}
	paint_mandel_region(widget, event->region, false);
	while (!stack_empty(pending)) {
		GdkEvent *e = stack_pop(pending);
		gtk_main_do_event(e);
		gdk_event_free(e);
	}
	return FALSE;
}

gboolean handle_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	if (!do_orbits && !do_select)
		return FALSE;

	clean_mandel();

	if (do_orbits)
		paint_orbit_pixel(widget, event->x, event->y);
	else if (do_select)
		paint_box(widget, select_orig_x, select_orig_y, event->x, event->y);

	/* we are done so ask for more events */
	gdk_window_get_pointer(widget->window, NULL, NULL, NULL);
	return FALSE;
}

gboolean handle_click(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (do_select && event->button != 1) {
		do_select = false;
		clean_mandel();
		return FALSE;
	}

	if (do_orbits && event->button != 2)
		return FALSE;

	if (event->button == 1) {
		if (!do_select) {
			select_orig_x = event->x;
			select_orig_y = event->y;
			do_select = true;
			return FALSE;
		} else {
			struct observer_state *cur = xmalloc(sizeof(*cur));
			paint_get_observer_state(cur);
			stack_push(states, cur);

			paint_set_box_limits(select_orig_x, select_orig_y,
					event->x, event->y);
			do_select = false;
		}
	} else if (event->button == 3) {
		if (stack_empty(states))
			return FALSE;
		paint_set_observer_state(stack_peek(states));
		free(stack_pop(states));
	} else if (event->button == 2) {
		GtkToggleAction *orbits_action = gui_menu_get_orbits_action();
		gtk_toggle_action_set_active(orbits_action,
				! gtk_toggle_action_get_active(orbits_action));
		return FALSE;
	}

	paint_force_redraw(widget, true);

	return FALSE;
}

gboolean handle_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	int nextmaxit = mandelbrot_get_maxit();

#define EVENT_KEYVAL_EITHER(a, b) \
	(event->keyval == (a) || event->keyval == (b))

	if (EVENT_KEYVAL_EITHER(GDK_KP_Add, GDK_plus))
		nextmaxit += 100;
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Subtract, GDK_minus))
		nextmaxit -= 100;
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Up, GDK_Up))
		paint_move_up(7);
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Down, GDK_Down))
		paint_move_down(7);
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Left, GDK_Left))
		paint_move_left(7);
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Right, GDK_Right))
		paint_move_right(7);

#undef EVENT_KEYVAL_EITHER

	switch (event->keyval) {
		case GDK_Up:
		case GDK_KP_Up:
		case GDK_Down:
		case GDK_KP_Down:
		case GDK_Left:
		case GDK_KP_Left:
		case GDK_Right:
			paint_force_redraw(drawing_area, false);
			break;
	}

	if (event->keyval == GDK_Escape && do_select) {
		do_select = false;
		clean_mandel();
	}

	if (nextmaxit != mandelbrot_get_maxit()) {
		mandelbrot_set_maxit(nextmaxit);
		printf("\rmaxit = %-6d", mandelbrot_get_maxit());
		fflush(stdout);
	}

	return FALSE;
}

void handle_save(GtkAction *action, gpointer data)
{
	gui_save_current(window);
}

void handle_recompute(GtkAction *action, gpointer data)
{
	paint_force_redraw(drawing_area, true);
}

void toggle_orbits(GtkToggleAction *action, gpointer data)
{
	clean_mandel();
	do_orbits = !do_orbits;
}

void theme_changed(
		GtkRadioAction *action, GtkRadioAction *current, gpointer data)
{
	const char *name = gtk_action_get_name(GTK_ACTION(current));
	char **names = color_get_names();
	for (unsigned i = 0; i < COLOR_THEME_LAST; i++)
		if (strcmp(name, names[i]) == 0)
			color_set_current(i);
}

void handle_about(GtkAction *action, gpointer data)
{
	gui_about_show(window);
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

	states = stack_alloc_init(free);

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
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
	gtk_widget_add_events(drawing_area, 0
			| GDK_BUTTON_PRESS_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK);
	g_signal_connect(drawing_area, "button-press-event",
			G_CALLBACK(handle_click), NULL);
	g_signal_connect(drawing_area, "motion-notify-event",
			G_CALLBACK(handle_motion), NULL);

	GtkWidget *lyout_top = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(lyout_top),
			gui_menu_build(window), FALSE, FALSE, 0);
	gtk_box_pack_end_defaults(GTK_BOX(lyout_top), drawing_area);

	gtk_container_add(GTK_CONTAINER(window), lyout_top);

	gui_progress_set_parent(window);

	gtk_widget_show_all(window);
	gtk_main();

	if (states)
		stack_destroy(states);

	putchar('\n');

	return EXIT_SUCCESS;
}
