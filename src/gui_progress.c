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

#include "gui_progress.h"

static GtkWidget *win = NULL;
static GtkWidget *bar = NULL;
static gdouble step = 0.1;

void gui_progress_begin(unsigned ticks)
{
	if (!win) {
		win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		bar = gtk_progress_bar_new();
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(bar),
				"Computing");
		gtk_container_add(GTK_CONTAINER(win), bar);
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar), 0);
	step = 1.0 / ticks;
	gtk_widget_show_all(win);
}

void gui_progress_tick(void)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar),
			gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(bar)) + step);
}

void gui_progress_end(void)
{
	gtk_widget_hide_all(win);
}
