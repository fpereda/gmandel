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

static inline void gtk_events_flush(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}

void gui_progress_set_parent(GtkWidget *parent)
{
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_transient_for(GTK_WINDOW(win), GTK_WINDOW(parent));
	gtk_window_set_position(GTK_WINDOW(win),
			GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(win), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(win), TRUE);

	bar = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(bar), "Computing");
	gtk_container_add(GTK_CONTAINER(win), bar);
}

void gui_progress_begin(unsigned ticks)
{
	gtk_widget_show_all(win);
	step = 1.0L / ticks;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar), 0);
	gtk_events_flush();
}

void gui_progress_tick(void)
{
	static char buf[20];
	gdouble cur = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(bar));
	cur += step;
	if (cur > 1.0L)
		return;
	snprintf(buf, sizeof(buf), "Computing %d %%", (int)(cur * 100));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(bar), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(bar), cur);
	gtk_events_flush();
}

void gui_progress_end(void)
{
	gtk_widget_hide_all(win);
}
