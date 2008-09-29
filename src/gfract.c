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

#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <gtk/gtk.h>

#include "mandelbrot.h"
#include "julia.h"
#include "color_filter.h"
#include "mupoint.h"
#include "xfuncs.h"
#include "gfract.h"
#include "gfract_engines.h"

#define LIMITS_ULX_DEFAULT (-2.1)
#define LIMITS_ULY_DEFAULT (1.1)
#define LIMITS_LLY_DEFAULT (-1.1)

struct observer_state {
	double ulx;
	double uly;
	double lly;
};

enum gfract_type {
	GFRACT_MANDEL = 0,
	GFRACT_JULIA,
};

G_DEFINE_TYPE(GFractMandel, gfract_mandel, GTK_TYPE_DRAWING_AREA);

#define GFRACT_MANDEL_GET_PRIVATE(obj) ( \
	G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	GFRACT_TYPE_MANDEL, GFractMandelPrivate))

typedef struct _GFractMandelPrivate GFractMandelPrivate;

struct _GFractMandelPrivate {
	GdkPixmap *draw;
	GdkPixmap *onscreen;
	struct observer_state paint_limits;
	struct mupoint mupoint;
	struct {
		long double v;
		unsigned n;
	} avgfactor;
	GtkWidget *progress;
	float progress_stp;
	float progress_cur;
	void (*progress_hook_start)(gpointer);
	void (*progress_hook_finish)(gpointer);
	gpointer progress_hook_start_data;
	gpointer progress_hook_finish_data;
	bool do_select;
	bool do_orbits;
	bool do_energy;
	unsigned select_orig_x;
	unsigned select_orig_y;
	GSList *states;
	unsigned maxit;
	GThread *worker;
	bool stop_worker;
	struct {
		float red;
		float blue;
		float green;
	} ratios;
	unsigned width;
	unsigned height;
	enum gfract_type type;
	long double cx;
	long double cy;
};

static void gfract_mandel_finalize(GObject *object);
static gboolean gfract_expose(GtkWidget *widget, GdkEventExpose *event);
static gboolean
gfract_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean
gfract_button_release(GtkWidget *widget, GdkEventButton *event);
static gboolean gfract_motion(GtkWidget *widget, GdkEventMotion *event);
static gboolean configure_fract(GtkWidget *widget, GdkEventConfigure *event);
static gpointer run_worker(gpointer data);
static void do_mu(GtkWidget *widget, unsigned begin, size_t n);
static void draw(GtkWidget *widget);
static void doenergy(GtkWidget *widget);

static void progress_start(GtkWidget *widget, unsigned ticks);
static void progress_tick(GtkWidget *widget);
static void progress_finish(GtkWidget *widget);

static inline long double paint_inc(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return (priv->paint_limits.uly - priv->paint_limits.lly)
		/ (priv->height - 1);
}

void gfract_pixel_to_point(GtkWidget *widget,
		unsigned px, unsigned py,
		long double *x, long double *y)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	if (x)
		*x = px * paint_inc(widget) + priv->paint_limits.ulx;
	if (y)
		*y = -(py * paint_inc(widget) - priv->paint_limits.uly);
}

static inline void point_to_pixel(GtkWidget *widget,
		struct orbit_point *o, gint *x, gint *y)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	*x = (o->x - priv->paint_limits.ulx) / paint_inc(widget);
	*y = (priv->paint_limits.uly - o->y) / paint_inc(widget);
}

static void gfract_mandel_class_init(GFractMandelClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

	object_class->finalize = gfract_mandel_finalize;

	widget_class->expose_event = gfract_expose;
	widget_class->configure_event = configure_fract;
	widget_class->button_press_event = gfract_button_press;
	widget_class->button_release_event = gfract_button_release;
	widget_class->motion_notify_event = gfract_motion;

	g_type_class_add_private(object_class, sizeof(GFractMandelPrivate));
}

