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

#include <limits.h>
#include <math.h>

#include "mandelbrot.h"

static unsigned maxit = 1000;

void mandelbrot_set_maxit(long n)
{
	if (n <= 0)
		maxit = 10;
	else if (n > UINT_MAX)
		maxit = UINT_MAX;
	else
		maxit = n;
}

unsigned mandelbrot_get_maxit(void)
{
	return maxit;
}

unsigned mandelbrot_it(
		long double *xc, long double *yc,
		long double *modulus)
{
	unsigned it = 1;

	long double x;
	long double y;
	long double x0;
	long double y0;
	long double x2;
	long double y2;

	x = x0 = *xc;
	y = y0 = *yc;

	x2 = x * x;
	y2 = y * y;

	while ((x2 + y2) < 4 && it++ < maxit) {
		y = 2 * x * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
	}

	if (it >= maxit || it == 0)
		return 0;

	/*
	 * When using the renormalized formula for the escape radius,
	 * a couple of additional iterations help reducing the size
	 * of the error term.
	 */
	unsigned n = 2;
	while (n--) {
		y = 2 * x * y + y0;
		x = x2 - y2 + x0;
		x2 = x * x;
		y2 = y * y;
	}

	if (modulus)
		*modulus = sqrtl(x2 + y2);

	return it;
}