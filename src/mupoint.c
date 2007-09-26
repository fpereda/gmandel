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

void mupoint_free(struct mupoint *m)
{
	for (unsigned i = 0; i < m->width; i++)
		free(m->mu[i]);
	free(m->mu);
	m->mu = NULL;
}

void mupoint_clean_col(struct mupoint *m, unsigned i)
{
	for (unsigned j = 0; j < m->height; j++)
		m->mu[i][j] = -1L;
}

void mupoint_clean(struct mupoint *m)
{
	for (unsigned i = 0; i < m->width; i++)
		mupoint_clean_col(m, i);
}

void mupoint_create_as_needed(struct mupoint *m, unsigned w, unsigned h)
{
	if (!m->mu) {
		m->width = w;
		m->height = h;
		m->mu = xmalloc(m->width * sizeof(*m->mu));
		for (unsigned i = 0; i < m->width; i++)
			m->mu[i] = xmalloc(m->height * sizeof(**m->mu));
		mupoint_clean(m);
	} else if (w > m->width || h > m->height) {
		unsigned max_w = w > m->width ? w : m->width;
		if (w > m->width)
			m->mu = xrealloc(m->mu, w * sizeof(*m->mu));
		if (h > m->height)
			for (unsigned i = 0; i < max_w; i++)
				m->mu[i] = i < m->width
					? xrealloc(m->mu[i], h * sizeof(**m->mu))
					: xmalloc(h * sizeof(**m->mu));
		else
			for (unsigned i = m->width; i < w; i++)
				m->mu[i] = xmalloc(m->height * sizeof(**m->mu));
		m->width = w;
		m->height = h;
		mupoint_clean(m);
	}
}

void mupoint_move_up(struct mupoint *m)
{
	size_t num = (m->height - 1) * sizeof(**m->mu);
	for (unsigned i = 0; i < m->width; i++) {
		memmove(&m->mu[i][1], &m->mu[i][0], num);
		m->mu[i][0] = -1L;
	}
}

void mupoint_move_down(struct mupoint *m)
{
	size_t num = (m->height - 1) * sizeof(**m->mu);
	for (unsigned i = 0; i < m->width; i++) {
		memmove(&m->mu[i][0], &m->mu[i][1], num);
		m->mu[i][m->height - 1] = -1L;
	}
}

void mupoint_move_right(struct mupoint *m)
{
	size_t num = (m->width - 1) * sizeof(*m->mu);
	void *p = m->mu[0];
	memmove(&m->mu[0], &m->mu[1], num);
	m->mu[m->width - 1] = p;
	mupoint_clean_col(m, m->width - 1);
}

void mupoint_move_left(struct mupoint *m)
{
	size_t num = (m->width - 1) * sizeof(*m->mu);
	void *p = m->mu[m->width - 1];
	memmove(&m->mu[1], &m->mu[0], num);
	m->mu[0] = p;
	mupoint_clean_col(m, 0);
}
