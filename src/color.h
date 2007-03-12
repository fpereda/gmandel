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

#if 1 /* test */
#   define COLOR_RATIO_BLUE  (50)
#   define COLOR_RATIO_GREEN (8)
#   define COLOR_RATIO_RED   (0)
#endif
#if 0 /* blueish */
#   define COLOR_RATIO_BLUE  (1500)
#   define COLOR_RATIO_GREEN (800)
#   define COLOR_RATIO_RED   (150)
#endif
#if 0 /* greenish */
#   define COLOR_RATIO_BLUE  (800)
#   define COLOR_RATIO_GREEN (1500)
#   define COLOR_RATIO_RED   (150)
#endif
#if 0 /* fireish */
#   define COLOR_RATIO_BLUE  (10)
#   define COLOR_RATIO_GREEN (100)
#   define COLOR_RATIO_RED   (1600)
#endif
#if 0 /* greyish */
#   define COLOR_RATIO_BLUE  (2000)
#   define COLOR_RATIO_GREEN (2000)
#   define COLOR_RATIO_RED   (2000)
#endif
#if 0 /* yellowish */
#   define COLOR_RATIO_BLUE  (100)
#   define COLOR_RATIO_GREEN (2000)
#   define COLOR_RATIO_RED   (2000)
#endif
#if 0 /* purplish */
#   define COLOR_RATIO_BLUE  (2000)
#   define COLOR_RATIO_GREEN (100)
#   define COLOR_RATIO_RED   (2000)
#endif
#if 0 /* brownish / orangish / weird */
#   define COLOR_RATIO_BLUE  (300)
#   define COLOR_RATIO_GREEN (800)
#   define COLOR_RATIO_RED   (2000)
#endif

static struct color_ratios_ {
	unsigned red;
	unsigned blue;
	unsigned green;
} color_ratio = {
	.red = COLOR_RATIO_RED,
	.blue = COLOR_RATIO_BLUE,
	.green = COLOR_RATIO_GREEN,
};