static void gfract_mandel_init(GFractMandel *fract)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(fract);

	fract->parent_widget = GTK_WIDGET(&fract->parent);

	priv->onscreen = NULL;
	priv->draw = NULL;

	priv->paint_limits.ulx = LIMITS_ULX_DEFAULT;
	priv->paint_limits.uly = LIMITS_ULY_DEFAULT;
	priv->paint_limits.lly = LIMITS_LLY_DEFAULT;

	priv->do_select = false;
	priv->do_orbits = false;
	priv->do_energy = false;

	priv->maxit = 1000;

	priv->states = NULL;

	priv->worker = NULL;
	priv->stop_worker = false;

	priv->ratios.red = priv->ratios.blue = priv->ratios.green = 0.5;

	priv->height = priv->width = 0;

	priv->cx = priv->cy = 0.0;

	priv->progress = NULL;
	priv->progress_stp = 0;
	priv->progress_cur = 0;
	priv->progress_hook_start = NULL;
	priv->progress_hook_finish = NULL;
	priv->progress_hook_start_data = NULL;
	priv->progress_hook_finish_data = NULL;

	gtk_widget_add_events(GTK_WIDGET(fract), 0
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK);
}

static void gfract_mandel_finalize(GObject *object)
{
	g_return_if_fail(GFRACT_IS_MANDEL(object));

	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(object);

	if (priv->onscreen) {
		g_object_unref(priv->onscreen);
		priv->onscreen = NULL;
	}

	if (priv->draw) {
		g_object_unref(priv->draw);
		priv->draw = NULL;
	}

	if (priv->states) {
		g_slist_foreach(priv->states, (GFunc)free, NULL);
		g_slist_free(priv->states);
		priv->states = NULL;
	}

	if (priv->worker) {
		g_thread_join(priv->worker);
		priv->worker = NULL;
	}

	mupoint_free(&priv->mupoint);

	if (G_OBJECT_CLASS(gfract_mandel_parent_class)->finalize)
		G_OBJECT_CLASS(gfract_mandel_parent_class)->finalize(object);
}

GtkWidget *gfract_new_mandel(guint width, guint height)
{
	GtkWidget *ret = g_object_new(GFRACT_TYPE_MANDEL, NULL);
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(ret);

	priv->type = GFRACT_MANDEL;

	priv->width = width;
	priv->height = height;

	gtk_widget_set_size_request(ret, width, height);

	return ret;
}

GtkWidget *gfract_new_julia(guint width, guint height)
{
	GtkWidget *ret = g_object_new(GFRACT_TYPE_MANDEL, NULL);
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(ret);

	priv->type = GFRACT_JULIA;

	priv->width = width;
	priv->height = height;

	gtk_widget_set_size_request(ret, width, height);

	return ret;
}

void gfract_compute(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	mupoint_clean(&priv->mupoint);
	gfract_compute_partial(widget);
}

void gfract_compute_partial(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->avgfactor.v = 0;
	priv->avgfactor.n = 0;
	gfract_redraw(widget);
}

void gfract_redraw(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	if (priv->worker)
		g_thread_join(priv->worker);
	if (priv->progress)
		priv->worker = g_thread_create(run_worker, widget, TRUE, NULL);
	else
		run_worker(widget);
}

static gboolean
gfract_button_press(GtkWidget *widget, GdkEventButton *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	if (priv->do_orbits)
		return FALSE;
	else if (priv->do_select && event->button != 1) {
		priv->do_select = false;
		gdk_window_invalidate_rect(widget->window, NULL, TRUE);
	} else if (!priv->do_select && event->button == 1) {
		priv->select_orig_x = event->x;
		priv->select_orig_y = event->y;
		priv->do_select = true;
	} else if (event->button == 3) {
		if (priv->states == NULL)
			return FALSE;
		struct observer_state *o = priv->states->data;
		memcpy(&priv->paint_limits, o, sizeof(*o));
		priv->states = g_slist_remove(priv->states, o);
		free(o);
		gfract_compute(widget);
	}

	return FALSE;
}

static gboolean
gfract_button_release(GtkWidget *widget, GdkEventButton *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	if (priv->do_orbits)
		return FALSE;
	else if (event->button != 1 || !priv->do_select)
		return FALSE;
	else if (priv->select_orig_y == event->y) {
		priv->do_select = false;
		gfract_clean(widget);
		return FALSE;
	}

	priv->do_select = false;

	struct observer_state *o = xmalloc(sizeof(*o));
	memcpy(o, &priv->paint_limits, sizeof(*o));
	priv->states = g_slist_prepend(priv->states, o);

	gfract_set_limits_box(widget,
			priv->select_orig_x, priv->select_orig_y,
			event->x, event->y);

	gfract_compute(widget);

	return FALSE;
}

