/* vim: set sts=4 sw=4 noet : */

/*
 * Copyright (c) 2007, 2008 Fernando J. Pereda <ferdy@ferdyx.org>
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

#ifndef GMANDEL_GFRACT_H_
#define GMANDEL_GFRACT_H_ 1

G_BEGIN_DECLS

#define GFRACT_TYPE_MANDEL (gfract_mandel_get_type())
#define GFRACT_MANDEL(obj) ( \
	G_TYPE_CHECK_INSTANCE_CAST((obj), \
	GFRACT_TYPE_MANDEL, \
	GFractMandel))
#define GFRACT_MANDEL_CLASS(obj) ( \
	G_TYPE_CHECK_CLASS_CAST((obj), \
	GFRACT_MANDEL, \
	GFractMandelClass))
#define GFRACT_IS_MANDEL(obj) ( \
	G_TYPE_CHECK_INSTANCE_TYPE((obj), \
	GFRACT_TYPE_MANDEL))
#define GFRACT_IS_MANDEL_CLASS(obj) ( \
	G_TYPE_CHECK_CLASS_TYPE((obj), \
	GFRACT_TYPE_MANDEL))
#define GFRACT_MANDEL_GET_CLASS(obj) ( \
	G_TYPE_INSTANCE_GET_CLASS((obj), \
	GFRACT_TYPE_MANDEL, \
	GFractMandelClass))

typedef struct _GFractMandel GFractMandel;
typedef struct _GFractMandelClass GFractMandelClass;

struct _GFractMandel {
	GtkDrawingArea parent;
	GtkWidget *parent_widget;
};

struct _GFractMandelClass {
	GtkDrawingAreaClass parent_class;
};

GtkWidget *gfract_new_mandel(guint width, guint height);
GtkWidget *gfract_new_julia(guint width, guint height);

void gfract_set_limits(GtkWidget *widget,
		gdouble ulx, gdouble uly, gdouble lly);
void gfract_set_limits_default(GtkWidget *widget);
void gfract_set_limits_box(GtkWidget *widget,
		guint sx, guint sy, guint dx, guint dy);
void gfract_get_limits(GtkWidget *widget,
		gdouble *ulx, gdouble *uly, gdouble *lly);

void gfract_set_maxit(GtkWidget *widget, glong maxit);
guint gfract_get_maxit(GtkWidget *widget);

void gfract_draw_box(GtkWidget *widget,
		guint sx, guint sy, guint dx, guint dy);

void gfract_clean(GtkWidget *widget);

void gfract_clear_history(GtkWidget *widget);
void gfract_set_history(GtkWidget *widget, GSList *n);
GSList *gfract_get_history(GtkWidget *widget);

void gfract_compute(GtkWidget *widget);
void gfract_compute_partial(GtkWidget *widget);
void gfract_redraw(GtkWidget *widget);

gboolean gfract_select_get_active(GtkWidget *widget);
void gfract_select_set_active(GtkWidget *widget, gboolean active);

void gfract_orbits_set_active(GtkWidget *widget, gboolean active);
gboolean gfract_orbits_get_active(GtkWidget *widget);
void gfract_draw_orbit(GtkWidget *widget, long double x, long double y);
void gfract_draw_orbit_pixel(GtkWidget *widget, guint px, guint py);

void gfract_move_up(GtkWidget *widget, guint n);
void gfract_move_down(GtkWidget *widget, guint n);
void gfract_move_right(GtkWidget *widget, guint n);
void gfract_move_left(GtkWidget *widget, guint n);

GdkPixbuf *gfract_get_pixbuf(GtkWidget *widget);

void gfract_stop(GtkWidget *widget);
void gfract_stop_wait(GtkWidget *widget);

void
gfract_set_ratios(GtkWidget *widget, gfloat red, gfloat blue, gfloat green);
void
gfract_get_ratios(GtkWidget *widget, gfloat *red, gfloat *blue, gfloat *green);

void gfract_set_center(GtkWidget *widget, long double x, long double y);

void gfract_pixel_to_point(GtkWidget *widget,
		unsigned px, unsigned py,
		long double *x, long double *y);

void gfract_set_progress(GtkWidget *widget, GtkWidget *progress);

void gfract_set_progress_hook_start(GtkWidget *widget,
		void (*f)(gpointer), gpointer  data);
void gfract_set_progress_hook_finish(GtkWidget *widget,
		void (*f)(gpointer), gpointer  data);

G_END_DECLS

#endif
