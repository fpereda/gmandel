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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "mandelbrot.h"
#include "xfuncs.h"
#include "gui.h"
#include "gui_menu.h"
#include "gui_progress.h"
#include "gui_status.h"
#include "gui_callbacks.h"
#include "gfract.h"

gboolean handle_click(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button != 2)
		return FALSE;

	GtkToggleAction *orbits_action = gui_menu_get_orbits_action();
	gtk_toggle_action_set_active(orbits_action,
			! gtk_toggle_action_get_active(orbits_action));

	gfract_mandel_clean(widget);

	return FALSE;
}

gboolean handle_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	struct gui_params *gui = data;
	int nextmaxit = gfract_mandel_get_maxit(gui->fract);

#define EVENT_KEYVAL_EITHER(a, b) \
	(event->keyval == (a) || event->keyval == (b))

	if (EVENT_KEYVAL_EITHER(GDK_KP_Add, GDK_plus))
		nextmaxit += 100;
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Subtract, GDK_minus))
		nextmaxit -= 100;
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Up, GDK_Up))
		gfract_mandel_move_up(gui->fract, 7);
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Down, GDK_Down))
		gfract_mandel_move_down(gui->fract, 7);
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Left, GDK_Left))
		gfract_mandel_move_left(gui->fract, 7);
	else if (EVENT_KEYVAL_EITHER(GDK_KP_Right, GDK_Right))
		gfract_mandel_move_right(gui->fract, 7);

#undef EVENT_KEYVAL_EITHER

	switch (event->keyval) {
		case GDK_Up:
		case GDK_KP_Up:
		case GDK_Down:
		case GDK_KP_Down:
		case GDK_Left:
		case GDK_KP_Left:
		case GDK_Right:
			gfract_mandel_compute_partial(gui->fract);
			break;
	}

	if (event->keyval == GDK_Escape
			&& gfract_mandel_select_get_active(gui->fract)) {
		gfract_mandel_select_set_active(gui->fract, FALSE);
		gfract_mandel_clean(gui->fract);
	}

	if (nextmaxit != gfract_mandel_get_maxit(gui->fract)) {
		gfract_mandel_set_maxit(gui->fract, nextmaxit);
		gui_status_set("maxit = %-6d", gfract_mandel_get_maxit(gui->fract));
	}

	return FALSE;
}
