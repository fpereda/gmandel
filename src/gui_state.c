/* vim: set sts=4 sw=4 noet : */

/*
 * Copyright (c) 2007, Jesus Rodriguez <foxandxss@gmail.com>
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
#include <string.h>
#include <errno.h>

#include <gtk/gtk.h>

#include "mandelbrot.h"
#include "paint.h"
#include "gui.h"
#include "gui_report.h"
#include "gui_status.h"

static bool gmandel_strtoul(
		GtkWidget *window, const char *var,
		const char *nptr, unsigned long *value)
{
	bool err = false;
	char *del = strrchr(nptr, '\n');
	if (del)
		*del = '\0';
	char *endptr;
	errno = 0;
	unsigned long n = strtoul(nptr, &endptr, 10);
	if (endptr == nptr || *endptr) {
		gui_report_error(window, "Could not get a sane value for %s", var);
		err = true;
	} else if (errno) {
		gui_report_error(window, "Could not get a sane value for %s: %s",
				var, g_strerror(errno));
		err = true;
	}

	*value = n;
	return !err;
}

static bool gmandel_strtod(
		GtkWidget *window, const char *var,
		const char *nptr, double *value)
{
	bool err = false;
	char *del = strrchr(nptr, '\n');
	if (del)
		*del = '\0';
	char *endptr;
	errno = 0;
	double num = strtod(nptr, &endptr);
	if (endptr == nptr || *endptr) {
		gui_report_error(window, "Could not get a sane value for %s", var);
		err = true;
	} else if (errno) {
		gui_report_error(window, "Could not get a sane value for %s: %s",
				var, g_strerror(errno));
		err = true;
	}

	*value = num;
	return !err;
}

bool gui_state_load(GtkWidget *window)
{
	char *filename = NULL;
	bool err = true;

	GtkWidget *fc = gtk_file_chooser_dialog_new(
			"Load a state",	GTK_WINDOW(window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(fc), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(fc)) != GTK_RESPONSE_ACCEPT)
		goto cleanup;

	err = false;

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		gui_report_error(window, "Error loading the state file '%s': %s",
				filename, g_strerror(errno));
		goto cleanup;
	}

	char buf[BUFSIZ];
	unsigned long maxit;
	double ulx;
	double uly;
	double lly;

	fgets(buf, sizeof(buf), file);
	if ((err = !gmandel_strtoul(window, "maxit", buf, &maxit)))
		goto bad_file;

#define READ_VAR_DOUBLE(a) do { \
	fgets(buf, sizeof(buf), file); \
	if ((err = !gmandel_strtod(window, #a, buf, &(a)))) \
		goto bad_file; \
} while (0);

	READ_VAR_DOUBLE(ulx);
	READ_VAR_DOUBLE(uly);
	READ_VAR_DOUBLE(lly);

#undef READ_VAR_DOUBLE

	mandelbrot_set_maxit(maxit);
	// paint_set_limits(ulx, uly, lly);

	gui_status_set("Correctly loaded current state %s", filename);

bad_file:
	if (err)
		gui_status_set("Error when loading current state from %s", filename);
	fclose(file);

cleanup:
	gtk_widget_destroy(fc);
	g_free(filename);
	return !err;
}

bool gui_state_save(GtkWidget *window)
{
	char *filename = NULL;
	bool err = false;

	GtkWidget *fc = gtk_file_chooser_dialog_new(
			"Save the state", GTK_WINDOW(window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(fc), TRUE);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(fc), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(fc)) != GTK_RESPONSE_ACCEPT)
		goto cleanup;

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));

	FILE *file = fopen(filename, "w");
	if (file == NULL) {
		gui_report_error(window, "Error saving the state file '%s': %s\n",
				filename, g_strerror(errno));
		gui_status_set("Error when saving the state file");
		err = true;
		goto cleanup;
	}
	unsigned int maxit = mandelbrot_get_maxit();
	double ulx, uly, lly;
	// paint_get_limits(&ulx, &uly, &lly);
	fprintf(file, "%d\n%a\n%a\n%a\n", maxit, ulx, uly, lly);
	fclose(file);

	gui_status_set("Correctly saved current state to %s", filename);

cleanup:
	gtk_widget_destroy(fc);
	g_free(filename);
	return !err;
}
