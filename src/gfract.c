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

#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <gtk/gtk.h>

#include "mandelbrot.h"
#include "paint.h"
#include "color_filter.h"
#include "color.h"
#include "mupoint.h"
#include "gui_progress.h"
#include "stack.h"
#include "xfuncs.h"
#include "gfract.h"

#define LIMITS_ULX_DEFAULT (-2.1)
#define LIMITS_ULY_DEFAULT (1.1)
#define LIMITS_LLY_DEFAULT (-1.1)

G_DEFINE_TYPE(GFractMandel, gfract_mandel, GTK_TYPE_DRAWING_AREA);

#define GFRACT_MANDEL_GET_PRIVATE(obj) ( \
	G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	GMANDEL_TYPE_FRACT, GFractMandelPrivate))

typedef struct _GFractMandelPrivate GFractMandelPrivate;

struct _GFractMandelPrivate {
	GtkWidget *win;
	GdkPixmap *draw;
	GdkPixmap *onscreen;
	struct observer_state paint_limits;
	struct mupoint mupoint;
	struct {
		long double v;
		unsigned n;
	} avgfactor;
	struct gui_progress *progress;
	bool do_select;
	bool do_orbits;
	unsigned select_orig_x;
	unsigned select_orig_y;
	stack *states;
};

static gboolean gfract_mandel_expose(GtkWidget *widget, GdkEventExpose *event);
static gboolean
gfract_mandel_button_press(GtkWidget *widget, GdkEventButton *event);
static gboolean
gfract_mandel_button_release(GtkWidget *widget, GdkEventButton *event);
static gboolean gfract_mandel_motion(GtkWidget *widget, GdkEventMotion *event);
static gboolean configure_fract(GtkWidget *widget, GdkEventConfigure *event);
static gpointer threaded_mandel(gpointer data);
static void mandel_do_mu(GtkWidget *widget, unsigned begin, size_t n);
static void mandel_draw(GtkWidget *widget);

static inline long double paint_inc(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return (priv->paint_limits.uly - priv->paint_limits.lly)
		/ (widget->allocation.height - 1);
}

static inline void pixel_to_point(GtkWidget *widget,
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
		struct orbit_point *o, unsigned *x, unsigned *y)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	unsigned tx = (o->x - priv->paint_limits.ulx) / paint_inc(widget);
	unsigned ty = (priv->paint_limits.uly - o->y) / paint_inc(widget);

#define SET_IN_BOUNDS(a, l, u) do { \
	if ((a) < (l)) \
		(a) = (l); \
	if ((a) > (u)) \
		(a) = (u); \
} while (0);

	SET_IN_BOUNDS(tx, 0, widget->allocation.width);
	SET_IN_BOUNDS(ty, 0, widget->allocation.height);

#undef SET_IN_BOUNDS

	*x = tx;
	*y = ty;
}

static void gfract_mandel_class_init(GFractMandelClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

	widget_class->expose_event = gfract_mandel_expose;
	widget_class->configure_event = configure_fract;
	widget_class->button_press_event = gfract_mandel_button_press;
	widget_class->button_release_event = gfract_mandel_button_release;
	widget_class->motion_notify_event = gfract_mandel_motion;

	g_type_class_add_private(object_class, sizeof(GFractMandelPrivate));
}

static void gfract_mandel_init(GFractMandel *fract)
{
	fract->parent = GTK_WIDGET(&fract->parent_widget);
}

GtkWidget *gfract_mandel_new(GtkWidget *win)
{
	GtkWidget *ret = g_object_new(GMANDEL_TYPE_FRACT, NULL);
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(ret);

	priv->onscreen = NULL;
	priv->draw = NULL;

	priv->win = win;

	priv->paint_limits.ulx = LIMITS_ULX_DEFAULT;
	priv->paint_limits.uly = LIMITS_ULY_DEFAULT;
	priv->paint_limits.lly = LIMITS_LLY_DEFAULT;

	priv->do_select = false;
	priv->do_orbits = false;

	priv->states = stack_alloc_init(free);

	gtk_widget_add_events(ret, 0
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK);

	return ret;
}

void gfract_mandel_compute(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	mupoint_clean(&priv->mupoint);
	g_thread_create(threaded_mandel, widget, FALSE, NULL);
}

static gboolean
gfract_mandel_button_press(GtkWidget *widget, GdkEventButton *event)
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
		if (stack_empty(priv->states))
			return FALSE;
		struct observer_state *o = stack_pop(priv->states);
		memcpy(&priv->paint_limits, o, sizeof(*o));
		priv->states->destroy(o);
		gfract_mandel_compute(widget);
	}

	return FALSE;
}

