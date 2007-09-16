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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "color.h"
#include "gui_menu.h"
#include "gui_actions.h"
#include "gui_callbacks.h"

static GtkToggleAction *orbits_action = NULL;

GtkToggleAction *gui_menu_get_orbits_action(void)
{
	return orbits_action;
}

GtkWidget *gui_menu_build(GtkWidget *window, gpointer data)
{
	static const GtkActionEntry entries[] = {
		{ "FileMenu", NULL, "_File" },
		{ "ControlsMenu", NULL, "_Controls" },
		{ "ColorMenu", NULL, "Color _Themes" },
		{ "HelpMenu", GTK_STOCK_HELP, "_Help" },
		{ "SaveImage", GTK_STOCK_SAVE_AS, "_Save image",
			"<shift><control>S", "Save current image", G_CALLBACK(handle_save) },
		{ "SaveCurrentState", GTK_STOCK_SAVE, "S_ave current state",
			"<control>S", "Save current state", G_CALLBACK(handle_savestate) },
		{ "LoadCurrentState", GTK_STOCK_JUMP_TO, "L_oad current state",
			"<control>O", "Load saved state", G_CALLBACK(handle_loadstate) },
		{ "Exit", GTK_STOCK_QUIT, "E_xit",
			"<control>Q", "Exit the program", gtk_main_quit },
		{ "About", GTK_STOCK_ABOUT, "_About",
			NULL, "Show about information", G_CALLBACK(handle_about) },
		{ "Recompute", GTK_STOCK_EXECUTE, "_Recompute",
			"<control>R", "Recompute the set", G_CALLBACK(handle_recompute) },
		{ "Restart", GTK_STOCK_HOME, "R_estart",
			"<control>H", "Restart the set", G_CALLBACK(handle_restart) },
	};

	static const GtkToggleActionEntry toggle_entries[] = {
		{ "Orbits", NULL, "_Orbits",
			"<alt>O", "Activate / Deactivate mandelbrot orbits",
			G_CALLBACK(toggle_orbits), FALSE },
	};

	static GtkRadioActionEntry radio_entries[COLOR_THEME_LAST];
	char **theme_names = color_get_names();
	for (unsigned i = 0; i < G_N_ELEMENTS(radio_entries); i++) {
		gchar *tooltip = g_strdup_printf("Set the '%s' theme", theme_names[i]);
		GtkRadioActionEntry ent = {
			g_strdup(theme_names[i]), NULL, g_strdup(theme_names[i]),
			NULL, tooltip, i };
		radio_entries[i] = ent;
	}

	static const char *ui_desc_pre =
		"<ui>"
		"  <menubar name='MainMenu'>"
		"    <menu action='FileMenu'>"
		"      <menuitem action='LoadCurrentState' />"
		"      <menuitem action='SaveCurrentState' />"
		"      <separator />"
		"      <menuitem action='SaveImage'/>"
		"      <separator/>"
		"      <menuitem action='Exit'/>"
		"    </menu>"
		"    <menu action='ControlsMenu'>"
		"      <menuitem action='Restart' />"
		"      <menuitem action='Recompute'/>"
		"      <menuitem action='Orbits'/>"
		"    </menu>"
		"    <menu action='ColorMenu'>";

	gchar *themes_menu_desc = g_strdup("");
	for (unsigned i = 0; i < G_N_ELEMENTS(radio_entries); i++) {
		char *buf = g_strdup_printf("<menuitem action='%s'/>", theme_names[i]);
		gchar *tmp = g_strconcat(themes_menu_desc, buf, NULL);
		g_free(buf);
		g_free(themes_menu_desc);
		themes_menu_desc = tmp;
	}

	static const char *ui_desc_post =
		"    </menu>"
		"    <menu action='HelpMenu'>"
		"      <menuitem action='About'/>"
		"    </menu>"
		"  </menubar>"
		"</ui>";

	gchar *ui_desc = g_strconcat(
			ui_desc_pre, themes_menu_desc, ui_desc_post, NULL);

	GtkActionGroup *action_group = gtk_action_group_new("MenuActions");
	gtk_action_group_add_actions(
			action_group, entries, G_N_ELEMENTS(entries), data);
	gtk_action_group_add_toggle_actions(
			action_group, toggle_entries, G_N_ELEMENTS(toggle_entries), data);
	gtk_action_group_add_radio_actions(
			action_group, radio_entries, G_N_ELEMENTS(radio_entries),
			0, G_CALLBACK(theme_changed), data);

	GtkUIManager *ui_manager = gtk_ui_manager_new();

	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);
	gtk_window_add_accel_group(GTK_WINDOW(window),
			gtk_ui_manager_get_accel_group(ui_manager));

	GtkAccelGroup *misc_accel = gtk_accel_group_new();
	GClosure *screen_clos = g_cclosure_new_swap(
			G_CALLBACK(handle_screenshot), data, NULL);
	gtk_accel_group_connect(misc_accel, GDK_s, GDK_MOD1_MASK,
			GTK_ACCEL_VISIBLE, screen_clos);
	gtk_window_add_accel_group(GTK_WINDOW(window), misc_accel);

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
	GtkWidget *menu = gtk_ui_manager_get_widget(ui_manager, "/MainMenu");

	g_free(ui_desc);

	return menu;
}
