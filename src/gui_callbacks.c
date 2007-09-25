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
#include "paint.h"
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

#if 0
gboolean handle_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	struct gui_params *gui = data;
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
			paint_force_redraw(gui->drawing_area, false);
			break;
	}

	if (event->keyval == GDK_Escape && gui->do_select) {
		gui->do_select = false;
		paint_clean_mandel(gui->drawing_area);
	}

	if (nextmaxit != mandelbrot_get_maxit()) {
		mandelbrot_set_maxit(nextmaxit);
		gui_status_set("maxit = %-6d", mandelbrot_get_maxit());
	}

	return FALSE;
}
#endif
