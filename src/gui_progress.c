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

#include <stdbool.h>

#include <gtk/gtk.h>

#include "gui_progress.h"
#include "xfuncs.h"

static inline void gtk_events_flush(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}

static void call_stop(GtkButton *button, gpointer data)
{
	gui_progress_stop(data);
}

struct gui_progress *
gui_progress_start(GtkWidget *parent, unsigned ticks,
		void (*stopf)(GtkWidget*), GtkWidget *stopa)
{
	struct gui_progress* g = xmalloc(sizeof(*g));
	g->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_transient_for(GTK_WINDOW(g->win), GTK_WINDOW(parent));
	gtk_window_set_position(GTK_WINDOW(g->win),
			GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_modal(GTK_WINDOW(g->win), TRUE);
	gtk_window_set_decorated(GTK_WINDOW(g->win), FALSE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(g->win), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(g->win), TRUE);

	g->bar = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(g->bar), "Computing");

	GtkWidget *but = gtk_button_new_from_stock(GTK_STOCK_STOP);
	g_signal_connect(but, "clicked", G_CALLBACK(call_stop), g);

	GtkWidget *box = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start_defaults(GTK_BOX(box), g->bar);
	gtk_box_pack_end_defaults(GTK_BOX(box), but);

	gtk_container_add(GTK_CONTAINER(g->win), box);

	g->step = 1.0L / ticks;
	g->cur = 0;
	g->active = true;
	g->stopf = stopf;
	g->stopa = stopa;

	gtk_widget_show_all(g->win);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g->bar), 0);

	gtk_events_flush();

	return g;
}

void gui_progress_stop(struct gui_progress *g)
{
	(*g->stopf)(g->stopa);
}

void gui_progress_tick(struct gui_progress *g)
{
	g->cur += g->step;
	if (g->cur > 1.0L)
		return;
	snprintf(g->buf, sizeof(g->buf), "Computing %d %%", (int)(g->cur * 100));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(g->bar), g->buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(g->bar), g->cur);
	gtk_events_flush();
}

void gui_progress_end(struct gui_progress *g)
{
	g->active = false;
	gtk_widget_hide_all(g->win);
	free(g);
	gtk_events_flush();
}
