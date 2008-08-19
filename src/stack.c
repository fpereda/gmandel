/* vim: set sts=4 sw=4 noet : */

/*
 * Copyright (c) 2007, Fernando J. Pereda <ferdy@ferdyx.org>
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

#include "stack.h"
#include "xfuncs.h"

stack *stack_alloc_init(void (*destroy)(void *))
{
	stack *s = xmalloc(sizeof(*s));
	stack_init(s, destroy);
	return s;
}

void stack_init(stack *s, void (*destroy)(void *))
{
	s->first = NULL;
	s->size = 0;
	s->destroy = destroy;
}

void stack_destroy(stack *s)
{
	stacknode *n = s->first;
	while (n) {
		stacknode *nn = n->next;
		if (s->destroy)
			s->destroy(n->data);
		free(n);
		n = nn;
	}
	free(s);
}

void stack_push(stack *s, void *d)
{
	stacknode *n = xmalloc(sizeof(*n));
	n->data = d;
	n->next = s->first;
	s->first = n;
	s->size++;
}

void *stack_pop(stack *s)
{
	if (stack_empty(s))
		return NULL;
	stacknode *n = s->first;
	s->first = n->next;
	s->size--;
	void *p = n->data;
	free(n);
	return p;
}
