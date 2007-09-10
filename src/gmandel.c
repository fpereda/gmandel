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

static GtkWidget *window = NULL;
static GtkWidget *drawing_area = NULL;
static GtkToggleAction *orbits_action = NULL;
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

	if (do_orbits) {
		double ulx;
		double uly;
		double lly;
		paint_get_limits(&ulx, &uly, &lly);

		unsigned width;
		unsigned height;
		paint_get_window_size(&width, &height);

		double inc_y = uly - lly;
		double inc = inc_y / (height - 1);

		long double x = event->x * inc + ulx;
		long double y = -(event->y * inc - uly);

		clean_mandel();
		paint_orbit(widget, x, y);
	} else if (do_select) {
		clean_mandel();
		paint_box(widget, select_orig_x, select_orig_y, event->x, event->y);
	}

	/* we are done so ask for more events */
	gdk_window_get_pointer(widget->window, NULL, NULL, NULL);
	return FALSE;
}

gboolean handle_click(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	if (states == NULL)
		states = stack_alloc_init(free);

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

			double ulx;
			double uly;
			double lly;
			paint_box_limits(select_orig_x, select_orig_y, event->x, event->y,
					&ulx, &uly, &lly);
			paint_set_limits(ulx, uly, lly);
			do_select = false;
		}
	} else if (event->button == 3) {
		if (stack_empty(states))
			return FALSE;
		paint_set_observer_state(stack_peek(states));
		free(stack_pop(states));
	} else if (event->button == 2) {
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
	switch (event->keyval) {
		case GDK_KP_Add:
		case GDK_plus:
			nextmaxit += 100;
			break;
		case GDK_KP_Subtract:
		case GDK_minus:
			nextmaxit -= 100;
			break;
		case GDK_Up:
		case GDK_KP_Up:
			paint_move_up(7);
			break;
		case GDK_Down:
		case GDK_KP_Down:
			paint_move_down(7);
			break;
		case GDK_Left:
		case GDK_KP_Left:
			paint_move_left(7);
			break;
		case GDK_Right:
		case GDK_KP_Right:
			paint_move_right(7);
			break;
	}

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

gboolean handle_save(GtkAction *action, gpointer data)
{
	gui_save_current(window);
	return FALSE;
}

gboolean handle_recompute(GtkAction *action, gpointer data)
{
	paint_force_redraw(drawing_area, true);
	return FALSE;
}

gboolean toggle_orbits(GtkToggleAction *action, gpointer data)
{
	clean_mandel();
	do_orbits = !do_orbits;
	return FALSE;
}

gboolean theme_changed(GtkWidget *widget, gpointer data)
{
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) == FALSE)
			return FALSE;
	char *name = data;
	char **names = color_get_names();
	for (unsigned i = 0; i < COLOR_THEME_LAST; i++)
		if (strcmp(name, names[i]) == 0)
			color_set_current(i);
	return FALSE;
}

gboolean handle_about(GtkAction *action, gpointer data)
{
	gui_about_show(window);
	return FALSE;
}

GtkWidget *build_menu(void)
{
	static const GtkActionEntry entries[] = {
		{ "FileMenu", NULL, "_File" },
		{ "ControlsMenu", NULL, "_Controls" },
		{ "ColorMenu", NULL, "Color _Themes" },
		{ "HelpMenu", GTK_STOCK_HELP, "_Help" },
		{ "Save", GTK_STOCK_SAVE_AS, "_Save as...",
			"<control>S", "Save current image", G_CALLBACK(handle_save) },
		{ "Exit", GTK_STOCK_QUIT, "E_x_it",
			"<control>Q", "Exit the program", gtk_main_quit },
		{ "About", GTK_STOCK_ABOUT, "_About",
			NULL, "Show about information", G_CALLBACK(handle_about) },
		{ "Recompute", GTK_STOCK_EXECUTE, "_Recompute",
			"<control>R", "Recompute the set", G_CALLBACK(handle_recompute) },
	};

	static const GtkToggleActionEntry toggle_entries[] = {
		{ "Orbits", NULL, "_Orbits",
			"<control>O", "Activate / Deactivate mandelbrot orbits",
			G_CALLBACK(toggle_orbits), FALSE },
	};

	static const char *ui_desc =
		"<ui>"
		"  <menubar name='MainMenu'>"
		"    <menu action='FileMenu'>"
		"      <menuitem action='Save'/>"
		"      <separator/>"
		"      <menuitem action='Exit'/>"
		"    </menu>"
		"    <menu action='ControlsMenu'>"
		"      <menuitem action='Recompute'/>"
		"      <menuitem action='Orbits'/>"
		"    </menu>"
		"    <menu action='HelpMenu'>"
		"      <menuitem action='About'/>"
		"    </menu>"
		"  </menubar>"
		"</ui>";

	GtkActionGroup *action_group = gtk_action_group_new("MenuActions");
	gtk_action_group_add_actions(
			action_group, entries, G_N_ELEMENTS(entries), window);
	gtk_action_group_add_toggle_actions(
			action_group, toggle_entries, G_N_ELEMENTS(toggle_entries), window);

	GtkUIManager *ui_manager = gtk_ui_manager_new();

	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);
	gtk_window_add_accel_group(GTK_WINDOW(window),
			gtk_ui_manager_get_accel_group(ui_manager));

	GError *error = NULL;
	if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_desc, -1, &error)) {
		g_message("building menus failed: %s", error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	GtkAction *tmp_action = gtk_ui_manager_get_action(
			ui_manager, "/MainMenu/ControlsMenu/Orbits");
	orbits_action = GTK_TOGGLE_ACTION(tmp_action);

	GtkWidget *help_menu = gtk_ui_manager_get_widget(
			ui_manager, "/MainMenu/HelpMenu");
	gtk_menu_item_set_right_justified(GTK_MENU_ITEM(help_menu), TRUE);
	return gtk_ui_manager_get_widget(ui_manager, "/MainMenu");
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
	gtk_box_pack_start(GTK_BOX(lyout_top), build_menu(), FALSE, FALSE, 0);
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
