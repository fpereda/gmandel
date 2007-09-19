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

#include "mupoint.h"
#include "xfuncs.h"

static gmandel_mu_t **mupoint;
static unsigned width;
static unsigned height;

gmandel_mu_t **mupoint_get_mupoint(void)
{
	return mupoint;
}

void mupoint_set_mupoint(gmandel_mu_t **m)
{
	mupoint = m;
}

void mupoint_set_size(unsigned w, unsigned h)
{
	width = w;
	height = h;
}

void mupoint_get_size(unsigned *w, unsigned *h)
{
	if (w)
		*w = width;
	if (h)
		*h = height;
}

void mupoint_clean_col(unsigned i)
{
	for (unsigned j = 0; j < height; j++)
		mupoint[i][j] = -1L;
}

void mupoint_clean(void)
{
	for (unsigned i = 0; i < width; i++)
		mupoint_clean_col(i);
}

void mupoint_create_as_needed(unsigned w, unsigned h)
{
	if (!mupoint) {
		mupoint_set_size(w, h);
		mupoint = xmalloc(width * sizeof(*mupoint));
		for (unsigned i = 0; i < width; i++)
			mupoint[i] = xmalloc(height * sizeof(**mupoint));
		mupoint_clean();
	}
}

void mupoint_move_up(void)
{
	size_t num = (height - 1) * sizeof(**mupoint);
	for (unsigned i = 0; i < width; i++) {
		memmove(&mupoint[i][1], &mupoint[i][0], num);
		mupoint[i][0] = -1L;
	}
}

void mupoint_move_down(void)
{
	size_t num = (height - 1) * sizeof(**mupoint);
	for (unsigned i = 0; i < width; i++) {
		memmove(&mupoint[i][0], &mupoint[i][1], num);
		mupoint[i][height - 1] = -1L;
	}
}

void mupoint_move_right(void)
{
	size_t num = (width - 1) * sizeof(*mupoint);
	void *p = mupoint[0];
	memmove(&mupoint[0], &mupoint[1], num);
	mupoint[width - 1] = p;
	mupoint_clean_col(width - 1);
}

void mupoint_move_left(void)
{
	size_t num = (width - 1) * sizeof(*mupoint);
	void *p = mupoint[width - 1];
	memmove(&mupoint[1], &mupoint[0], num);
	mupoint[0] = p;
	mupoint_clean_col(0);
}
