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

#include "gui_save.h"
#include "paint.h"

void gui_save_current(GtkWidget *window)
{
	unsigned width;
	unsigned height;
	paint_get_window_size(&width, &height);

	GtkWidget *fc = gtk_file_chooser_dialog_new(
			"Save current image", GTK_WINDOW(window),
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

