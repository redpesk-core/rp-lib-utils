/*
 * Copyright (C) 2015-2023 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>

#include "rp-pearson.h"

static uint8_t TP4[16] = {
	4,  6,  1,  10,  9, 14, 11,  5,
	3,  2, 12, 15, 0,  7,  8, 13
};

/*
 * Returns a 8 bits hash value for the 'text'.
 *
 * Tiny hash function inspired from pearson
 * 
 * @param text the text to hash
 * @return a 8 bits hash value
 */
uint8_t rp_pearson8(const char *text)
{
	const char *iter = text;
	uint8_t rl = 0, ru = 15, c = (uint8_t)*iter;
	while(c) {
		rl = TP4[rl ^ (15 & c)];
		ru = TP4[ru ^ (c >> 4)];
		c = (uint8_t)*++iter;
	}
	return ((ru << 4) | rl) ^ ((uint8_t)(ptrdiff_t)(iter - text));
}

/*
 * Returns a 8 bits hash value for the 'text' of 'length'.
 *
 * Tiny hash function inspired from pearson
 * 
 * @param text the text to hash
 * @param length length of the text to hash
 * @return a 8 bits hash value
 */
uint8_t rp_pearson8_len(const char *text, size_t length)
{
	size_t pos;
	uint8_t c, rl = 0, ru = 15;
	for (pos = 0 ; pos != length ; pos++) {
		c = (uint8_t)text[pos];
		rl = TP4[rl ^ (15 & c)];
		ru = TP4[ru ^ (c >> 4)];
	}
	return ((ru << 4) | rl) ^ ((uint8_t)length);
}
