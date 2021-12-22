/*
 * Copyright (C) 2015-2021 IoT.bzh Company
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

#include <ctype.h>

#include "globmatch.h"

#define TOLOWER(ch)        (tolower((int)(unsigned char)(ch)))
#define EQ(flags,cha,chb)  ((flags & FNM_CASEFOLD) ? \
				(TOLOWER(cha) == TOLOWER(chb)) : (cha == chb))

/**
 * Matches whether the string 'str' matches the pattern 'pat'
 * and returns its matching score.
 *
 * @param pat the glob pattern
 * @param str the string to match
 * @return 0 if no match or number representing the matching score
 */
static unsigned match(const char *pat, const char *str, int flags)
{
	unsigned r, rs, rr;
	char c, x;

	/* scan the prefix without glob */
	r = 1;
	while ((c = *pat++) != GLOB) {
		x = *str++;
		if (!EQ(flags,c,x))
			return 0; /* no match */
		if (!c)
			return r; /* match up to end */
		r++;
	}

	/* glob found */
	c = *pat++;
	if (!c) {
		/* not followed by pattern */
		if (flags & FNM_PATHNAME) {
			while(*str)
				if (*str++ == '/')
					return 0;
		}
		return r;
	}

	/* evaluate the best score for following pattern */
	rs = 0;
	while (*str) {
		x = *str++;
		if (EQ(flags,c,x)) {
			/* first char matches, check remaining string */
			rr = match(pat, str, flags);
			if (rr > rs)
				rs = rr;
		} else if ((flags & FNM_PATHNAME) && x == '/')
			return 0;
	}

	/* best score or not match if rs == 0 */
	return rs ? rs + r : 0;
}

/**
 * Matches whether the string 'str' matches the pattern 'pat'
 * and returns its matching score.
 *
 * @param pat the glob pattern
 * @param str the string to match
 * @return 0 if no match or number representing the matching score
 */
unsigned globmatch(const char *pat, const char *str)
{
	return match(pat, str, 0);
}

/**
 * Matches whether the string 'str' matches the pattern 'pat'
 * and returns its matching score.
 *
 * @param pat the glob pattern
 * @param str the string to match
 * @return 0 if no match or number representing the matching score
 */
unsigned globmatchi(const char *pat, const char *str)
{
	return match(pat, str, FNM_CASEFOLD);
}

#if !WITH_FNMATCH
int fnmatch(const char *pattern, const char *string, int flags)
{
	return !match(pattern, string, flags);
}
#endif