static gboolean
gfract_mandel_button_release(GtkWidget *widget, GdkEventButton *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	if (priv->do_orbits)
		return FALSE;
	else if (event->button != 1 || !priv->do_select)
		return FALSE;
	else if (priv->select_orig_y == event->y) {
		priv->do_select = false;
		gfract_mandel_clean(widget);
		return FALSE;
	}

	priv->do_select = false;

	struct observer_state *o = xmalloc(sizeof(*o));
	memcpy(o, &priv->paint_limits, sizeof(*o));
	stack_push(priv->states, o);

	gfract_mandel_set_limits_box(widget,
			priv->select_orig_x, priv->select_orig_y,
			event->x, event->y);

	gfract_mandel_compute(widget);

	return FALSE;
}

static gboolean gfract_mandel_motion(GtkWidget *widget, GdkEventMotion *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	if (!priv->do_select && !priv->do_orbits)
		return FALSE;

	gfract_mandel_clean(widget);

	if (priv->do_select)
		gfract_mandel_draw_box(widget,
				priv->select_orig_x, priv->select_orig_y,
				event->x, event->y);
	else if (priv->do_orbits)
		gfract_mandel_draw_orbit_pixel(widget, event->x, event->y);

	/* we are done so ask for more events */
	gdk_window_get_pointer(widget->window, NULL, NULL, NULL);
	return FALSE;
}

static void gfract_mandel_expose_rect(GtkWidget *widget, GdkRectangle rect)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	GFractMandel *fract = GFRACT_MANDEL(widget);

	gdk_draw_drawable(fract->parent->window,
			widget->style->fg_gc[GTK_WIDGET_STATE(widget)], priv->onscreen,
			rect.x, rect.y, rect.x, rect.y, rect.width, rect.height);
}

void gfract_mandel_clean(GtkWidget *widget)
{
	GdkRectangle all = { .x = 0, .y = 0, .width = -1, .height = -1 };
	gfract_mandel_expose_rect(widget, all);
}

static gboolean gfract_mandel_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GdkRectangle *rect;
	int n_rects;
	gdk_region_get_rectangles(event->region, &rect, &n_rects);

	for (unsigned i = 0; i < n_rects; i++)
		gfract_mandel_expose_rect(widget, rect[i]);

	g_free(rect);

	return FALSE;
}

static gboolean configure_fract(GtkWidget *widget, GdkEventConfigure *event)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	GFractMandel *fract = GFRACT_MANDEL(widget);

	if (priv->draw)
		g_object_unref(priv->draw);
	if (priv->onscreen)
		g_object_unref(priv->onscreen);
	priv->draw = gdk_pixmap_new(fract->parent->window,
			widget->allocation.width, widget->allocation.height, -1);
	priv->onscreen = gdk_pixmap_new(fract->parent->window,
			widget->allocation.width, widget->allocation.height, -1);
	gdk_draw_rectangle(priv->onscreen, widget->style->black_gc, TRUE, 0, 0,
			widget->allocation.width, widget->allocation.height);
	mupoint_create_as_needed(&priv->mupoint,
			widget->allocation.width, widget->allocation.height);

	gfract_mandel_compute(widget);

	return TRUE;
}

static gpointer threaded_mandel(gpointer data)
{
	gdk_threads_enter();

	GtkWidget *widget = data;
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);

	priv->avgfactor.v = 0;
	priv->avgfactor.n = 0;

	unsigned width = widget->allocation.width;
	unsigned ticks = width / 16 + width / 128;
	priv->progress = gui_progress_with_parent(priv->win, ticks);

	mandel_do_mu(widget, 0, widget->allocation.width);
	mandel_draw(widget);

	void *aux = priv->onscreen;
	priv->onscreen = priv->draw;
	priv->draw = aux;

	gui_progress_end(priv->progress);

	gdk_window_invalidate_rect(widget->window, NULL, TRUE);

	gdk_threads_leave();

	return data;
}

static void mandel_draw(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	struct mupoint *m = &priv->mupoint;
	unsigned width = widget->allocation.width;
	unsigned height = widget->allocation.height;

	unsigned ticked = 0;

	GdkGC *gc = gdk_gc_new(priv->draw);

	long double avg;
	if (priv->avgfactor.n > 0)
		avg = priv->avgfactor.v / priv->avgfactor.n;
	else
		avg = 1;
	long double energyfactor = do_energyfactor(avg, 0.2, 0.8) * 1000;

	for (unsigned i = 0; i < width; i++) {
		for (unsigned j = 0; j < height; j++) {
			long double factor = m->mu[i][j] * energyfactor;
			guint32 red = color_get_current()->red * factor;
			guint32 blue = color_get_current()->blue * factor;
			guint32 green = color_get_current()->green * factor;

			static const guint16 cmax = ~0;

			red = red > cmax ? cmax : red;
			blue = blue > cmax ? cmax : blue;
			green = green > cmax ? cmax : green;

			GdkColor color = { .red = red, .blue = blue, .green = green };
			gdk_gc_set_rgb_fg_color(gc, &color);
			gdk_draw_point(priv->draw, gc, i, j);
		}
		if ((ticked++ & 127) == 0)
			gui_progress_tick(priv->progress);
	}

	g_object_unref(gc);
}

