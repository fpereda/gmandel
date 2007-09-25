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

#ifndef GMANDEL_GFRACT_H_
#define GMANDEL_GFRACT_H_ 1

G_BEGIN_DECLS

#define GMANDEL_TYPE_FRACT (gfract_mandel_get_type())
#define GMANDEL_FRACT(obj) ( \
	G_TYPE_CHECK_INSTANCE_CAST((obj), \
	GMANDEL_TYPE_FRACT, \
	GFractMandel))
#define GMANDEL_FRACT_CLASS(obj) ( \
	G_TYPE_CHECK_CLASS_CAST((obj), \
	GMANDEL_FRACT, \
	GFractMandelClass))
#define GMANDEL_IS_FRACT(obj) ( \
	G_TYPE_CHECK_INSTANCE_TYPE((obj), \
	GMANDEL_TYPE_FRACT))
#define GMANDEL_IS_FRACT_CLASS(obj) ( \
	G_TYPE_CHECK_CLASS_TYPE((obj), \
	GMANDEL_TYPE_FRACT))
#define GMANDEL_FRACT_GET_CLASS(obj) ( \
	G_TYPE_INSTANCE_GET_CLASS((obj), \
	GMANDEL_TYPE_FRACT, \
	GFractMandelClass))

typedef struct _GFractMandel GFractMandel;
typedef struct _GFractMandelClass GFractMandelClass;

struct _GFractMandel {
	GtkDrawingArea parent_widget;
	GtkWidget *parent;
};

struct _GFractMandelClass {
	GtkDrawingAreaClass parent_class;
};

GtkWidget *gfract_mandel_new(GtkWidget *win);

void gfract_mandel_set_limits(GtkWidget *widget,
		double ulx, double uly, double lly);
void gfract_mandel_set_limits_default(GtkWidget *widget);
void gfract_mandel_set_limits_box(GtkWidget *widget,
		unsigned sx, unsigned sy, unsigned dx, unsigned dy);

void gfract_mandel_draw_box(GtkWidget *widget,
		unsigned sx, unsigned sy, unsigned dx, unsigned dy);

void gfract_mandel_clean(GtkWidget *widget);

void gfract_mandel_history_clear(GtkWidget *widget);

void gfract_mandel_compute(GtkWidget *widget);

G_END_DECLS

#endif
