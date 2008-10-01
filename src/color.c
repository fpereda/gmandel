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

#include "color.h"

static char *color_names[] = {
	[COLOR_THEME_ICEBLUE] = "iceblue",
	[COLOR_THEME_FIRERED] = "firered",
	[COLOR_THEME_GREENPARK] = "greenpark",
	[COLOR_THEME_GREYSCALE] = "greyscale",
	[COLOR_THEME_DEEPBLUE] = "deepblue",
	[COLOR_THEME_VERYGREEN] = "verygreen",
	[COLOR_THEME_LAST] = NULL,
};

static const struct color_ratios color_ratios[] = {
	[COLOR_THEME_ICEBLUE] = { .red = 0.1, .blue = 1, .green = 0.6 },
	[COLOR_THEME_FIRERED] = { .red = 1, .blue = 0.1, .green = 0.3 },
	[COLOR_THEME_GREENPARK] = { .red = 0.1, .blue = 0.4, .green = 1 },
	[COLOR_THEME_GREYSCALE] = { .red = 0.7, .blue = 0.7, .green = 0.7 },
	[COLOR_THEME_DEEPBLUE] = { .red = 0.1, .blue = 1, .green = 0.3 },
	[COLOR_THEME_VERYGREEN] = { .red = 0.01, .blue = 0.01, .green = 0.3 },
};

char **color_get_names(void)
{
	return color_names;
}

const struct color_ratios *color_get(enum COLOR_THEMES c)
{
	if (c >= 0 && c < COLOR_THEME_LAST)
		return &color_ratios[c];
	else
		return NULL;
}