static void mandel_do_mu(GtkWidget *widget, unsigned begin, size_t n)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	struct mupoint *m = &priv->mupoint;
	unsigned height = widget->allocation.height;
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
			unsigned it = mandelbrot_it(&x, &y, &modulus);

			/* Renormalized formula for the escape radius.
			 * Optimize away the case where it == 0
			 */
			if (it > 0) {
				long double mu = it - logl(fabsl(logl(modulus)));
				mu /= M_LN2;
				m->mu[i + begin][j] = mu;
				acc += mu;
				nacc++;
			} else
				m->mu[i + begin][j] = 0L;
inc_and_cont:
			y -= paint_inc(widget);
		}
		x += paint_inc(widget);
		if ((ticked++ & 15) == 0)
			gui_progress_tick(priv->progress);
	}

	priv->avgfactor.v += acc;
	priv->avgfactor.n += nacc;
}

void gfract_mandel_history_clear(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	while (!stack_empty(priv->states))
		priv->states->destroy(stack_pop(priv->states));
}

void gfract_mandel_set_limits_default(GtkWidget *widget)
{
	gfract_mandel_set_limits(widget,
			LIMITS_ULX_DEFAULT,
			LIMITS_ULY_DEFAULT,
			LIMITS_LLY_DEFAULT);
}

void gfract_mandel_set_limits(GtkWidget *widget,
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
	unsigned n_width =
		widget->allocation.width * n_height / widget->allocation.height;
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

void gfract_mandel_set_limits_box(GtkWidget *widget,
		guint sx, guint sy, guint dx, guint dy)
{
	double ulx;
	double uly;
	double lly;
	box_limits(widget, sx, sy, dx, dy, &ulx, &uly, &lly);
	gfract_mandel_set_limits(widget, ulx, uly, lly);
}

void gfract_mandel_draw_box(GtkWidget *widget,
		guint sx, guint sy, guint dx, guint dy)
{
	unsigned n_height = MAX(sy, dy) - MIN(sy, dy);
	unsigned n_width = widget->allocation.width * n_height
		/ widget->allocation.height;

	if (dx < sx)
		n_width = -n_width;

	dx = sx + n_width;

	GdkGC *gc = widget->style->white_gc;
	gdk_draw_line(widget->window, gc, sx, sy, dx, sy);
	gdk_draw_line(widget->window, gc, sx, sy, sx, dy);
	gdk_draw_line(widget->window, gc, sx, dy, dx, dy);
	gdk_draw_line(widget->window, gc, dx, dy, dx, sy);
}

void gfract_mandel_orbits_set_active(GtkWidget *widget, gboolean active)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	priv->do_orbits = active;
}

gboolean gfract_mandel_orbits_get_active(GtkWidget *widget)
{
	GFractMandelPrivate *priv = GFRACT_MANDEL_GET_PRIVATE(widget);
	return priv->do_orbits;
}

void gfract_mandel_draw_orbit(GtkWidget *widget, long double x, long double y)
{
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
	struct orbit_point *o = mandelbrot_orbit(&x, &y, &n);

	for (unsigned i = 0; i < n; i++) {
		unsigned sx;
		unsigned sy;
		unsigned dx;
		unsigned dy;
		point_to_pixel(widget, &o[i], &sx, &sy);
		if (i == 0) {
			struct orbit_point so = { .x = x, .y = y };
			point_to_pixel(widget, &so, &dx, &dy);
		} else
			point_to_pixel(widget, &o[i - 1], &dx, &dy);
		gdk_gc_set_rgb_fg_color(gc, &colors[i % G_N_ELEMENTS(colors)]);
		gdk_draw_line(widget->window, gc,
				sx, sy, dx, dy);
	}

	g_object_unref(gc);
	free(o);
}

void gfract_mandel_draw_orbit_pixel(GtkWidget *widget, guint px, guint py)
{
	long double x;
	long double y;
	pixel_to_point(widget, px, py, &x, &y);
	gfract_mandel_draw_orbit(widget, x, y);
}
