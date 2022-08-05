/*
 * Copyright (C) 2015-2022 IoT.bzh Company
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

#pragma once

#include <stdlib.h>
#include <stdint.h>

/*
 * Returns a 8 bits hash value for the 'text'.
 *
 * Tiny hash function inspired from pearson
 * 
 * @param text the text to hash
 * @return a 8 bits hash value
 */
extern uint8_t rp_pearson8(const char *text);

/*
 * Returns a 8 bits hash value for the 'text' of 'length'.
 *
 * Tiny hash function inspired from pearson
 * 
 * @param text the text to hash
 * @param length length of the text to hash
 * @return a 8 bits hash value
 */
extern uint8_t rp_pearson8_len(const char *text, size_t length);

/* defines sub length pearson using inlines */

#define _DEF_RP_PEARSON_(N) \
		inline uint8_t rp_pearson##N(const char *text) {\
			uint8_t h = rp_pearson8(text);\
			return ((h ^ (h >> (8 - (N)))) & (255 >> (8 - (N)))); }

#define _DEF_RP_PEARSON_LEN_(N) \
		inline uint8_t rp_pearson##N##_len(const char *text, size_t length) {\
			uint8_t h = rp_pearson8_len(text, length);\
			return ((h ^ (h >> (8 - (N)))) & (255 >> (8 - (N)))); }

_DEF_RP_PEARSON_(3)
_DEF_RP_PEARSON_LEN_(3)
_DEF_RP_PEARSON_(4)
_DEF_RP_PEARSON_LEN_(4)
_DEF_RP_PEARSON_(5)
_DEF_RP_PEARSON_LEN_(5)
_DEF_RP_PEARSON_(6)
_DEF_RP_PEARSON_LEN_(6)
_DEF_RP_PEARSON_(7)
_DEF_RP_PEARSON_LEN_(7)
