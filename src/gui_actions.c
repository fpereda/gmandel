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

#include "color.h"
#include "gui.h"
#include "gui_about.h"
#include "gui_save.h"
#include "gui_actions.h"
#include "gui_state.h"
#include "gfract.h"

void handle_savestate(GtkAction *action, gpointer data)
{
	gui_state_save(data);
}

void handle_loadstate(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	if (!gui_state_load(gui))
		return;
	gfract_mandel_history_clear(gui->fract);
	gfract_mandel_compute(gui->fract);
}

void handle_save(GtkAction *action, gpointer data)
{
	gui_save_current(data);
}

void handle_restart(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	gfract_mandel_set_limits_default(gui->fract);
	gfract_mandel_history_clear(gui->fract);
	gfract_mandel_compute(gui->fract);
}

void handle_recompute(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	gfract_mandel_compute(gui->fract);
}

void toggle_orbits(GtkToggleAction *action, gpointer data)
{
	struct gui_params *gui = data;
	gfract_mandel_orbits_set_active(gui->fract,
			!gfract_mandel_orbits_get_active(gui->fract));
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
	gfract_mandel_redraw(gui->fract);
}

void handle_about(GtkAction *action, gpointer data)
{
	struct gui_params *gui = data;
	gui_about_show(gui->window);
}

void handle_screenshot(gpointer data)
{
	gui_save_screenshot(data);
}
