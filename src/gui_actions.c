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

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "paint.h"
#include "color.h"
#include "gui.h"
#include "gui_about.h"
#include "gui_save.h"
#include "gui_actions.h"

void handle_save(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	gui_save_current(gui->window);
}

void handle_restart(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	paint_set_limits_default();
	while (!stack_empty(gui->states))
		gui->states->destroy(stack_pop(gui->states));
	paint_force_redraw(gui->drawing_area, true);
}

void handle_recompute(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	paint_force_redraw(gui->drawing_area, true);
}

void toggle_orbits(GtkToggleAction *action, gpointer data)
{
	struct gui_params *gui = data;
	paint_clean_mandel(gui->drawing_area);
	gui->do_orbits = !gui->do_orbits;
}

void theme_changed(
		GtkRadioAction *action, GtkRadioAction *current, gpointer data)
{
	struct gui_params *gui = data;
	const char *name = gtk_action_get_name(GTK_ACTION(current));
	char **names = color_get_names();
	for (unsigned i = 0; i < COLOR_THEME_LAST; i++)
		if (strcmp(name, names[i]) == 0)
			color_set_current(i);
	paint_force_redraw(gui->drawing_area, false);
}

void handle_about(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	gui_about_show(gui->window);
}

void handle_screenshot(gpointer data)
{
	struct gui_params *gui = data;
	gui_save_screenshot(gui->drawing_area);
}