static gboolean gfract_motion(GtkWidget *widget, GdkEventMotion *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	if (!priv->do_select && !priv->do_orbits)
		goto done;

	gfract_clean(widget);

	if (priv->do_select)
		gfract_draw_box(widget,
				priv->select_orig_x, priv->select_orig_y,
				event->x, event->y);
	else if (priv->do_orbits)
		gfract_draw_orbit_pixel(widget, event->x, event->y);

done:
	/* we are done so ask for more events */
	gdk_window_get_pointer(widget->window, NULL, NULL, NULL);
	return TRUE;
}

static void gfract_expose_rect(GtkWidget *widget, GdkRectangle rect)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	GFractMandel *fract = GFRACT_MANDEL(widget);

	gdk_draw_drawable(fract->parent_widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)], priv->onscreen,
			rect.x, rect.y, rect.x, rect.y, rect.width, rect.height);
}

void gfract_clean(GtkWidget *widget)
{
	GdkRectangle all = { .x = 0, .y = 0, .width = -1, .height = -1 };
	gfract_expose_rect(widget, all);
}

static gboolean gfract_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GdkRectangle *rect;
	int n_rects;
	gdk_region_get_rectangles(event->region, &rect, &n_rects);

	for (unsigned i = 0; i < n_rects; i++)
		gfract_expose_rect(widget, rect[i]);

	g_free(rect);

	return FALSE;
}

static gboolean configure_fract(GtkWidget *widget, GdkEventConfigure *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	GFractMandel *fract = GFRACT_MANDEL(widget);

	if (priv->worker)
		gfract_stop_wait(widget);
	if (priv->draw)
		g_object_unref(priv->draw);
	if (priv->onscreen)
		g_object_unref(priv->onscreen);
	priv->draw = gdk_pixmap_new(fract->parent_widget->window,
			priv->width, priv->height, -1);
	priv->onscreen = gdk_pixmap_new(fract->parent_widget->window,
			priv->width, priv->height, -1);
	gdk_draw_rectangle(priv->onscreen, widget->style->black_gc, TRUE, 0, 0,
			priv->width, priv->height);
	mupoint_create_as_needed(&priv->mupoint,
			priv->width, priv->height);

	g_object_ref_sink(priv->draw);
	g_object_ref_sink(priv->onscreen);

	gfract_compute(widget);

	return TRUE;
}

static gpointer run_worker(gpointer data)
{
	GtkWidget *widget = data;
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	priv->stop_worker = false;

	unsigned ticks = priv->width / 16 + priv->width / 128;

	if (priv->progress) {
		gdk_threads_enter();
		progress_start(widget, ticks);
		gdk_threads_leave();
	}

	do_mu(widget, 0, priv->width);

	if (priv->stop_worker)
		goto cleanup;

	if (priv->do_energy)
		doenergy(widget);
	priv->do_energy = false;

	if (priv->stop_worker)
		goto cleanup;

	draw(widget);

	if (priv->stop_worker)
		goto cleanup;

	void *aux = priv->onscreen;
	priv->onscreen = priv->draw;
	priv->draw = aux;

cleanup:
	if (priv->progress) {
		gdk_threads_enter();
		progress_finish(widget);
		gdk_threads_leave();
	}

	gdk_threads_enter();
	gdk_window_invalidate_rect(widget->window, NULL, TRUE);
	gdk_threads_leave();

	return data;
}

