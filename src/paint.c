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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "paint.h"
#include "mandelbrot.h"
#include "mupoint.h"
#include "color.h"
#include "xfuncs.h"
#include "gui_status.h"
#include "gui_progress.h"

static void recalculate_average_energy(void)
{
	gmandel_mu_t **mupoint = mupoint_get_mupoint();
	avgfactor.v = 0L;
	avgfactor.n = 0;
	for (unsigned i = 0; i < window_size.width; i++)
		for (unsigned j = 0; j < window_size.height; j++) {
			if (mupoint[i][j] == 0)
				continue;
			avgfactor.v += mupoint[i][j];
			avgfactor.n++;
		}
}

void paint_box_limits(
		unsigned sx, unsigned sy, unsigned dx, unsigned dy,
		double *ulx, double *uly, double *lly)
{
	unsigned n_height = MAX(sy, dy) - MIN(sy, dy);
	unsigned n_width = window_size.width * n_height / window_size.height;
	if (dx < sx)
		n_width = -n_width;

	/* sx, sy, dx and dy are measured in pixels. thats why this
	 * check is _so_ anti-intuitive
	 */
	unsigned nuy = MIN(sy, dy);
	unsigned nly = MAX(sy, dy);
	unsigned nux = MIN(sx + n_width, sx);

	if (ulx)
		*ulx = nux * paint_inc() + paint_limits.ulx;
	if (lly)
		*lly = -(nly * paint_inc() - paint_limits.uly);
	if (uly)
		*uly = -(nuy * paint_inc() - paint_limits.uly);
}
