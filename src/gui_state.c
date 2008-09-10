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
#include "gui.h"
#include "gui_report.h"
#include "gui_status.h"
#include "gfract.h"
#include "xfuncs.h"

struct observer_state {
	double ulx;
	double uly;
	double lly;
};

static void gmandel_stripnl(char *s)
{
	for (; s && *s; s++)
		if (*s == '\n')
			*s = '\0';
}

static bool gmandel_fgets(GtkWidget *window, char *buf, int n, FILE *file)
{
	if (fgets(buf, n, file) != NULL) {
		gmandel_stripnl(buf);
		return true;
	}

	if (feof(file))
		gui_report_error(window, "Unexpected EOF");
	else if (ferror(file))
		gui_report_error(window, "Unexpected error");

	return false;
}

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

static bool loader_1(struct gui_params *gui, FILE *file)
{
	char buf[BUFSIZ];
	unsigned long maxit;
	double ulx;
	double uly;
	double lly;

	if (!gmandel_fgets(gui->window, buf, sizeof(buf), file))
		return false;
	else if (!gmandel_strtoul(gui->window, "maxit", buf, &maxit))
		return false;

#define READ_VAR_DOUBLE(a) do { \
	if (!gmandel_fgets(gui->window, buf, sizeof(buf), file)) \
		return false; \
	else if (!gmandel_strtod(gui->window, #a, buf, &(a))) \
		return false; \
} while (0);

	READ_VAR_DOUBLE(ulx);
	READ_VAR_DOUBLE(uly);
	READ_VAR_DOUBLE(lly);

#undef READ_VAR_DOUBLE

	gfract_set_maxit(gui->fract, maxit);
	gfract_set_limits(gui->fract, ulx, uly, lly);

	gfract_clear_history(gui->fract);

	return true;
}

static bool loader_2(struct gui_params *gui, FILE *file)
{
	if (!loader_1(gui, file))
		return false;

	char buf[BUFSIZ];

	unsigned long num_elem;
	if (!gmandel_fgets(gui->window, buf, sizeof(buf), file))
		return false;
	else if (!gmandel_strtoul(gui->window, "num_elem", buf, &num_elem))
		return false;

	GSList *nh = NULL;

#define READ_VAR_DOUBLE(a) do { \
	if (!gmandel_fgets(gui->window, buf, sizeof(buf), file)) \
		return false; \
	else if (!gmandel_strtod(gui->window, #a, buf, &(a))) \
		return false; \
} while (0);

	while (num_elem--) {
		struct observer_state *o = xmalloc(sizeof(*o));
		READ_VAR_DOUBLE(o->ulx);
		READ_VAR_DOUBLE(o->uly);
		READ_VAR_DOUBLE(o->lly);
		nh = g_slist_prepend(nh, o);
	}

#undef READ_VAR_DOUBLE

	gfract_set_history(gui->fract, nh);
	g_slist_free(nh);

	return true;
}

enum {
	GMANDEL_STATE_0 = 0, /* old, unsupported, format */
	GMANDEL_STATE_1,
	GMANDEL_STATE_2,
	GMANDEL_STATE_CURRENT = GMANDEL_STATE_2,
	GMANDEL_STATE_LAST,
};

static const char *format_strings[] = {
	[GMANDEL_STATE_1] = "gmandel-1",
	[GMANDEL_STATE_2] = "gmandel-2",
	[GMANDEL_STATE_LAST] = NULL,
};

static bool (*format_loaders[])(struct gui_params *, FILE *) = {
	[GMANDEL_STATE_0] = NULL,
	[GMANDEL_STATE_1] = loader_1,
	[GMANDEL_STATE_2] = loader_2,
	[GMANDEL_STATE_LAST] = NULL,
};

bool gui_state_load(struct gui_params *gui)
{
	char *filename = NULL;
	bool err = true;

	GtkWidget *fc = gtk_file_chooser_dialog_new(
			"Load a state",	GTK_WINDOW(gui->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	g_object_ref_sink(fc);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(fc), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(fc)) != GTK_RESPONSE_ACCEPT)
		goto cleanup;

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc));

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		gui_report_error(gui->window, "Could not open '%s': %s",
				filename, g_strerror(errno));
		goto cleanup;
	}

	char buf[BUFSIZ];
	if (!gmandel_fgets(gui->window, buf, sizeof(buf), file)) {
		err = true;
		goto bad_file;
	}

	gmandel_stripnl(buf);

	bool found = false;
	for (unsigned i = GMANDEL_STATE_CURRENT; i > GMANDEL_STATE_0; i--) {
		if (strcmp(buf, format_strings[i]) != 0)
			continue;
		found = true;
		err = !(*format_loaders[i])(gui, file);
		break;
	}

	if (!found)
		gui_report_error(gui->window, "Unsupported format");

bad_file:
	fclose(file);
	if (err)
		gui_status_set("Error while loading current state from %s", filename);
	else
		gui_status_set("Loaded state from %s", filename);
cleanup:
	gtk_widget_destroy(fc);
	g_free(filename);
	return !err;
}

void write_hist_entry(struct observer_state *o, FILE *f)
{
	fprintf(f, "%a\n%a\n%a\n", o->ulx, o->uly, o->lly);
}

bool gui_state_save(struct gui_params *gui)
{
	char *filename = NULL;
	bool err = false;

	GtkWidget *fc = gtk_file_chooser_dialog_new(
			"Save the state", GTK_WINDOW(gui->window),
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
		gui_report_error(gui->window, "Error saving the state file '%s': %s\n",
				filename, g_strerror(errno));
		gui_status_set("Error when saving the state file");
		err = true;
		goto cleanup;
	}

	fprintf(file, "%s\n", format_strings[GMANDEL_STATE_CURRENT]);

	unsigned int maxit = gfract_get_maxit(gui->fract);
	double ulx, uly, lly;
	gfract_get_limits(gui->fract, &ulx, &uly, &lly);
	fprintf(file, "%d\n%a\n%a\n%a\n", maxit, ulx, uly, lly);

	GSList *hist = g_slist_reverse(gfract_get_history(gui->fract));
	guint n = g_slist_length(hist);
	fprintf(file, "%u\n", n);
	g_slist_foreach(hist, (GFunc)write_hist_entry, file);

	fclose(file);

	gui_status_set("Correctly saved current state to %s", filename);

cleanup:
	gtk_widget_destroy(fc);
	g_free(filename);
	return !err;
}