static void draw(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	struct mupoint *m = &priv->mupoint;
	unsigned width = priv->width;
	unsigned height = priv->height;

	unsigned ticked = 0;

	gdk_threads_enter();
	GdkGC *gc = gdk_gc_new(priv->draw);
	gdk_threads_leave();

	long double avg;
	if (priv->avgfactor.n > 0)
		avg = priv->avgfactor.v / priv->avgfactor.n;
	else
		avg = 1;
	long double energyfactor = do_energyfactor(avg, 0.2, 0.8) * 1000;

	for (unsigned i = 0; i < width; i++) {
		for (unsigned j = 0; j < height; j++) {
			long double factor = m->mu[i][j] * energyfactor;
			guint32 red = priv->ratios.red * factor;
			guint32 blue = priv->ratios.blue * factor;
			guint32 green = priv->ratios.green * factor;

			static const guint16 cmax = ~0;

			red = red > cmax ? cmax : red;
			blue = blue > cmax ? cmax : blue;
			green = green > cmax ? cmax : green;

			gdk_threads_enter();
			GdkColor color = { .red = red, .blue = blue, .green = green };
			gdk_gc_set_rgb_fg_color(gc, &color);
			gdk_draw_point(priv->draw, gc, i, j);
			gdk_threads_leave();
		}
		if (priv->progress && (ticked++ & 127) == 0) {
			if (priv->stop_worker)
				return;
			gdk_threads_enter();
			progress_tick(widget);
			gdk_threads_leave();
		}
	}

	gdk_threads_enter();
	g_object_unref(gc);
	gdk_threads_leave();
}

static void doenergy(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->avgfactor.v = 0L;
	priv->avgfactor.n = 0;
	unsigned width = priv->width;
	unsigned height = priv->height;
	for (unsigned i = 0; i < width; i++)
		for (unsigned j = 0; j < height; j++) {
			if (priv->mupoint.mu[i][j] == 0)
				continue;
			priv->avgfactor.v += priv->mupoint.mu[i][j];
			priv->avgfactor.n++;
		}
}

static void do_mu(GtkWidget *widget, unsigned begin, size_t n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	struct mupoint *m = &priv->mupoint;
	unsigned height = priv->height;
	long double x;
	long double y;
	long double acc = 0;
	unsigned nacc = 0;
	unsigned ticked = 0;

	x = priv->paint_limits.ulx + begin * paint_inc(widget);
	for (unsigned i = 0; i < n; i++) {
		y = priv->paint_limits.uly;
		for (unsigned j = 0; j < height; j++) {
			if (m->mu[i + begin][j] != -1L)
				goto inc_and_cont;

			long double modulus;
			unsigned it = priv->type == GFRACT_MANDEL
				? mandelbrot_it(priv->maxit, &x, &y, &modulus)
				: julia_it(priv->maxit, &x, &y, &priv->cx, &priv->cy, &modulus);

			/* Renormalized formula for the escape radius.
			 * Optimize away the case where it == 0
			 */
			if (it > 0) {
				long double mu = it - logl(fabsl(logl(modulus)));
				mu /= M_LN2;
				m->mu[i + begin][j] = mu < 0 ? 0 : mu;
				acc += mu;
				nacc++;
			} else
				m->mu[i + begin][j] = 0L;
inc_and_cont:
			y -= paint_inc(widget);
		}
		x += paint_inc(widget);
		if (priv->progress && (ticked++ & 15) == 0) {
			if (priv->stop_worker)
				return;
			gdk_threads_enter();
			progress_tick(widget);
			gdk_threads_leave();
		}
	}

	priv->avgfactor.v += acc;
	priv->avgfactor.n += nacc;
}

void gfract_clear_history(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	g_slist_foreach(priv->states, (GFunc)free, NULL);
	g_slist_free(priv->states);
	priv->states = NULL;
}

void gfract_set_history(GtkWidget *widget, GSList *n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	gfract_clear_history(widget);
	priv->states = g_slist_copy(n);
}

GSList *gfract_get_history(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return g_slist_copy(priv->states);
}

void gfract_get_limits(GtkWidget *widget,
		gdouble *ulx, gdouble *uly, gdouble *lly)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	if (ulx)
		*ulx = priv->paint_limits.ulx;
	if (uly)
		*uly = priv->paint_limits.uly;
	if (lly)
		*lly = priv->paint_limits.lly;
}

void gfract_set_limits_default(GtkWidget *widget)
{
	gfract_set_limits(widget,
			LIMITS_ULX_DEFAULT,
			LIMITS_ULY_DEFAULT,
			LIMITS_LLY_DEFAULT);
}

void gfract_set_limits(GtkWidget *widget,
		gdouble ulx, gdouble uly, gdouble lly)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->paint_limits.ulx = ulx;
	priv->paint_limits.uly = uly;
	priv->paint_limits.lly = lly;
}

