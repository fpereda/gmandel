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
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "parallel_paint.h"
#include "paint.h"

struct paint_do_mu_args {
	unsigned begin;
	size_t n;
	double inc;
};

static void *adapt_paint_do_mu(void *arg)
{
	struct paint_do_mu_args *a = arg;
	paint_do_mu(a->begin, a->n, a->inc);
	return NULL;
}

void parallel_paint_do_mu(size_t n, double inc)
{
	pthread_t threads[N_THREADS];
	struct paint_do_mu_args args[N_THREADS];

	unsigned each_n = n / N_THREADS;
	unsigned last_n = n % N_THREADS;
	if (last_n == 0)
		last_n = each_n;

	for (unsigned i = 0; i < N_THREADS; i++) {
		args[i].begin = i * each_n;
		args[i].n = (i == N_THREADS - 1) ? last_n : each_n;
		args[i].inc = inc;
		int ret = pthread_create(&threads[i], NULL, adapt_paint_do_mu, &args[i]);
		if (ret != 0) {
			fprintf(stderr, "pthread_create failed: '%s'\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
	}

	for (unsigned i = 0; i < N_THREADS; i++)
		pthread_join(threads[i], NULL);
}
