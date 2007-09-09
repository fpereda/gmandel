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

#define SIZEOF_ARRAY(a) (sizeof(a)/sizeof(a[0]))

static GtkWidget *drawing_area = NULL;
static stack *states = NULL;
static bool do_orbits = false;

static void save_current(void)
{
	unsigned width;
	unsigned height;
	paint_get_window_size(&width, &height);

	GtkWidget *w = gtk_widget_get_toplevel(drawing_area);
	GtkWidget *fc = gtk_file_chooser_dialog_new(
			"Save current image", GTK_WINDOW(w),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(
			GTK_FILE_CHOOSER(fc), TRUE);
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "JPG, PNG and BMP files");
	gtk_file_filter_add_pixbuf_formats(filter);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(fc), filter);

	if (gtk_dialog_run(GTK_DIALOG(fc)) != GTK_RESPONSE_ACCEPT)
		goto cleanup;

	char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));
	char *format = strrchr(filename, '.');
	if (!format)
		format = "png";
	else
		format++;
	if (strcmp(format, "jpg") == 0)
		format = "jpeg";
	else if (strcmp(format, "jpeg") != 0
			&& strcmp(format, "bmp") != 0)
		format = "png";

	GdkPixbuf *buf = gdk_pixbuf_get_from_drawable(
			NULL, paint_get_pixmap(), NULL, 0, 0, 0, 0, width, height);
	gdk_pixbuf_save(buf, filename, format, NULL, NULL);
	g_free(filename);

cleanup:
	gtk_widget_destroy(fc);
}

gboolean save_wrapper(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	save_current();
	return FALSE;
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
	if (!do_orbits)
		return FALSE;

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

	GdkRectangle all = { .x = 0, .y = 0, .width = -1, .height = -1};
	paint_mandel(widget, all, false);
	paint_orbit(widget, x, y);

	/* we are done so ask for more events */
	gdk_window_get_pointer(widget->window, NULL, NULL, NULL);
	return FALSE;
}

gboolean handle_click(GtkWidget *widget, GdkEventButton *event, gpointer data)
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
	} else if (event->button == 3) {
		if (stack_empty(states))
			return FALSE;
		paint_set_observer_state(stack_peek(states));
		free(stack_pop(states));
	} else
		return FALSE;

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

	if (nextmaxit != mandelbrot_get_maxit()) {
		mandelbrot_set_maxit(nextmaxit);
		printf("\rmaxit = %-6d", mandelbrot_get_maxit());
		fflush(stdout);
	}

	return FALSE;
}

gboolean handle_recompute(GtkWidget *widget, gpointer data)
{
	paint_force_redraw(drawing_area, true);
	return FALSE;
}

gboolean handle_clean(GtkWidget *widget, gpointer data)
{
	GdkRectangle all = { .x = 0, .y = 0, .width = -1, .height = -1 };
	paint_mandel(drawing_area, all, false);
	return FALSE;
}

gboolean toggle_orbits(GtkWidget *widget, gpointer data)
{
	GdkRectangle all = { .x = 0, .y = 0, .width = -1, .height = -1 };
	paint_mandel(drawing_area, all, false);
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

gboolean handle_about(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gui_about_show(gtk_widget_get_toplevel(widget));
	return FALSE;
}

GtkWidget *build_menu(void)
{
	GtkWidget *menu = gtk_menu_bar_new();

	/* File menu */
	GtkWidget *file = gtk_menu_item_new_with_mnemonic("_File");
	GtkWidget *filemenu = gtk_menu_new();

	GtkWidget *save = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
	g_signal_connect(save, "activate",
			G_CALLBACK(save_wrapper), NULL);

	GtkWidget *quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	g_signal_connect(quit, "activate",
			G_CALLBACK(gtk_main_quit), NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),
			gtk_separator_menu_item_new());
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);

	/* Controls */
	GtkWidget *controls = gtk_menu_item_new_with_mnemonic("_Controls");
	GtkWidget *controls_menu = gtk_menu_new();

	GtkWidget *recompute = gtk_menu_item_new_with_mnemonic("_Recompute");
	g_signal_connect(recompute, "activate", G_CALLBACK(handle_recompute), NULL);
	GtkWidget *clean = gtk_menu_item_new_with_mnemonic("_Clean");
	g_signal_connect(clean, "activate", G_CALLBACK(handle_clean), NULL);
	GtkWidget *orbits = gtk_check_menu_item_new_with_mnemonic("_Orbits");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(orbits), FALSE);
	g_signal_connect(orbits, "toggled", G_CALLBACK(toggle_orbits), NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), recompute);
	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), clean);
	gtk_menu_shell_append(GTK_MENU_SHELL(controls_menu), orbits);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(controls), controls_menu);

	/* Colors */
	GtkWidget *colors = gtk_menu_item_new_with_mnemonic("Color _Themes");
	GtkWidget *colors_menu = gtk_menu_new();

	GSList *themes_group = NULL;
	char **color_names = color_get_names();
	for (unsigned i = 0; i < COLOR_THEME_LAST; i++) {
		GtkWidget *theme = gtk_radio_menu_item_new_with_label(
				themes_group, color_names[i]);
		themes_group = gtk_radio_menu_item_get_group(
				GTK_RADIO_MENU_ITEM(theme));
		if (i == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(theme), TRUE);
		g_signal_connect(theme, "activate",
				G_CALLBACK(theme_changed), color_names[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(colors_menu), theme);
	}

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(colors), colors_menu);

	/* Help menu */
	GtkWidget *help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, NULL);
	GtkWidget *helpmenu = gtk_menu_new();

	GtkWidget *about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT,
			NULL);
	g_signal_connect(about, "activate",
			G_CALLBACK(handle_about), NULL);

	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);

	/* Add everything to the menubar */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), controls);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), colors);
	gtk_menu_item_set_right_justified(GTK_MENU_ITEM(help), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), help);

	return menu;
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