static void box_limits(GtkWidget *widget,
		unsigned sx, unsigned sy, unsigned dx, unsigned dy,
		double *ulx, double *uly, double *lly)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	unsigned n_height = MAX(sy, dy) - MIN(sy, dy);
	unsigned n_width = priv->width * n_height / priv->height;
	if (dx < sx)
		n_width = -n_width;

	/* sx, sy, dx and dy are measured in pixels. thats why this
	 * check is _so_ anti-intuitive
	 */
	unsigned nuy = MIN(sy, dy);
	unsigned nly = MAX(sy, dy);
	unsigned nux = MIN(sx + n_width, sx);

	if (ulx)
		*ulx = nux * paint_inc(widget) + priv->paint_limits.ulx;
	if (lly)
		*lly = -(nly * paint_inc(widget) - priv->paint_limits.uly);
	if (uly)
		*uly = -(nuy * paint_inc(widget) - priv->paint_limits.uly);
}

void gfract_set_limits_box(GtkWidget *widget,
		guint sx, guint sy, guint dx, guint dy)
{
	double ulx;
	double uly;
	double lly;
	box_limits(widget, sx, sy, dx, dy, &ulx, &uly, &lly);
	gfract_set_limits(widget, ulx, uly, lly);
}

void gfract_draw_box(GtkWidget *widget,
		guint sx, guint sy, guint dx, guint dy)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	unsigned n_height = MAX(sy, dy) - MIN(sy, dy);
	unsigned n_width = priv->width * n_height / priv->height;

	if (dx < sx)
		n_width = -n_width;

	dx = sx + n_width;

	GdkGC *gc = widget->style->white_gc;
	gdk_draw_line(widget->window, gc, sx, sy, dx, sy);
	gdk_draw_line(widget->window, gc, sx, sy, sx, dy);
	gdk_draw_line(widget->window, gc, sx, dy, dx, dy);
	gdk_draw_line(widget->window, gc, dx, dy, dx, sy);
}

void gfract_orbits_set_active(GtkWidget *widget, gboolean active)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->do_orbits = active;
}

gboolean gfract_orbits_get_active(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return priv->do_orbits;
}

void gfract_draw_orbit(GtkWidget *widget, long double x, long double y)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	GdkGC *gc = gdk_gc_new(widget->window);
	static GdkColor colors[] = {
		{ .red = ~0, .green = 0, .blue = 0, },
		{ .red = 0, .green = ~0, .blue = 0, },
		{ .red = 0, .green = 0, .blue = ~0, },
		{ .red = ~0, .green = ~0, .blue = 0, },
		{ .red = ~0, .green = 0, .blue = ~0, },
		{ .red = 0, .green = ~0, .blue = ~0, },
		{ .red = ~0, .green = ~0, .blue = ~0, },
	};

	unsigned n;
	struct orbit_point *o = priv->type == GFRACT_MANDEL
		? mandelbrot_orbit(priv->maxit, &x, &y, &n)
		: julia_orbit(priv->maxit, &x, &y, &priv->cx, &priv->cy, &n);

	for (unsigned i = 0; i < n; i++) {
		gint sx;
		gint sy;
		gint dx;
		gint dy;
		point_to_pixel(widget, &o[i], &sx, &sy);
		if (i == 0) {
			struct orbit_point so = { .x = x, .y = y };
			point_to_pixel(widget, &so, &dx, &dy);
		} else
			point_to_pixel(widget, &o[i - 1], &dx, &dy);
		gdk_gc_set_rgb_fg_color(gc, &colors[i % G_N_ELEMENTS(colors)]);
		gdk_draw_line(widget->window, gc, sx, sy, dx, dy);
	}

	gdk_flush();
	g_object_unref(gc);
	free(o);
}

void gfract_draw_orbit_pixel(GtkWidget *widget, guint px, guint py)
{
	long double x;
	long double y;
	gfract_pixel_to_point(widget, px, py, &x, &y);
	gfract_draw_orbit(widget, x, y);
}

