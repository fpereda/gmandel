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
 *     * Neither the name of the library nor the names of its
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

#ifndef GMANDEL_XFUNCS_H_
#define GMANDEL_XFUNCS_H_ 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__GNUC__)
#    define GMANDEL_ATTRIBUTE(x) __attribute__((x))
#else
#    define GMANDEL_ATTRIBUTE(x)
#endif

#if defined(__GNUC__)
#    define likely(a)   __builtin_expect((a), 1)
#    define unlikely(a) __builtin_expect((a), 0)
#else
#    define likely(a)   (a)
#    define unlikely(a) (a)
#endif

static GMANDEL_ATTRIBUTE(noreturn) void oom(const char *s)
{
	fprintf(stderr, "Oom. %s\n", s);
	exit(100);
}

static inline void *xmalloc(size_t s)
{
	void *p = malloc(s ? s : 1);
	if (unlikely(!p))
		oom("malloc failed.");
	return p;
}

static inline void *xrealloc(void *p, size_t s)
{
	void *ret = realloc(p, s ? s : 1);
	if (unlikely(!ret))
		oom("realloc failed.");
	return ret;
}

static inline char *xstrdup(const char *c)
{
	char *p = strdup(c);
	if (unlikely(!p))
		oom("strdup failed");
	return p;
}

#endif