void gfract_move_up(GtkWidget *widget, guint n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->paint_limits.uly += n * paint_inc(widget);
	priv->paint_limits.lly += n * paint_inc(widget);
	while (n--)
		mupoint_move_up(&priv->mupoint);
	priv->do_energy = true;
}

void gfract_move_down(GtkWidget *widget, guint n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->paint_limits.uly -= n * paint_inc(widget);
	priv->paint_limits.lly -= n * paint_inc(widget);
	while (n--)
		mupoint_move_down(&priv->mupoint);
	priv->do_energy = true;
}

void gfract_move_right(GtkWidget *widget, guint n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->paint_limits.ulx += n * paint_inc(widget);
	while (n--)
		mupoint_move_right(&priv->mupoint);
	priv->do_energy = true;
}

void gfract_move_left(GtkWidget *widget, guint n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->paint_limits.ulx -= n *paint_inc(widget);
	while (n--)
		mupoint_move_left(&priv->mupoint);
	priv->do_energy = true;
}

GdkPixbuf *gfract_get_pixbuf(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return gdk_pixbuf_get_from_drawable(NULL,
			priv->onscreen, NULL, 0, 0, 0, 0, priv->width, priv->height);
}

void gfract_set_maxit(GtkWidget *widget, glong maxit)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	if (maxit <= 0)
		priv->maxit = 10;
	else if (maxit > UINT_MAX)
		priv->maxit = UINT_MAX;
	else
		priv->maxit = maxit;
}

guint gfract_get_maxit(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return priv->maxit;
}

gboolean gfract_select_get_active(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return priv->do_select;
}

void gfract_select_set_active(GtkWidget *widget, gboolean active)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->do_select = active;
}

void gfract_stop(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->stop_worker = true;

	if (priv->states) {
		struct observer_state *o = priv->states->data;
		memcpy(&priv->paint_limits, o, sizeof(*o));
		priv->states = g_slist_remove(priv->states, o);
		free(o);
	}
}

void gfract_stop_wait(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	gfract_stop(widget);
	if (priv->worker)
		g_thread_join(priv->worker);
	priv->worker = NULL;
}

void gfract_set_ratios(GtkWidget *widget, gfloat red, gfloat blue, gfloat green)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	if (red >= 0)
		priv->ratios.red = red;
	if (blue >= 0)
		priv->ratios.blue = blue;
	if (green >= 0)
		priv->ratios.green = green;
}

void
gfract_get_ratios(GtkWidget *widget, gfloat *red, gfloat *blue, gfloat *green)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	if (red)
		*red = priv->ratios.red;
	if (blue)
		*blue = priv->ratios.blue;
	if (green)
		*green = priv->ratios.green;
}

void gfract_set_center(GtkWidget *widget, long double x, long double y)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->cx = x;
	priv->cy = y;
}

void gfract_set_progress(GtkWidget *widget, GtkWidget *progress)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->progress = progress;
}

void
gfract_set_progress_hook_start(GtkWidget *widget,
		void (*f)(gpointer), gpointer  data)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->progress_hook_start = f;
	priv->progress_hook_start_data = data;
}

void
gfract_set_progress_hook_finish(GtkWidget *widget,
		void (*f)(gpointer), gpointer data)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->progress_hook_finish = f;
	priv->progress_hook_finish_data = data;
}

static void progress_start(GtkWidget *widget, unsigned ticks)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->progress_cur = 0;
	priv->progress_stp = 1.0L / ticks;
	gtk_widget_set_sensitive(widget, FALSE);
	if (priv->progress_hook_start)
		(*priv->progress_hook_start)(priv->progress_hook_start_data);
}

static void progress_tick(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->progress_cur += priv->progress_stp;
	if (priv->progress_cur > 1.0L)
		return;
	gchar *s = g_strdup_printf("Computing %u %%",
			(unsigned)(priv->progress_cur * 100));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(priv->progress), s);
	g_free(s);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->progress),
			priv->progress_cur);
}

static void progress_finish(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->progress_cur = 0;
	priv->progress_stp = 0;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(priv->progress), NULL);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->progress), 0.0L);
	gtk_widget_set_sensitive(widget, TRUE);
	if (priv->progress_hook_finish)
		(*priv->progress_hook_finish)(priv->progress_hook_finish_data);
}
